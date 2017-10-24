//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-10-19 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Register.h>
#include <ThreeState.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED1_PIN 4
#define LED2_PIN 5
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

#define SENS1_PIN 6 //14
#define SENS2_PIN 3 //15

// number of available peers per channel
#define PEERS_PER_CHANNEL 8

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x49,0x29,0xd3},       // Device ID
    "papa4929d3",           // Device Serial
    {0x00,0xb2},            // Device Model
    0x13,                   // Firmware Version
    as::DeviceType::ThreeStateSensor, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef DualStatusLed<LED2_PIN,LED1_PIN> LedType;
typedef AskSin<LedType,BatterySensor,RadioType> HalType;

DEFREGISTER(Reg0,DREG_CYCLICINFOMSG,MASTERID_REGS,DREG_TRANSMITTRYMAX)
class WDSList0 : public RegList0<Reg0> {
public:
  WDSList0(uint16_t addr) : RegList0<Reg0>(addr) {}
  void defaults () {
    clear();
    cycleInfoMsg(false);
    transmitDevTryMax(6);
  }
};

DEFREGISTER(Reg1,CREG_AES_ACTIVE,CREG_MSGFORPOS,CREG_EVENTFILTERTIME,CREG_TRANSMITTRYMAX)
class WDSList1 : public RegList1<Reg1> {
public:
  WDSList1 (uint16_t addr) : RegList1<Reg1>(addr) {}
  void defaults () {
    clear();
    msgForPosA(1); // DRY
    msgForPosB(3); // WET
    msgForPosC(2); // WATER
    aesActive(false);
    eventFilterTime(5);
    transmitTryMax(6);
  }
};

typedef ThreeStateChannel<HalType,WDSList0,WDSList1,DefList4,PEERS_PER_CHANNEL> ChannelType;
typedef ThreeStateDevice<HalType,ChannelType,1,WDSList0> DevType;

HalType hal;
DevType sdev(devinfo,0x20);
ConfigButton<DevType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  sdev.channel(1).init(SENS1_PIN,SENS2_PIN);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  hal.battery.init(seconds2ticks(60UL*60), sysclock);
  hal.battery.low(22);
  hal.battery.critical(19);
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
    hal.activity.savePower<Sleep<> >(hal);
  }
}
