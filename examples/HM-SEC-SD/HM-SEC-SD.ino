//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-02-08 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// ci-test=yes board=328p aes=no
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER


#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Register.h>
#include <MultiChannelDevice.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED1_PIN 4
#define LED2_PIN 5
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

#define SENS1_PIN 6
#define SOUND_PIN 15

// number of available peers per channel
#define PEERS_PER_CHANNEL 6
#define CYCLETIME seconds2ticks(60UL * 60 * 16)

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
  {0x03, 0x12, 0x34},     // Device ID
  "papa031234",           // Device Serial
  {0x00, 0x42},           // Device Model
  0x01,                   // Firmware Version
  DeviceType::SmokeDetector,    // Device Type
  {0x01, 0x00}            // Info Bytes
};

/**
   Configure the used hardware
*/
typedef AvrSPI<10, 11, 12, 13> SPIType;
typedef Radio<SPIType, 2> RadioType;
typedef DualStatusLed<LED2_PIN, LED1_PIN> LedType;
typedef AskSin<LedType, BatterySensor, RadioType> Hal;
Hal hal;

DEFREGISTER(Reg0, DREG_INTKEY, MASTERID_REGS)
class SecSDList0 : public RegList0<Reg0> {
  public:
    SecSDList0(uint16_t addr) : RegList0<Reg0>(addr) {}
    void defaults () {
      clear();
    }
};

DEFREGISTER(Reg1, CREG_AES_ACTIVE)
class SecSDList1 : public RegList1<Reg1> {
  public:
    SecSDList1 (uint16_t addr) : RegList1<Reg1>(addr) {}
    void defaults () {
      clear();
      // aesActive(false);
    }
};


class SecSDChannel : public Channel<Hal,SecSDList1,EmptyList,DefList4,PEERS_PER_CHANNEL,SecSDList0>, public Alarm {
  uint8_t senspin, soundpin;
  uint8_t state, count;
public:
  typedef Channel<Hal,SecSDList1,EmptyList,DefList4,PEERS_PER_CHANNEL,SecSDList0> BaseChannel;

  SecSDChannel () : BaseChannel(), Alarm(0), senspin(0), soundpin(0), state(0), count(0) {}
  virtual ~SecSDChannel () {}

  void setup(Device<Hal,SecSDList0>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev,number,addr);
  }

  void init (uint8_t pin1,uint8_t pin2) {
    senspin=pin1;
    soundpin=pin2;
    pinMode(senspin,INPUT_PULLUP);
    pinMode(soundpin,OUTPUT);
    // start polling
    set(seconds2ticks(1));
    sysclock.add(*this);
  }

  uint8_t status () const {
    return state;
  }

  uint8_t flags () const {
    uint8_t flags = this->device().battery().low() ? 0x80 : 0x00;
    return flags;
  }

  void trigger (AlarmClock& clock)  {
    // check for smoke
    uint8_t newstate = smokeState();
    if( state != newstate ) {
      state = newstate;
      // get team leader - peer 0
      uint8_t idx = 0;
      Peer leader = peer(idx);
      if( leader.valid() == true ) {
        SensorEventMsg& msg = (SensorEventMsg&)device().message();
        msg.init(device().nextcount(),number(),count++,state,device().battery().low());
        msg.flags(Message::BCAST | Message::BURST); // original smoke detector is burst device
        msg.setRpten();
        msg.from(leader);
        device().getDeviceID(msg.to());

        process(msg); // we first

        device().send(msg);
        device().send(msg);
        device().send(msg);
      }
    }
    // reactivate for next measure
    set(seconds2ticks(1));
    clock.add(*this);
  }

  // we get an event from the team lead
  bool process (const SensorEventMsg& msg) {
    sound(msg.value() == 200);
    return true;
  }

  uint8_t smokeState () {
    return digitalRead(senspin) == LOW ? 200 : 1;
  }

  void sound (bool on) {
    digitalWrite(soundpin,on==true ? HIGH : LOW);
  }

  bool process (__attribute__((unused)) const ActionSetMsg& msg) { return false; }
  bool process (__attribute__((unused)) const RemoteEventMsg& msg) { return false; }
  bool process (__attribute__((unused)) const ActionCommandMsg& msg) { return false; }

};

class SecSDDevice : public MultiChannelDevice<Hal,SecSDChannel,1,SecSDList0> {
  public:
    typedef MultiChannelDevice<Hal,SecSDChannel,1,SecSDList0> BaseDevice;
    SecSDDevice(const DeviceInfo& info, uint16_t addr) : BaseDevice(info, addr) {}
    virtual ~SecSDDevice () {}

    virtual void configChanged () {
      BaseDevice::configChanged();
    }
};

SecSDDevice sdev(devinfo, 0x20);
ConfigButton<SecSDDevice> cfgBtn(sdev);

class CycleInfoAlarm : public Alarm {
public:
  CycleInfoAlarm () : Alarm (CYCLETIME) {}
  virtual ~CycleInfoAlarm () {}

  void trigger (AlarmClock& clock)  {
    set(CYCLETIME);
    clock.add(*this);
    sdev.channel(1).changed(true); // force StatusInfoMessage to central
  }
} cycle;


void initPeerings (bool first) {
  // create internal peerings
  // set ourself as team leader
  if ( first == true ) {
    HMID devid;
    sdev.getDeviceID(devid);
    Peer ipeer(devid, 1);
    sdev.channel(1).peer(ipeer);
  }
}

void setup () {
  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
  bool first = sdev.init(hal);
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  sdev.channel(1).init(SENS1_PIN, SOUND_PIN);
  initPeerings(first);
  sdev.initDone();
  sysclock.add(cycle);
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if ( worked == false && poll == false ) {
    // if nothing to do - go sleep
    hal.activity.savePower<Idle<> >(hal);
  }
}
