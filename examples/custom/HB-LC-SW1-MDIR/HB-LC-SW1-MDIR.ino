//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2020-11-29 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// ci-test=yes board=328p aes=no
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <Switch.h>
#include <Motion.h>
#include <sensors/Bme280.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

// relay pin
#define RELAY_PIN 16

#define PIR_PIN 3
#define LDR_ENABLE  6 // D6
#define LDR_SENS   17 // A3

// number of available peers per channel
#define PEERS_PER_CHANNEL 6
#define PEERS_PER_PIRCHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0xf2,0x0b,0x01},       // Device ID
    "papaf20b01",           // Device Serial
    {0xf2,0x0b},            // Device Model
    0x10,                   // Firmware Version
    as::DeviceType::Switch, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> RadioSPI;
typedef AskSin<StatusLed<LED_PIN>,NoBattery,Radio<RadioSPI,2> > Hal;
Hal hal;

class LDR : public virtual Brightness {
public:
  LDR () : Brightness() {
    _present=true;
    pinMode(LDR_ENABLE,OUTPUT);
    pinMode(LDR_SENS,INPUT);
    digitalWrite(LDR_ENABLE,LOW);
  }
  void measure (__attribute__((unused)) bool async=false) {
    digitalWrite(LDR_ENABLE,HIGH);
    _delay_ms(5);
    _brightness = analogRead(LDR_SENS);
    DDECLN(_brightness);
    digitalWrite(LDR_ENABLE,LOW);
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

class ValuesChannel : public Channel<Hal,ValuesList1,EmptyList,EmptyList,0,List0>, Alarm {
  Bme280    bme280;
public:
  typedef Channel<Hal,ValuesList1,EmptyList,EmptyList,0,List0> BaseChannel;

  ValuesChannel () : BaseChannel(), Alarm(0) {}
  virtual ~ValuesChannel () {}

  virtual void trigger (AlarmClock& clock) {
    bme280.measure();

    DPRINT("T: ");DDEC(bme280.temperature());
    DPRINT("   P: ");DDEC(bme280.pressure());
    DPRINT("   H: ");DDECLN(bme280.humidity());

    ValuesMsg& msg = (ValuesMsg&)device().message();
    msg.init(device().nextcount(),number());
    msg.add(bme280.temperature());
    msg.add(bme280.humidity());
    msg.add(bme280.pressure());
    device().broadcastEvent(msg);
//    device().send(msg,device().getMasterID());

    uint8_t delay = max((uint8_t)15,this->getList1().eventDelaytime());
    set(AskSinBase::byteTimeCvtSeconds(delay));
    clock.add(*this);
  }

  void setup(Device<Hal,List0>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev, number, addr);
    bme280.init();
    set(seconds2ticks(5));
    sysclock.add(*this);
  }

  uint8_t status () const { return 0; }
  uint8_t flags () const { return 0; }
};


typedef SwitchChannel<Hal,PEERS_PER_CHANNEL,List0>        SwChannel;
typedef MotionChannel<Hal,PEERS_PER_PIRCHANNEL,List0,LDR> PirChannel;

class SwPirDevice : public ChannelDevice<Hal,VirtBaseChannel<Hal,List0>,3,List0> {
  VirtChannel<Hal,SwChannel,List0>  s;
  VirtChannel<Hal,PirChannel,List0> m;
  VirtChannel<Hal,ValuesChannel>    v;

public:
  typedef ChannelDevice<Hal,VirtBaseChannel<Hal,List0>,3,List0> DeviceType;
  SwPirDevice (const DeviceInfo& info,uint16_t addr) : DeviceType(info,addr) {
    DeviceType::registerChannel(s,1);
    DeviceType::registerChannel(m,2);
    DeviceType::registerChannel(v,3);
  }
  virtual ~SwPirDevice () {}

  SwChannel&  swChannel  () { return s; }
  PirChannel& pirChannel () { return m; }
  ValuesChannel& valChannel () { return v; }
};

SwPirDevice sdev(devinfo,0x20);
ConfigButton<SwPirDevice> cfgBtn(sdev);

void setupPeers(bool first) {
  if( first == true ) {
    HMID hmid;
    sdev.getDeviceID(hmid);
    Peer pp(hmid,2);
    sdev.swChannel().peer(pp);
    SwitchPeerList l = sdev.swChannel().getList3(pp).sh();
    l.jtOn(AS_CM_JT_ON);
    l.onTime((4 << 5) | 2); // 2min
    l.ctValLo(0);

    Peer ps(hmid,1);
    sdev.pirChannel().peer(ps);
    sdev.pirChannel().getList1().minInterval(0);
  }
}

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  bool first = sdev.init(hal);
  setupPeers(first);
  sdev.swChannel().init(RELAY_PIN,false);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  motionChannelISR(sdev.pirChannel(),PIR_PIN);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Idle<> >(hal);
  }
}
