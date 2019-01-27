//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <Device.h>
#include <Register.h>


// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
#define LED_PIN2 5
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x01,0x01,0x01},       // Device ID
    "FreqTest00",           // Device Serial
    {0x00,0x00},            // Device Model
    0x10,                   // Firmware Version
    as::DeviceType::Sensor, // Device Type
    {0x00,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> RadioSPI;
typedef Radio<RadioSPI,2> RadioType;
typedef DualStatusLed<5,4> LedType;
typedef AskSin<LedType,NoBattery,RadioType> HalType;



#define STARTFREQ 0x656A             // frequency we start scanning
#define MINFREQ (STARTFREQ - 0x300)  // frequency we abort scanning
#define SEARCHSTEP 0x50              // step with during search
#define BOUNDSTEP 0x10               // step width during upper/lower bound analysis


// #define ACTIVE_PING
HMID PING_FROM(0x12,0x34,0x56);      // from address for status message
HMID PING_TO(0x99,0x66,0x99);        // to address for status message / central / CCU
#ifdef ACTIVE_PING
  #define SCANTIME seconds2ticks(5)   // maximal time to wait for a valid message
#else
  #define SCANTIME seconds2ticks(60)   // maximal time to wait for a valid message
#endif



class TestDevice : public Device<HalType,DefList0>, Alarm {
  DefList0 l0;
  uint16_t freq, start, end;
  uint8_t received, rssi;
public:
  enum SearchMode { Search, Up, Down, Done };
  SearchMode mode;
  HMID id;

  typedef Device<HalType,DefList0> BaseDevice;
  TestDevice (const DeviceInfo& i,uint16_t addr) : BaseDevice(i,addr,l0,0), Alarm(0), l0(addr),
      freq(STARTFREQ), start(STARTFREQ), end(STARTFREQ),
      received(0), rssi(0), mode(Search)  {}
  virtual ~TestDevice () {}

  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    DPRINT("  ");DDEC(received);DPRINT("/");DDECLN(rssi);

    if( mode == Search ) {
      if( received > 0 ) {
        start = end = freq;
        mode = Up; // start find upper bound
        DPRINTLN("Search for upper bound");
        setFreq(freq + BOUNDSTEP);
      }
      else {
        if( freq < MINFREQ ) mode = Done;
        if( freq <= STARTFREQ ) setFreq(STARTFREQ + (STARTFREQ-freq) + SEARCHSTEP);
        else setFreq(STARTFREQ - (freq-STARTFREQ));
      }
    }
    else if(mode == Up) {
      if( received > 0 ) {
        end = freq;
        setFreq(end + BOUNDSTEP);
      }
      else {
        mode = Down; // start find lower bound
        DPRINTLN("Search for lower bound");
        setFreq(start - BOUNDSTEP);
      }
    }
    else if(mode == Down) {
      if( received > 0 ) {
        start = freq;
        setFreq(start - BOUNDSTEP);
      }
      else {
        mode = Done;
      }
    }
    if( mode == Done ) {
      DPRINT("\nDone: 0x21");DHEX(start);DPRINT(" - 0x21");DHEXLN(end);
      if( start == end && start == STARTFREQ ) {
        DPRINT("Could not receive any message");
      }
      else {
        freq = start+((end - start)/2);
        DPRINT("Calculated Freq: 0x21");DHEX((uint8_t)(freq>>8));DHEXLN((uint8_t)(freq&0xff));

        // store frequency
        DPRINT("Store into config area: ");DHEX((uint8_t)(freq>>8));DHEXLN((uint8_t)(freq&0xff));
        StorageConfig sc = getConfigArea();
        sc.clear();
        sc.setByte(CONFIG_FREQ1, freq>>8);
        sc.setByte(CONFIG_FREQ2, freq&0xff);
        sc.validate();

        this->getHal().activity.savePower<Sleep<> >(this->getHal());
      }
    }
  }

  virtual bool process(Message& msg) {
    msg.from().dump(); DPRINT(".");
    rssi = max(rssi,radio().rssi());
    received++;
    if( received > 0 ) {
      trigger(sysclock);
    }
    return true;
  }

  bool init (HalType& hal) {
    this->setHal(hal);
    this->getDeviceID(id);
    hal.init(id);

    DPRINTLN("Start searching ...");
    setFreq(STARTFREQ);
    return false;
  }

  void setFreq (uint16_t current) {
    sysclock.cancel(*this);
    freq = current;
    rssi=0;
    received=0;
    DPRINT("Freq 0x21");DHEX(freq);DPRINT(": ");
    this->radio().initReg(CC1101_FREQ2, 0x21);
    this->radio().initReg(CC1101_FREQ1, freq >> 8);
    this->radio().initReg(CC1101_FREQ0, freq & 0xff);
    set(SCANTIME);
    sysclock.add(*this);
  }
};

HalType hal;
TestDevice sdev(devinfo,0x20);

class InfoSender : public Alarm {
  class channel {
  public:
    uint8_t number() const {return 1; }
    uint8_t status() const {return 0; }
    uint8_t flags()  const {return 0; }
    void patchStatus(__attribute__ ((unused)) Message& msg) {}
    void changed(__attribute__ ((unused)) bool b) {}
  };
  uint8_t cnt;
  channel ch;
public:
  InfoSender () : Alarm(0), cnt(0) {}
  virtual ~InfoSender () {}

  virtual void trigger (AlarmClock& clock) {
    InfoActuatorStatusMsg msg;
    msg.init(cnt++, ch, hal.radio.rssi());
    msg.to(PING_TO);
    msg.from(PING_FROM);
    msg.ackRequired();
    msg.setRpten();
    sdev.radio().write(msg,msg.burstRequired());
    sdev.led().ledOn(millis2ticks(100), 0);
    if( sdev.mode != TestDevice::SearchMode::Done ) {
      set(seconds2ticks(1));
      clock.add(*this);
    }
  }
} info;

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
#ifdef ACTIVE_PING
  // start sender
  info.trigger(sysclock);
#endif
}

void loop() {
  sdev.pollRadio();
  hal.runready();
}
