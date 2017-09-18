//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-03-20 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

// number of relays by defining the device
#define HM_LC_SW1_SM 0x00,0x02
#define HM_LC_SW2_SM 0x00,0x0a
#define HM_LC_SW4_SM 0x00,0x03

#define CFG_LOWACTIVE_BYTE 0x00
#define CFG_LOWACTIVE_ON   0x01
#define CFG_LOWACTIVE_OFF  0x00

#define DEVICE_CONFIG CFG_LOWACTIVE_OFF

#include <SPI.h>    // when we include SPI.h - we can use LibSPI class
#include <EEPROM.h> // the EEPROM library contains Flash Access Methods
#include <AskSinPP.h>

#include <MultiChannelDevice.h>
#include <SwitchChannel.h>

// use builtin led
#define LED_PIN LED_BUILTIN
// Arduino pin for the config button
// use button on maple mini board
#define CONFIG_BUTTON_PIN PB8

#define RELAY1_PIN PC13
#define RELAY2_PIN PC14
#define RELAY3_PIN PC15
#define RELAY4_PIN PA0

// number of available peers per channel
#define PEERS_PER_CHANNEL 4


// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x12,0x34,0x56},       // Device ID
    "papa000000",           // Device Serial
    {HM_LC_SW4_SM},         // Device Model
    0x16,                   // Firmware Version
    as::DeviceType::Switch, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef LibSPI<PA4> RadioSPI;
typedef AskSin<StatusLed<LED_BUILTIN>,NoBattery,Radio<RadioSPI,PB0> > Hal;
Hal hal;

// map number of channel to pin
// this will be called by the SwitchChannel class
uint8_t SwitchPin (uint8_t number) {
  switch( number ) {
    case 2: return RELAY2_PIN;
    case 3: return RELAY3_PIN;
    case 4: return RELAY4_PIN;
  }
  return RELAY1_PIN;
}

// setup the device with channel type and number of channels
typedef MultiChannelDevice<Hal,SwitchChannel<Hal,PEERS_PER_CHANNEL>,4> SwitchDevice;
SwitchDevice sdev(devinfo,0x20);

ConfigToggleButton<SwitchDevice,LOW,HIGH,INPUT_PULLDOWN> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);

  // this will also trigger powerUpAction handling
  bool low = sdev.getConfigByte(CFG_LOWACTIVE_BYTE);
  DPRINT("Invert ");low ? DPRINTLN("active") : DPRINTLN("disabled");
  for( uint8_t i=1; i<=sdev.channels(); ++i ) {
    sdev.channel(i).lowactive(low);
  }

  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);

  // TODO - random delay
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
//    hal.activity.savePower<Idle>(hal);
  }
}
