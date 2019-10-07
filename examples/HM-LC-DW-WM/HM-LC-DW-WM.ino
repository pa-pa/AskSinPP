//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2019-03-05 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Dimmer.h>

// We use a Pro Mini
// PIN for Status LED
#define LED_PIN 4
// PIN for ConfigToggleButton
#define CONFIG_BUTTON_PIN 8
// PINs for PWM LED drivers
#define DIMMER1_PIN 3
#define DIMMER2_PIN 5


// number of available peers per channel
#define PEERS_PER_CHANNEL 2

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x01,0x08,0x01},       // Device ID
    "papa010801",           // Device Serial
    {0x01,0x08},            // Device Model: HM-LC-DW-WM dual white LED dimmer
    0x2C,                   // Firmware Version
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
typedef DimmerChannel<HalType,PEERS_PER_CHANNEL> ChannelType;
typedef DimmerDevice<HalType,ChannelType,6,3> DimmerType;

HalType hal;
DimmerType sdev(devinfo,0x20);
DualWhiteControl<HalType,DimmerType,PWM8<> > control(sdev);
ConfigToggleButton<DimmerType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  if( control.init(hal,DIMMER1_PIN,DIMMER2_PIN) ) {
    // first init - setup connection between button and first channel
    sdev.channel(1).peer(cfgBtn.peer());
  }
  // Init the hw button
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);

  sdev.initDone();

  // Output ID and Serial in serial console
  DDEVINFO(sdev);
}

void loop() {
  hal.runready();
  sdev.pollRadio();
}
