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

#include <Register.h>
#include <MultiChannelDevice.h>
#include <OneWire.h>
#include <sensors/Ds18b20.h>
#include <sensors/Tsl2561.h>
//#include <sensors/Bh1750.h>
//#include <sensors/Bmp180.h>
#include <sensors/Sht10.h>
//#include <sensors/Dht.h>

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
typedef StatusLed<LED_PIN> LedType;
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

DEFREGISTER(WeatherRegsList0,MASTERID_REGS,DREG_BURSTRX)
typedef RegList0<WeatherRegsList0> WeatherList0;


class WeatherChannel : public Channel<Hal,List1,EmptyList,List4,PEERS_PER_CHANNEL,WeatherList0>, public Alarm {

  uint16_t        millis;
//  Dht<6,DHT11>      dht11;
  Sht10<A4,A5>    sht10;
//  Bmp180          bmp180;
  Tsl2561<>       tsl2561;
//  Bh1750<>          bh1750;
  Ds18b20         ds18b20;
  OneWire         ow;

public:
  WeatherChannel () : Channel(), Alarm(5), millis(0), ow(6) {}
  virtual ~WeatherChannel () {}

  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    // wait also for the millis
    if( millis != 0 ) {
      tick = millis2ticks(millis);
      millis = 0; // reset millis
      sysclock.add(*this); // millis with sysclock
    }
    else {      
      // measure and send
      measure();
      uint8_t msgcnt = device().nextcount();
      WeatherEventMsg& msg = (WeatherEventMsg&)device().message();
//      msg.init(msgcnt,dht11.temperature(),dht11.humidity(),device().battery().low());
      msg.init(msgcnt,sht10.temperature(),sht10.humidity(),device().battery().low());
//      msg.init(msgcnt,0,0,device().battery().low());
      device().broadcastPeerEvent(msg,*this);
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
    DPRINTLN("Measure...  ");
//    dht11.measure();
//    DPRINT("T: ");DDEC(dht11.temperature());DPRINT("  H: ");DDECLN(dht11.humidity());
    sht10.measure();
    DPRINT("T: ");DDEC(sht10.temperature());DPRINT("  H: ");DDECLN(sht10.humidity());
//    bmp180.measure();
//    DPRINT("T: ");DDEC(bmp180.temperature());DPRINT("  P: ");DDECLN(bmp180.pressure());
    tsl2561.measure();
    DPRINT("H: ");DDECLN(tsl2561.brightness());
//    bh1750.measure();
//    DPRINT("H: ");DDECLN(bh1750.brightness());
    ds18b20.measure();
    DPRINT("T: ");DDECLN(ds18b20.temperature());
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

  void setup(Device<Hal,WeatherList0>* dev,uint8_t number,uint16_t addr) {
    Channel::setup(dev,number,addr);
    tick = 5;
    rtc.add(*this);
//    dht11.init();
    sht10.init();
//    bmp180.init();
    tsl2561.init();
//    bh1750.init();
    Ds18b20::init(ow, &ds18b20, 1);
  }

  uint8_t status () const {
    return 0;
  }

  uint8_t flags () const {
    return 0;
  }

};


typedef MultiChannelDevice<Hal,WeatherChannel,1,WeatherList0> WeatherType;
WeatherType sdev(devinfo,0x20);

ConfigButton<WeatherType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<SleepRTC>(hal);
  }
}
