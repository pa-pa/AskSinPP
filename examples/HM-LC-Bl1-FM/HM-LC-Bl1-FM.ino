//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-12-14 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Blind.h>


// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

#define ON_RELAY_PIN 17
#define DIR_RELAY_PIN 16

#define UP_BUTTON_PIN 6
#define DOWN_BUTTON_PIN 3

// number of available peers per channel
#define PEERS_PER_CHANNEL 12

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x59,0x32,0xaf},              // Device ID
    "papa5932af",                  // Device Serial
    {0x00,0x05},                   // Device Model
    0x24,                          // Firmware Version
    as::DeviceType::BlindActuator, // Device Type
    {0x01,0x00}                    // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> RadioSPI;
typedef AskSin<StatusLed<4>,NoBattery,Radio<RadioSPI,2> > Hal;

DEFREGISTER(BlindReg0,MASTERID_REGS,DREG_INTKEY,DREG_CONFBUTTONTIME,DREG_LOCALRESETDISABLE)

class BlindList0 : public RegList0<BlindReg0> {
public:
  BlindList0 (uint16_t addr) : RegList0<BlindReg0>(addr) {}
  void defaults () {
    clear();
    // intKeyVisible(false);
    confButtonTime(0xff);
    // localResetDisable(false);
  }
};


// setup the device with channel type and number of channels
typedef MultiChannelDevice<Hal,BlindChannel<Hal,PEERS_PER_CHANNEL,BlindList0>,1,BlindList0> BlindType;

Hal hal;
BlindType sdev(devinfo,0x20);
ConfigToggleButton<BlindType> cfgBtn(sdev);
InternalButton<BlindType> btnup(sdev,1);
InternalButton<BlindType> btndown(sdev,2);

void initPeerings (bool first) {
  // create internal peerings - CCU2 needs this
  if( first == true ) {
    HMID devid;
    sdev.getDeviceID(devid);
    Peer p1(devid,1);
    Peer p2(devid,2);
    sdev.channel(1).peer(p1,p2);
  }
}

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  //storage().setByte(0,0);
  bool first = sdev.init(hal);
  sdev.channel(1).init(ON_RELAY_PIN,DIR_RELAY_PIN);

  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  buttonISR(btnup,UP_BUTTON_PIN);
  buttonISR(btndown,DOWN_BUTTON_PIN);

  initPeerings(first);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Idle<> >(hal);
  }
}
