//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-11-16 stan23 Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2021-03-25 modified for the STM32L1xx CPU by trilu
//- -----------------------------------------------------------------------------------------------------------------------

#define HIDE_IGNORE_MSG

// as we have no defined HW in STM32duino yet, we are working on base of the RAK811 board,
// which has a different pin map. deviances are handled for the moment in a local header file. 
// This might change when an own HW for AskSin is defined
#include "stm32l1-variant.h"

// Derive ID and Serial from the device UUID
//#define USE_HW_SERIAL

// when we include SPI.h - we can use LibSPI class, we want to use SPI2
// defines are not required if an own AskSin HW is available in STM32 core
#define PIN_SPI_MOSI PB15
#define PIN_SPI_MISO PB14
#define PIN_SPI_SCK  PB13
#include <SPI.h>

#include <AskSinPP.h>
#include <Device.h>
#include <Register.h>

#define CC1101_GDO0_PIN     PA8
#define CC1101_CS_PIN       PB12
#define CC1101_EN_PIN       PA15

// Pin definition of the specific device
#define CONFIG_BUTTON_PIN   PA11
#define LED_PIN             PC13




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
typedef LibSPI<CC1101_CS_PIN> RadioSPI;
typedef Radio<RadioSPI, CC1101_GDO0_PIN> RadioType;
typedef StatusLed<LED_PIN> LedType;
typedef AskSin<LedType,NoBattery,RadioType> HalType;



#define STARTFREQ 0x656A             // frequency we start scanning
#define MINFREQ (STARTFREQ - 0x300)  // frequency we abort scanning
#define SEARCHSTEP 0x50              // step with during search
#define BOUNDSTEP 0x10               // step width during upper/lower bound analysis



// see https://github.com/AskSinPP/asksinpp.de/blob/master/Grundlagen/FAQ/Fehlerhafte_CC1101.md
// #define ACTIVE_PING
HMID PING_FROM(0x12,0x34,0x56);      // from address for status message e.g. switch
HMID PING_TO(0x99,0x66,0x99);        // to address for status message / central / CCU
#ifdef ACTIVE_PING
  #define SCANTIME seconds2ticks(5)  // maximal time to wait for a valid message
#else
  #define SCANTIME seconds2ticks(20) // maximal time to wait for a valid message
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
    DPRINT("  ");DDEC(received);
    if (!received) {
      DPRINTLN("");
    } else {
      DPRINT(" / -");DDEC(rssi);DPRINTLN("dBm");
    }

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
        DPRINT("Calculated Freq: 0x21");DHEX((uint8_t)(freq>>8));DHEX((uint8_t)(freq&0xff));
        printFreq(0x210000 + freq);DPRINTLN("");

        // store frequency
        DPRINT("Store into config area: ");DHEX((uint8_t)(freq>>8));DHEX((uint8_t)(freq&0xff));
        StorageConfig sc = getConfigArea();
#if defined ARDUINO_ARCH_STM32F1
        Wire.begin();
        DPRINT(".");
#endif
        sc.clear();
        DPRINT(".");
        sc.setByte(CONFIG_FREQ1, freq>>8);
        DPRINT(".");
        sc.setByte(CONFIG_FREQ2, freq&0xff);
        DPRINT(".");
        sc.validate();

        DPRINTLN("stored!");

        // measurement is done, loop here forever
        while(1);
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
    DPRINT("Freq 0x21");DHEX(freq);
    printFreq(0x210000 + freq);
    DPRINT(": ");
    this->radio().initReg(CC1101_FREQ2, 0x21);
    this->radio().initReg(CC1101_FREQ1, freq >> 8);
    this->radio().initReg(CC1101_FREQ0, freq & 0xff);
    set(SCANTIME);
    sysclock.add(*this);
  }

  void printFreq(uint32_t freq) {
    char buffer[16];
    float val = (float)freq * 26.0 / 65536.0;
    dtostrf(val, 8, 3, buffer);
    DPRINT(buffer);DPRINT(" MHz");
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
#ifdef ACTIVE_PING
    InfoActuatorStatusMsg msg;
    msg.init(cnt++, ch, hal.radio.rssi());
    msg.to(PING_TO);
    msg.from(PING_FROM);
    msg.ackRequired();
    msg.setRpten();
    sdev.radio().write(msg,msg.burstRequired());
#endif
    sdev.led().ledOn(millis2ticks(100), 0);
    if( sdev.mode != TestDevice::SearchMode::Done ) {
      set(seconds2ticks(1));
      clock.add(*this);
    }
  }
} info;

void setup () {
  // start cc1101 module
  pinMode(CC1101_EN_PIN, OUTPUT);
  digitalWrite(CC1101_EN_PIN, HIGH);

  delay(1000);

  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  // start sender
  info.trigger(sysclock);

  //sdev.led().invert(true);

}

void loop() {
  sdev.pollRadio();
  hal.runready();
}
