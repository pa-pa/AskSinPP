//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2020-01-23 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//
// based on https://github.com/kc-GitHub/Wettersensor
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EXTRAMILLIS 730 // 730 millisecond extra time to better hit the slot
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Register.h>
#include <MultiChannelDevice.h>
#include <Weather.h>

#include <Wire.h>
#include <sensors/Bmp180.h>
#include <sensors/Tsl2561.h>
#include <sensors/Sht10.h>

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
//    {0xab,0xcd,0xef},       // Device ID
    {0x0b,0x00,0x01},       // Device ID
    "HB0Default",           // Device Serial
    {0xf1,0x01},            // Device Model
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
typedef AskSinRTC<LedType,BatterySensor,RadioType> Hal;

/*
 * Define List0 registers
 */
#define DREG_ALTITUDE 36,37
DEFREGISTER(WeatherRegsList0,MASTERID_REGS,DREG_BURSTRX,DREG_LEDMODE,DREG_TRANSMITTRYMAX,DREG_LOWBATLIMIT,DREG_ALTITUDE)
class WeatherList0 : public RegList0<WeatherRegsList0> {
public:
  WeatherList0(uint16_t a) : RegList0<WeatherRegsList0>(a) {}

  bool altitude (uint16_t value) const {
    return this->writeRegister(36, (value >> 8) & 0xff) && this->writeRegister(37, value & 0xff);
  }
  uint16_t altitude () const {
    return (this->readRegister(36,0) << 8) + this->readRegister(37,0);
  }

};

/*
 * Sensors class is used by the WeatherChannel to measure the data. It has to implement
 * temperature(), humidity(), pressure(), brightness()
 */
class Sensors : public Alarm {
  Sht10<A4,A5>    sht10;
  Bmp180          bmp;
  Tsl2561<>       tsl;
public:
  Sensors () {}
  virtual ~Sensors() {}

  // init the used hardware
  void init () {
    sht10.init();
    bmp.init();
    tsl.init();
  }
  // return how many milliseconds the measure should start in front of sending the message
  uint16_t before () const { return 4000; }
  // get the data
  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    DPRINTLN("Measure...  ");
    sht10.measure();
    bmp.measure();
    tsl.measure();
    DPRINT("T: ");DDEC(sht10.temperature());DPRINT("  H: ");DDECLN(sht10.humidity());
    DPRINT("P: ");DDEC(bmp.pressure());DPRINT("  B: ");DDECLN(tsl.brightness());
  }
  uint16_t temperature () { return sht10.temperature(); }
  uint8_t  humidity () { return sht10.humidity(); }
  uint16_t pressure () { return bmp.pressure() * 10; }
  uint32_t brightness () { return tsl.brightness(); }
};


class SensChannel : public WeatherChannel<Hal,RTC,Sensors,PEERS_PER_CHANNEL,EXTRAMILLIS,WeatherList0> {
public:
  SensChannel () : WeatherChannel<Hal,RTC,Sensors,PEERS_PER_CHANNEL,EXTRAMILLIS,WeatherList0>() {}
  virtual ~SensChannel () {}

  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    // check if delay for millis is active - only if using RTC
    if( delayMillis() == false ) {
      uint8_t msgcnt = this->device().nextcount();
      // send
      WeatherEventMsg& msg = (WeatherEventMsg&)this->device().message();
      msg.init(msgcnt,this->sensors().temperature(),this->sensors().humidity(),this->device().battery().low());
      // we add extra data to the weather event
      // TODO use altitude ????
      msg.append(this->sensors().pressure());
      // TODO - original uses Lux ?????
      msg.append(this->sensors().brightness());
      msg.append((uint16_t)(this->device().battery().voltageHighRes()*10));
      this->device().broadcastEvent(msg);
      // reactivate for next send
      reactivate(msg);
    }
  }

};

class SensDevice : public MultiChannelDevice<Hal,SensChannel,1,WeatherList0> {
public:
  SensDevice (const DeviceInfo& i,uint16_t addr) : MultiChannelDevice<Hal,SensChannel,1,WeatherList0>(i,addr) {}
  virtual ~SensDevice () {}

  virtual void configChanged () {
    battery().low(getList0().lowBatLimit());
    battery().critical(getList0().lowBatLimit()-3);
  }

};

Hal hal;
SensDevice sdev(devinfo,0x20);
ConfigButton<SensDevice> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  hal.initBattery(60UL*60,22,19);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.sleep<>();
  }
}
