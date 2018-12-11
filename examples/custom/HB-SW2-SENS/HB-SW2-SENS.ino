//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-10-24 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
//#define USE_OTA_BOOTLOADER
//#define NDEBUG

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Switch.h>
#include <ThreeState.h>

#define CONFIG_BUTTON_PIN 8
#define LED_PIN           4

#define RELAY1_PIN 17
#define RELAY2_PIN 16

#define SENS1_PIN 6
#define SENS2_PIN 3

// number of available peers per channel

#define PEERS_PER_SWCHANNEL  6
#define PEERS_PER_SENSCHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x63,0xCA,0x41},       // Device ID
    "HBSwSen001",           // Device Serial
    {0xf2,0x02},            // Device Model
    0x01,                   // Firmware Version
    as::DeviceType::Switch, // Device Type
    {0x01,0x00}             // Info Bytes
};

// Configure the used hardware
typedef AvrSPI<10,11,12,13> RadioSPI;
typedef AskSin<StatusLed<LED_PIN>,NoBattery,Radio<RadioSPI,2> > Hal;
Hal hal;

DEFREGISTER(Reg0,DREG_INTKEY,DREG_CYCLICINFOMSG,MASTERID_REGS)
class SwSensList0 : public RegList0<Reg0> {
public:
  SwSensList0(uint16_t addr) : RegList0<Reg0>(addr) {}
};

DEFREGISTER(Reg1,CREG_AES_ACTIVE,CREG_MSGFORPOS,CREG_EVENTDELAYTIME,CREG_LEDONTIME,CREG_TRANSMITTRYMAX)
class SensList1 : public RegList1<Reg1> {
public:
  SensList1 (uint16_t addr) : RegList1<Reg1>(addr) {}
  void defaults () {
    clear();
    msgForPosA(1); // CLOSED
    msgForPosB(2); // OPEN
    msgForPosC(3); // TILTED
    // aesActive(false);
    // eventDelaytime(0);
    ledOntime(100);
    transmitTryMax(6);
  }
};

typedef SwitchChannel<Hal,PEERS_PER_SWCHANNEL,SwSensList0>  SwChannel;
typedef ThreeStateChannel<Hal,SwSensList0,SensList1,DefList4,PEERS_PER_SENSCHANNEL> SensChannel;

class MixDevice : public ChannelDevice<Hal,VirtBaseChannel<Hal,SwSensList0>,3,SwSensList0> {
#define CYCLETIME seconds2ticks(60UL*60*24) // at least one message per day
class CycleInfoAlarm : public Alarm {
  MixDevice& dev;
public:
  CycleInfoAlarm (MixDevice& d) : Alarm (CYCLETIME), dev(d) {}
  virtual ~CycleInfoAlarm () {}

  void trigger (AlarmClock& clock)  {
    set(CYCLETIME);
    clock.add(*this);
    dev.sensorChannel().changed(true); // force StatusInfoMessage to central
  }
} cycle;

public:
  VirtChannel<Hal,SwChannel,SwSensList0> c1,c2;
  VirtChannel<Hal,SensChannel,SwSensList0> c3;
public:
  typedef ChannelDevice<Hal,VirtBaseChannel<Hal,SwSensList0>,3,SwSensList0> DeviceType;
  MixDevice (const DeviceInfo& info,uint16_t addr) : DeviceType(info,addr), cycle(*this) {
    DeviceType::registerChannel(c1,1);
    DeviceType::registerChannel(c2,2);
    DeviceType::registerChannel(c3,3);
  }
  virtual ~MixDevice () {}

  SwChannel& switch1Channel ()  { return c1; }
  SwChannel& switch2Channel ()  { return c2; }
  SensChannel& sensorChannel () { return c3; }

  virtual void configChanged () {
    if( /*this->getList0().cycleInfoMsg() ==*/ true ) {
      DPRINTLN("Activate Cycle Msg");
      sysclock.cancel(cycle);
      cycle.set(CYCLETIME);
      sysclock.add(cycle);
    }
    else {
      DPRINTLN("Deactivate Cycle Msg");
      sysclock.cancel(cycle);
    }
  }
};
MixDevice sdev(devinfo,0x20);
ConfigButton<MixDevice> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  sdev.switch1Channel().init(RELAY1_PIN);
  sdev.switch2Channel().init(RELAY2_PIN);
  sdev.sensorChannel().init(SENS1_PIN,SENS2_PIN);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Idle<> >(hal);
  }
}
