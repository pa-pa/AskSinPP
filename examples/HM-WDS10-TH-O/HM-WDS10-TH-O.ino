//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
// https://github.com/spease/Sensirion.git
#include <Sensirion.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x34,0x56,0x78},       // Device ID
    "papa111111",           // Device Serial
    {0x00,0x3d},            // Device Model
    0x10,                   // Firmware Version
    as::DeviceType::THSensor, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef StatusLed<4> LedType;
typedef AskSin<LedType,BatterySensor,RadioType> BaseHal;
class Hal : public BaseHal {
public:
  void init (const HMID& id) {
    BaseHal::init(id);
    // init real time clock - 1 tick per second
    rtc.init();
    // measure battery every 1h
    battery.init(60UL*60,rtc);
    battery.low(22);
    battery.critical(19);
  }

  bool runready () {
    return rtc.runready() || BaseHal::runready();
  }
} hal;

class WeatherEventMsg : public Message {
public:
  void init(uint8_t msgcnt,int16_t temp,uint8_t humidity, bool batlow) {
    uint8_t t1 = (temp >> 8) & 0x7f;
    uint8_t t2 = temp & 0xff;
    if( batlow == true ) {
      t1 |= 0x80; // set bat low bit
    }
    Message::init(0xc,msgcnt,0x70,BIDI,t1,t2);
    pload[0] = humidity;
  }
};

class WeatherChannel : public Channel<Hal,List1,EmptyList,List4,PEERS_PER_CHANNEL>, public Alarm {

  WeatherEventMsg msg;
  int16_t         temp;
  uint8_t         humidity;

  Sensirion       sht10;
  uint16_t        millis;

public:
  WeatherChannel () : Channel(), Alarm(5), temp(0), humidity(0), millis(0) {}
  virtual ~WeatherChannel () {}

  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    // wait also for the millis
    if( millis != 0 ) {
      tick = millis2ticks(millis);
      millis = 0; // reset millis
      sysclock.add(*this); // millis with sysclock
    }
    else {
      uint8_t msgcnt = device().nextcount();
      measure();
      msg.init(msgcnt,temp,humidity,device().battery().low());
      device().sendPeerEvent(msg,*this);
//      device().send(msg,device().getMasterID());

      // reactivate for next measure
      HMID id;
      device().getDeviceID(id);
      uint32_t nextsend = delay(id,msgcnt);
      tick = nextsend / 1000; // seconds to wait
      millis = nextsend % 1000; // millis to wait
      rtc.add(*this);
    }
  }

  // here we do the measurement
  void measure () {
    DPRINT("Measure...\n");
    uint16_t rawData;
    if ( sht10.measTemp(&rawData)== 0) {
      float t = sht10.calcTemp(rawData);
      temp = t * 10;
      if( sht10.measHumi(&rawData)== 0 ) {
        humidity = sht10.calcHumi(rawData, t);
      }
    }
  }

  // here we calc when to send next value
  uint32_t delay (const HMID& id,uint8_t msgcnt) {
    uint32_t value = ((uint32_t)id) << 8 | msgcnt;
    value = (value * 1103515245 + 12345) >> 16;
    value = (value & 0xFF) + 480;
    value *= 250;

    DDEC(value / 1000);DPRINT(".");DDECLN(value % 1000);

    return value;
  }

  void setup(Device<Hal>* dev,uint8_t number,uint16_t addr) {
    Channel::setup(dev,number,addr);
    rtc.add(*this);
    sht10.config(A4,A5);
    sht10.writeSR(LOW_RES);
  }

  uint8_t status () const {
    return 0;
  }

  uint8_t flags () const {
    return 0;
  }

};


typedef MultiChannelDevice<Hal,WeatherChannel,1> WeatherType;
WeatherType sdev(devinfo,0x20);

ConfigButton<WeatherType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<SleepRTC>(hal);
  }
}
