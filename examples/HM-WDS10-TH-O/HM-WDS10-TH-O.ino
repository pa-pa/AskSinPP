//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
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
    {0x34,0x56,0x79},       // Device ID
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
typedef AskSinRTC<LedType,BatterySensor,RadioType> Hal;

/*
 * Define List0 registers
 */
DEFREGISTER(WeatherRegsList0,MASTERID_REGS,DREG_BURSTRX)
typedef RegList0<WeatherRegsList0> WeatherList0;

/*
 * Sensors class is used by the WeatherChannel to measure the data. It has to implement
 * temperature() and humidity().
 */
class Sensors : public Alarm {
  Sht10<A4,A5>    sht10;
public:
  Sensors () {}
  virtual ~Sensors () {}
  // init the used hardware
  void init () { sht10.init(); }
  // return how many milliseconds the measure should start in front of sending the message
  uint16_t before () const { return 4000; }
  // get the data
  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    DPRINTLN("Measure...  ");
    sht10.measure();
    DPRINT("T: ");DDEC(sht10.temperature());DPRINT("  H: ");DDECLN(sht10.humidity());
  }
  uint16_t temperature () { return sht10.temperature(); }
  uint8_t  humidity () { return sht10.humidity(); }
};


typedef WeatherChannel<Hal,RTC,Sensors,PEERS_PER_CHANNEL,EXTRAMILLIS,WeatherList0> ChannelType;
typedef MultiChannelDevice<Hal,ChannelType,1,WeatherList0> WeatherType;

Hal hal;
WeatherType sdev(devinfo,0x20);
ConfigButton<WeatherType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  hal.initBattery(60UL*60,22,19);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.sleep<>();
  }
}
