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

#include <Dimmer.h>

#include <actors/PCA9685.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

#define DIMMER_PIN 3

// number of available peers per channel
#define PEERS_PER_CHANNEL 4

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x11,0x12,0x22},       // Device ID
    "papa111222",           // Device Serial
    {0x00,0x67},            // Device Model
    0x25,                   // Firmware Version
    as::DeviceType::Dimmer, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef StatusLed<LED_PIN> LedType;
typedef AskSin<LedType,NoBattery,RadioType> HalType;
typedef DimmerChannel<HalType,PEERS_PER_CHANNEL,PWM16ext> ChannelType;
typedef DimmerDevice<HalType,ChannelType,3,3> DimmerType;

HalType hal;
DimmerType sdev(devinfo,0x20);
DimmerControl<HalType,DimmerType,PWM16ext<> > control(sdev);
ConfigToggleButton<DimmerType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  if( control.init(hal,DIMMER_PIN) ) {
    // first init - setup connection between config button and first channel
    sdev.channel(1).peer(cfgBtn.peer());
  }
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Idle<true> >(hal);
  }
}
