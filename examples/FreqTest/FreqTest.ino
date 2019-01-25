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

HMID central; //(0xee,0xee,0xee); // count only messages from that device
#define RANGE 0x80
#define SCANTIME seconds2ticks(60)
//#define ACTIVE_PING

class TestDevice : public Device<HalType,DefList0>, Alarm {
  DefList0 l0;
  uint16_t freq, start, end;
  uint8_t received, rssi;
public:
  bool done;
  HMID id;

  typedef Device<HalType,DefList0> BaseDevice;
  TestDevice (const DeviceInfo& i,uint16_t addr) : BaseDevice(i,addr,l0,0), Alarm(0), l0(addr), freq(0x656A),
      start(0xffff), end(0), received(0), rssi(0), done(false) {}
  virtual ~TestDevice () {}

  virtual void trigger (AlarmClock& clock) {
    DPRINT("  ");DDEC(received);DPRINT("/");DDECLN(rssi);
    if( received > 0 ) {
      start = min(start,freq);
      end   = max(end,freq);
    }
    if( freq < 0x656A + RANGE) {
      setFreq(freq+0x10);
    }
    else {
      DPRINT("\nDone: 0x21");DHEX(start);DPRINT(" - 0x21");DHEXLN(end);
      freq = start+((end - start)/2);
      DPRINT("Setting: 0x21");DHEX((uint8_t)(freq>>8));DHEXLN((uint8_t)(freq&0xff));
      done = true;

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

  virtual bool process(Message& msg) {
    if( central == HMID::broadcast || (msg.from() == central && msg.to() == id) ) {
      if( central == HMID::broadcast ) msg.from().dump();
      DPRINT(".");
      //msg.from().dump(); DPRINT("->"); DDECLN(radio().rssi());
      rssi = max(rssi,radio().rssi());
      received++;
      if( received > 2 ) {
        trigger(sysclock);
      }
    }
    return true;
  }

  bool init (HalType& hal) {
    this->setHal(hal);
    this->getDeviceID(id);
    hal.init(id);
    hal.config(getConfigArea());
    setFreq(0x656A - RANGE);
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
    void patchStatus(Message& msg) {}
    void changed(bool b) {}
  };
  uint8_t cnt;
  channel ch;
public:
  InfoSender () : Alarm(0), cnt(0) {}
  virtual ~InfoSender () {}

  virtual void trigger (AlarmClock& clock) {
    InfoActuatorStatusMsg msg;
    msg.init(cnt++, ch, hal.radio.rssi());
    msg.to(central);
    msg.ackRequired();
    msg.setRpten();
    sdev.getDeviceID(msg.from());
    sdev.radio().write(msg,msg.burstRequired());
    sdev.led().ledOn(millis2ticks(100), 0);
    if( sdev.done == false ) {
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
