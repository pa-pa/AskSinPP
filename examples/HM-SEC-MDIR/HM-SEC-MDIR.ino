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
#include <Motion.h>

#include <SparkFunTSL2561.h>
#include <Wire.h>


// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8
// Arduino pin for the PIR
// A0 == PIN 14 on Pro Mini
#define PIR_PIN 14

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x56,0x78,0x90},       // Device ID
    "papa222222",           // Device Serial
    {0x00,0x4a},            // Device Model
    0x16,                   // Firmware Version
    as::DeviceType::MotionDetector, // Device Type
    {0x01,0x01,0x00}        // Info Bytes
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
    // set low voltage to 2.2V
    // measure battery every 1h
    battery.init(seconds2ticks(60UL*60),sysclock);
    battery.low(22);
    battery.critical(19);
  }
} hal;

// Create an SFE_TSL2561 object, here called "light":
SFE_TSL2561 light;
bool lightenabled = false;
unsigned char TSL2561_gain;
unsigned int TSL2561_time;
unsigned int TSL2561_ms;

uint8_t measureBrightness () {
  static uint16_t maxvalue = 0;
  uint8_t value = 0;
  if( lightenabled == true ) {
    unsigned int data0, data1;
    if (light.getData(data0,data1)) {
      double lux;    // Resulting lux value
      light.getLux (TSL2561_gain,TSL2561_ms,data0,data1,lux);
      uint16_t current = (uint16_t)lux;
      DPRINT(F("light: ")); DHEXLN(current);
      if( maxvalue < current ) {
        maxvalue = current;
      }
      value = 200UL * current / maxvalue;
    }
  }
  return value;
}


typedef MultiChannelDevice<Hal,MotionChannel<Hal,PEERS_PER_CHANNEL>,1> MotionType;
MotionType sdev(devinfo,0x20);

ConfigButton<MotionType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);

  light.begin();
    // If gain = false (0), device is set to low gain (1X)
    // If gain = high (1), device is set to high gain (16X)
    // If time = 0, integration will be 13.7ms
    // If time = 1, integration will be 101ms
    // If time = 2, integration will be 402ms
    // If time = 3, use manual start / stop to perform your own integration
  TSL2561_gain = 0;
  TSL2561_time = 2;
  lightenabled = light.setTiming(TSL2561_gain,TSL2561_time,TSL2561_ms); //gain,time);
  if( lightenabled == true ) {
    light.setPowerUp();
  }

  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  motionISR(sdev,1,PIR_PIN);
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    // deep discharge protection
    // if we drop below critical battery level - switch off all and sleep forever
    if( hal.battery.critical() ) {
      // this call will never return
      hal.activity.sleepForever(hal);
    }
    // if nothing to do - go sleep
    hal.activity.savePower<Sleep<>>(hal);
  }
}
