//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-08-09 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

//#define STORAGEDRIVER at24cX<0x50,128,32>

#include <SPI.h>    // when we include SPI.h - we can use LibSPI class
#include <Wire.h>
#include <EEPROM.h> // the EEPROM library contains Flash Access Methods
#include <AskSinPP.h>

#include <Dimmer.h>


// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN LED_BUILTIN
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN PB12

#define DIMMER_PIN PB1

// number of available peers per channel
#define PEERS_PER_CHANNEL 4

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x66,0x23,0xab},       // Device ID
    "papa6623ab",           // Device Serial
//    {0x01,0x09},            // Device Model
//    0x2C,                   // Firmware Version
    {0x00,0x67},            // Device Model
    0x25,                   // Firmware Version
    as::DeviceType::Dimmer, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef LibSPI<PA4> RadioSPI;
typedef AskSin<StatusLed<LED_BUILTIN>,NoBattery,Radio<RadioSPI,PB0> > HalType;
typedef DimmerChannel<HalType,PEERS_PER_CHANNEL> ChannelType;
typedef DimmerDevice<HalType,ChannelType,6,3,PWM16<> > DimmerType;

HalType hal;
DimmerType sdev(devinfo,0x20);
ConfigToggleButton<DimmerType> cfgBtn(sdev);

void setup () {
  delay(5000);
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal,DIMMER_PIN,PB9);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  sdev.initDone();
  sdev.led().invert(true);
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
//    hal.activity.savePower<Idle<true> >(hal);
  }
}
