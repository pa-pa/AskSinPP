//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-03-20 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// ci-test=yes board=maplemini aes=no
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


//#define STORAGEDRIVER at24c32
#define STORAGEDRIVER at24cX<0x50,128,32>

#include <SPI.h>    // when we include SPI.h - we can use LibSPI class
#include <Wire.h>
#include <EEPROM.h> // the EEPROM library contains Flash Access Methods
#include <AskSinPP.h>

#include <MultiChannelDevice.h>
#include <Switch.h>

// use builtin led
#define LED_PIN LED_BUILTIN
// Arduino pin for the config button
// use button on maple mini board
#define CONFIG_BUTTON_PIN PB8

#define RELAY1_PIN PC13
#define RELAY2_PIN PC14
#define RELAY3_PIN PC15
#define RELAY4_PIN PA8

// number of available peers per channel
#define PEERS_PER_CHANNEL 6


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

// setup the device with channel type and number of channels
typedef MultiChannelDevice<Hal,SwitchChannel<Hal,PEERS_PER_CHANNEL,List0>,4> SwitchDevice;
SwitchDevice sdev(devinfo,0x20);

ConfigToggleButton<SwitchDevice,LOW,HIGH,INPUT_PULLDOWN> cfgBtn(sdev);

void initPeerings (bool first) {
  // create internal peerings - CCU2 needs this
  if( first == true ) {
    HMID devid;
    sdev.getDeviceID(devid);
    for( uint8_t i=1; i<=sdev.channels(); ++i ) {
      Peer ipeer(devid,i);
      sdev.channel(i).peer(ipeer);
    }
  }
}

void setup () {
  delay(2000);
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  Wire.begin();
  bool first = sdev.init(hal);
  // this will also trigger powerUpAction handling
  bool low = sdev.getConfigByte(CFG_LOWACTIVE_BYTE);
  DPRINT("Invert ");low ? DPRINTLN("active") : DPRINTLN("disabled");
  sdev.channel(1).init(RELAY1_PIN,low);
  sdev.channel(2).init(RELAY2_PIN,low);
  sdev.channel(3).init(RELAY3_PIN,low);
  sdev.channel(4).init(RELAY4_PIN,low);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  initPeerings(first);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
//    hal.activity.savePower<Idle>(hal);
  }
}

