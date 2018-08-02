//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-22 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Register.h>
#include <Device.h>
#include <MultiChannelDevice.h>

#include <Sensirion.h>
#include <Sensors.h>
#include <sensors/Sht10.h>

// use builtin led
#define LED_PIN 4
// Arduino pin for the config button
// use button on maple mini board
#define CONFIG_BUTTON_PIN 8


// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0xfa,0x32,0x71},       // Device ID
    "papafa3271",           // Device Serial
    {0xf2,0x05},            // Device Model
    0x01,                   // Firmware Version
	  as::DeviceType::Sensor, // Device Type
    {0x00,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> RadioSPI;
typedef AskSin<StatusLed<4>,BatterySensor,Radio<RadioSPI,2> > Hal;

DEFREGISTER(SensReg0,MASTERID_REGS)
class SensList0 : public RegList0<SensReg0> {
public:
  SensList0 (uint16_t addr) : RegList0<SensReg0>(addr) {}
};

DEFREGISTER(ValuesReg1,CREG_AES_ACTIVE,CREG_EVENTDELAYTIME)
class ValuesList1 : public RegList1<ValuesReg1> {
public:
  ValuesList1 (uint16_t addr) : RegList1<ValuesReg1>(addr) {}
  void defaults () {
     aesActive(false);
     eventDelaytime(0x83); // 3 minutes
   }
};

class ValuesChannel : public Channel<Hal,ValuesList1,EmptyList,EmptyList,0,SensList0>, Alarm {
  Sht10<A4,A5>    sht10;

public:
  typedef Channel<Hal,ValuesList1,EmptyList,EmptyList,0,SensList0> BaseChannel;

  ValuesChannel () : BaseChannel(), Alarm(0) {}
  virtual ~ValuesChannel () {}

  virtual void trigger (AlarmClock& clock) {
    sht10.measure();

    DPRINT("T: ");DDEC(sht10.temperature());
    DPRINT("   H: ");DDECLN(sht10.humidity());

    ValuesMsg& msg = device().message().values();
    msg.init(device().nextcount(),number());
    msg.add(sht10.temperature());
    msg.add(sht10.humidity());
    device().send(msg, device().getMasterID());

    uint8_t delay = max(15,this->getList1().eventDelaytime());
    set(AskSinBase::byteTimeCvtSeconds(delay));
    clock.add(*this);
  }

  void setup(Device<Hal,SensList0>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev, number, addr);
    sht10.init();
    set(seconds2ticks(5));
    sysclock.add(*this);
  }

  uint8_t status () const { return 0; }
  uint8_t flags () const { return 0; }

};

class SensType : public MultiChannelDevice<Hal,ValuesChannel,1,SensList0> {
public:
  typedef MultiChannelDevice<Hal,ValuesChannel,1,SensList0> DevType;
  SensType (const DeviceInfo& i,uint16_t addr) : DevType(i,addr) {}
  virtual ~SensType () {}
};

Hal hal;
SensType sdev(devinfo,0x20);
ConfigButton<SensType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Sleep<>>(hal);
  }
}
