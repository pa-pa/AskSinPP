//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-03-20 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

//#define STORAGEDRIVER at24c32
#define STORAGEDRIVER at24cX<0x50,128,32>
#define NORTC

#include <SPI.h>    // when we include SPI.h - we can use LibSPI class
#include <Wire.h>
#include <EEPROM.h> // the EEPROM library contains Flash Access Methods
#include <AskSinPP.h>
#include <OneWireSTM.h>
#include <sensors/Bme280.h>

#include <Register.h>
#include <Device.h>
#include <MultiChannelDevice.h>
#include <Switch.h>
#include <Remote.h>
#include <Motion.h>
#include <IButton.h>

#include <Sensors.h>
#include <sensors/Bh1750.h>

// use builtin led
#define LED_PIN LED_BUILTIN
// Arduino pin for the config button
// use button on maple mini board
#define CONFIG_BUTTON_PIN PB8

#define BELL_BUTTON_PIN PB4
#define BELL_LIGHT_PIN PA8
#define IR_PIN PA9
#define MOTION_PIN PB3

#define DOOR_PIN PC13
#define BUTTON_GREEN PC14
#define BUTTON_RED PC15

#define IBUTTON_READER_PIN PA0

#define IBUTTON_CHANNELS 6
#define NUM_CHANNELS IBUTTON_CHANNELS+5

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0xaa,0x11,0x33},       // Device ID
    "papaaa1133",           // Device Serial
    {0xf2,0x03},            // Device Model
    0x01,                   // Firmware Version
	  as::DeviceType::Remote, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef LibSPI<PA4> RadioSPI;
typedef AskSin<StatusLed<LED_BUILTIN>,NoBattery,Radio<RadioSPI,PB0> > Hal;

DEFREGISTER(DoorReg0,MASTERID_REGS,DREG_BACKONTIME)
class DoorList0 : public RegList0<DoorReg0> {
public:
  DoorList0 (uint16_t addr) : RegList0<DoorReg0>(addr) {}
  void defaults () {
    clear();
    backOnTime(3);
  }
};

class BellChannel : public RemoteChannel<Hal,PEERS_PER_CHANNEL,DoorList0> {
  class LightOff : public Alarm {
  public:
    LightOff () : Alarm(0) {}
    virtual ~LightOff () {}
    virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
      pwmWrite(BELL_LIGHT_PIN, 2048);
    }
  } loff;
public:
  BellChannel () {}
  virtual ~BellChannel () {}

  void setup(Device<Hal,DoorList0>* dev,uint8_t number,uint16_t addr) {
    RemoteChannel<Hal,PEERS_PER_CHANNEL,DoorList0>::setup(dev, number, addr);
    pinMode(BELL_LIGHT_PIN,PWM);
    pwmWrite(BELL_LIGHT_PIN, 2048);
  }

  virtual void state(uint8_t s) {
    RemoteChannel<Hal,PEERS_PER_CHANNEL,DoorList0>::state(s);
    if( s == Button::pressed || s == Button::longpressed ) {
      pwmWrite(BELL_LIGHT_PIN, 65535);
      sysclock.cancel(loff);
      loff.set(seconds2ticks(device().getList0().backOnTime()));
      sysclock.add(loff);
    }
  }
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

class ValuesChannel : public Channel<Hal,ValuesList1,EmptyList,EmptyList,0,DoorList0>, Alarm {
  Bme280    bme280;
  Bh1750<>  bh1750;
public:
  typedef Channel<Hal,ValuesList1,EmptyList,EmptyList,0,DoorList0> BaseChannel;

  ValuesChannel () : BaseChannel(), Alarm(0) {}
  virtual ~ValuesChannel () {}

  virtual void trigger (AlarmClock& clock) {
    bme280.measure();
    bh1750.measure();

    DPRINT("T: ");DDEC(bme280.temperature());
    DPRINT("   P: ");DDEC(bme280.pressure());
    DPRINT("   H: ");DDECLN(bme280.humidity());
    DPRINT("B: ");DDECLN(bh1750.brightness());

    ValuesMsg& msg = (ValuesMsg&)device().message();
    msg.init(device().nextcount(),number());
    msg.add(bme280.temperature());
    msg.add(bme280.humidity());
    msg.add(bme280.pressure());
    msg.add((uint16_t)bh1750.brightness());
    device().send(msg, device().getMasterID());

    uint8_t delay = max((uint8_t)15,this->getList1().eventDelaytime());
    set(AskSinBase::byteTimeCvtSeconds(delay));
    clock.add(*this);
  }

  void setup(Device<Hal,DoorList0>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev, number, addr);
    bme280.init();
    bh1750.init();
    set(seconds2ticks(5));
    sysclock.add(*this);
  }

  uint8_t status () const { return 0; }
  uint8_t flags () const { return 0; }

};

typedef SwitchChannel<Hal,PEERS_PER_CHANNEL,DoorList0> OpenerChannel;
typedef SwitchChannel<Hal,PEERS_PER_CHANNEL,DoorList0> IrChannel;
typedef MotionChannel<Hal,PEERS_PER_CHANNEL,DoorList0,Bh1750<> > PirChannel;
typedef IButtonChannel<Hal,PEERS_PER_CHANNEL,DoorList0> IButChannel;

class DoorBellDev :  public ChannelDevice<Hal,VirtBaseChannel<Hal,DoorList0>,NUM_CHANNELS,DoorList0> {
public:
  VirtChannel<Hal,BellChannel,DoorList0>  bell;
  VirtChannel<Hal,IButChannel,DoorList0>  ibut[IBUTTON_CHANNELS];
  VirtChannel<Hal,OpenerChannel,DoorList0>  opener;
  VirtChannel<Hal,IrChannel,DoorList0>  ir;
  VirtChannel<Hal,ValuesChannel,DoorList0>  values;
  VirtChannel<Hal,PirChannel,DoorList0>  motion;
public:
  typedef ChannelDevice<Hal,VirtBaseChannel<Hal,DoorList0>,NUM_CHANNELS,DoorList0> DeviceType;
  DoorBellDev(const DeviceInfo& info,uint16_t addr) : DeviceType(info,addr) {
    int ch=1;
    DeviceType::registerChannel(bell,ch++);
    DeviceType::registerChannel(opener,ch++);
    DeviceType::registerChannel(ir,ch++);
    DeviceType::registerChannel(values,ch++);
    DeviceType::registerChannel(motion,ch++);
    for( uint8_t i=0; i<ibuttonCount(); ++i ) {
      DeviceType::registerChannel(ibut[i],ch++);
    }
  }
  virtual ~DoorBellDev () {}

  BellChannel& bellChannel () { return bell; }
  OpenerChannel& openerChannel () { return opener; }
  IrChannel& irChannel () { return ir; }
  ValuesChannel& valuesChannel () { return values; }
  PirChannel& motionChannel () { return motion; }
  IButChannel& ibuttonChannel (uint8_t num) { return ibut[num]; }
  uint8_t ibuttonCount () const { return IBUTTON_CHANNELS; }
};


Hal hal;
DoorBellDev sdev(devinfo,0x20);
ConfigButton<DoorBellDev,LOW,HIGH,INPUT_PULLDOWN> cfgBtn(sdev);
IButtonScanner<DoorBellDev,IButChannel,IBUTTON_READER_PIN,BUTTON_GREEN,BUTTON_RED> scanner(sdev);

BME280I2C bmp;

void setup () {
  delay(2000); // wait until Maple Mini USB-Serial is available
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  Wire.begin();

  if( sdev.init(hal) == true ) {
    // setup internal peers on first init
    HMID devid;
    sdev.getDeviceID(devid);
    Peer bpeer(devid,3);
    sdev.bellChannel().peer(bpeer);
    Peer ipeer(devid,1);
    sdev.irChannel().peer(ipeer);
    SwitchList3 list3 = sdev.irChannel().getList3(ipeer);
    list3.sh().onTime(0x4c); // 60s
    list3.sh().jtOn(AS_CM_JT_ON);
    list3.lg().onTime(0x4c); // 60s
    list3.lg().jtOn(AS_CM_JT_ON);
  }
  remoteChannelISR(sdev.bellChannel(),BELL_BUTTON_PIN);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  sdev.openerChannel().init(DOOR_PIN);
  sdev.irChannel().init(IR_PIN);
  motionChannelISR(sdev.motionChannel(),MOTION_PIN);
  sdev.initDone();

  sysclock.add(scanner);

/*
  DHEX(sdev.bellChannel().getList1().address()); DPRINT("  ");
  sdev.bellChannel().getList1().dump();
  for( uint8_t i=0; i<IBUTTON_CHANNELS; ++i ) {
    DHEX(sdev.ibutChannel(i).getList1().address()); DPRINT("  ");
    sdev.ibutChannel(i).getList1().dump();
  }
  DHEX(sdev.openerChannel().getList1().address()); DPRINT("  ");
  sdev.openerChannel().getList1().dump();
*/
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
//    hal.activity.savePower<Sleep<>>(hal);
  }
}
