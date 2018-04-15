//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-05-10 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
// uncomment the following line if you have a TSL2561 connected at address 0x29
#include <sensors/Tsl2561.h>

#include <MultiChannelDevice.h>
#include <Remote.h>
#include <Motion.h>

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

#define BUTTON1_PIN 15
#define BUTTON2_PIN 16

// number of available peers per channel
#define PEERS_PER_PIRCHANNEL 6
#define PEERS_PER_BTNCHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x90,0x78,0x90},       // Device ID
    "papa111333",           // Device Serial
    {0x00,0xdb},            // Device Model
    0x11,                   // Firmware Version
    as::DeviceType::MotionDetector, // Device Type
    {0x00,0x00}             // Info Bytes
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
    // measure battery every 1h
    battery.init(seconds2ticks(60UL*60),sysclock);
    battery.low(22);
    battery.critical(19);
  }
} hal;

class BtnPirList0Data : public List0Data {
  uint8_t CycleInfoMsg      : 8;   // 0x09 - 09
  uint8_t TransmitDevTryMap : 8;   // 0x14 - 20
  uint8_t LocalResetDisbale : 1;   // 0x18 - 24

public:
  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x09: return sizeof(List0Data) + 0;
      case 0x14: return sizeof(List0Data) + 1;
      case 0x18: return sizeof(List0Data) + 2;
      default:   break;
    }
    return List0Data::getOffset(reg);
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case sizeof(List0Data) + 0:  return 0x09;
      case sizeof(List0Data) + 1:  return 0x14;
      case sizeof(List0Data) + 2:  return 0x18;
      default: break;
    }
    return List0Data::getRegister(offset);
  }
};

class BtnPirList0 : public ChannelList<BtnPirList0Data> {
public:
  BtnPirList0(uint16_t a) : ChannelList(a) {}

  operator List0& () const { return *(List0*)this; }

  // from List0
  HMID masterid () { return ((List0*)this)->masterid(); }
  void masterid (const HMID& mid) { ((List0*)this)->masterid(mid); }
  bool aesActive() const { return ((List0*)this)->aesActive(); }

  bool cycleInfoMsg () const { return getByte(sizeof(List0Data) + 0); }
  bool cycleInfoMsg (bool value) const { return setByte(sizeof(List0Data) + 0,value); }
  uint8_t transmitDevTryMax () const { return getByte(sizeof(List0Data) + 1); }
  bool transmitDevTryMax (uint8_t value) const { return setByte(sizeof(List0Data) + 1,value); }
  bool localResetDisable () const { return isBitSet(sizeof(List0Data) + 2,0x01); }
  bool localResetDisable (bool value) const { return setBit(sizeof(List0Data) + 2,0x01,value); }

  void defaults () {
    ((List0*)this)->defaults();
    cycleInfoMsg(false);
    localResetDisable(false);
    transmitDevTryMax(3);
  }
};

typedef RemoteChannel<Hal,PEERS_PER_BTNCHANNEL,BtnPirList0> BtnChannel;
#ifdef __SENSORS_TSL2561_h__
typedef MotionChannel<Hal,PEERS_PER_PIRCHANNEL,BtnPirList0,Tsl2561<TSL2561_ADDR_LOW> > PirChannel;
#else
typedef MotionChannel<Hal,PEERS_PER_PIRCHANNEL,BtnPirList0> PirChannel;
#endif

class MixDevice : public ChannelDevice<Hal,VirtBaseChannel<Hal,BtnPirList0>,3,BtnPirList0> {
public:
  VirtChannel<Hal,BtnChannel,BtnPirList0> c1,c2;
  VirtChannel<Hal,PirChannel,BtnPirList0> c3;
public:
  typedef ChannelDevice<Hal,VirtBaseChannel<Hal,BtnPirList0>,3,BtnPirList0> DeviceType;
  MixDevice (const DeviceInfo& info,uint16_t addr) : DeviceType(info,addr) {
    DeviceType::registerChannel(c1,1);
    DeviceType::registerChannel(c2,2);
    DeviceType::registerChannel(c3,3);
  }
  virtual ~MixDevice () {}

  BtnChannel& btn1Channel () { return c1; }
  BtnChannel& btn2Channel () { return c2; }
  PirChannel& pirChannel () { return c3; }
};
MixDevice sdev(devinfo,0x20);

ConfigButton<MixDevice> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  remoteChannelISR(sdev.btn1Channel(),BUTTON1_PIN);
  remoteChannelISR(sdev.btn2Channel(),BUTTON2_PIN);
  motionChannelISR(sdev.pirChannel(),PIR_PIN);
  sdev.initDone();
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
