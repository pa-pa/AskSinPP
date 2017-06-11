//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define CFG_STEPUP_BYTE 0x00
#define CFG_STEPUP_OFF  0x00
#define CFG_STEPUP_ON   0x01

#define CFG_BAT_LOW_BYTE 0x01
#define CFG_BAT_CRITICAL_BYTE 0x02

// define all device properties
#define DEVICE_ID HMID(0x09,0x56,0x34)
#define DEVICE_SERIAL "papa222111"
#define DEVICE_MODEL  0x00,0x30
#define DEVICE_FIRMWARE 0x18
#define DEVICE_TYPE DeviceType::ThreeStateSensor
#define DEVICE_INFO 0x01,0x01,0x00
#define DEVICE_CONFIG CFG_STEPUP_OFF,22,19

#define MODE_POLL

// 24 0030 4D455130323134373633 80 910101

#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <TimerOne.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED1_PIN 4
#define LED2_PIN 5
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

#define SENS1_PIN 14
#define SENS2_PIN 15
#define SABOTAGE_PIN 16

// number of available peers per channel
#define PEERS_PER_CHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

class BatSensor : public BatterySensorUni<17,7,3000> {
  bool m_Extern;
public:
  // sense pin = A3 = 17, activation pin = D7 = 7
  BatSensor () : BatterySensorUni(), m_Extern(false) {}
  virtual ~BatSensor () {}

  void hasStepUp (bool value) {
    m_Extern = value;
    voltage();
  }

  virtual uint8_t voltage () {
    if( m_Extern == true ) {
      return BatterySensorUni<17,7,3000>::voltage();
    }
    return BatterySensor::voltage();
  }
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef DualStatusLed<LED2_PIN,LED1_PIN> LedType;
typedef AskSin<LedType,BatSensor,RadioType> BaseHal;
class Hal : public BaseHal {
public:
  void init () {
    BaseHal::init();
    // measure battery every 1h
    battery.init(seconds2ticks(60UL*60),sysclock);
  }
} hal;


class RHSList0Data : public List0Data {
  uint8_t CycleInfoMsg      : 8;   // 0x09 - 09
  uint8_t TransmitDevTryMap : 8;   // 0x14 - 20
  uint8_t sabotageMsg       : 8;   // 0x10 - 16

public:
  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x09: return sizeof(List0Data) + 0;
      case 0x14: return sizeof(List0Data) + 1;
      case 0x16: return sizeof(List0Data) + 2;
      default:   break;
    }
    return List0Data::getOffset(reg);
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case sizeof(List0Data) + 0:  return 0x09;
      case sizeof(List0Data) + 1:  return 0x14;
      case sizeof(List0Data) + 2:  return 0x16;
      default: break;
    }
    return List0Data::getRegister(offset);
  }
};

class RHSList0 : public ChannelList<RHSList0Data> {
public:
  RHSList0(uint16_t a) : ChannelList(a) {}

  operator List0& () const { return *(List0*)this; }

  // from List0
  HMID masterid () { return ((List0*)this)->masterid(); }
  void masterid (const HMID& mid) { ((List0*)this)->masterid(mid); }
  bool aesActive() const { return ((List0*)this)->aesActive(); }

  bool cycleInfoMsg () const { return getByte(sizeof(List0Data) + 0); }
  bool cycleInfoMsg (bool value) const { return setByte(sizeof(List0Data) + 0,value); }
  uint8_t transmitDevTryMax () const { return getByte(sizeof(List0Data) + 1); }
  bool transmitDevTryMax (uint8_t value) const { return setByte(sizeof(List0Data) + 1,value); }
  bool sabotageMsg () const { return getByte(sizeof(List0Data) + 2); }
  bool sabotageMsg (bool value) const { return setByte(sizeof(List0Data) + 2,value); }

  void defaults () {
    ((List0*)this)->defaults();
    cycleInfoMsg(false);
    sabotageMsg(true);
    transmitDevTryMax(6);
  }
};

class RHSList1Data {

   uint8_t aesActive                 :1;   // 0x08.0, s:1    d: false
   uint8_t unused1                   :7;
   uint8_t msgForPosC                :2;   // 0x20.2, s:2    d: TILTED
   uint8_t msgForPosB                :2;   // 0x20.4, s:2    d: OPEN
   uint8_t msgForPosA                :2;   // 0x20.6, s:2    d: CLOSED
   uint8_t eventDelaytime            :8;   // 0x21.0, s:8    d: 0.0 s
   uint8_t ledOntime                 :8;   // 0x22.0, s:8    d: 0.5 s
   uint8_t transmitTryMax            :8;   // 0x30.0, s:8    d: nF

public:
   static uint8_t avrRegister[5];

   static uint8_t getOffset  (uint8_t r) {
      for (uint8_t i=0;i<sizeof(avrRegister);i++) {
         if(avrRegister[i]==r) {
            return i;
         }
      }
      return 0xff;
   }
   static uint8_t getRegister(uint8_t o) {
     if (o < sizeof(avrRegister)) {
        return avrRegister[o];
     };
     return 0xff;
   }
};

uint8_t RHSList1Data::avrRegister[] = {0x08,0x20,0x21,0x22,0x30};

#define NO_MSG 0
#define CLOSED_MSG 1
#define CLOSED_STATE 0
#define OPEN_MSG 2
#define OPEN_STATE 200
#define TILTED_MSG 3
#define TILTED_STATE 100

#define NoPos 0
#define PosA  1
#define PosB  2
#define PosC  3


class RHSList1 : public ChannelList<RHSList1Data> {
public:
   RHSList1(uint16_t a) : ChannelList(a) {}

  bool aesActive(bool v) const { return setBit(0, 0x01, v); }
  bool aesActive() const { return isBitSet(0, 0x01); }

  bool msgForPosC(uint8_t v) const { return setByte(1,v,0x0c,2); }
  uint8_t msgForPosC() const { return getByte(1,0x0c,2); }
  bool msgForPosB(uint8_t v) const { return setByte(1,v,0x30,4); }
  uint8_t msgForPosB() const { return getByte(1,0x30,4); }
  bool msgForPosA(uint8_t v) const { return setByte(1,v,0xc0,6); }
  uint8_t msgForPosA() const { return getByte(1,0xc0,6); }

  bool eventDelaytime(uint8_t v) const { return setByte(2,v); }
  uint8_t eventDelaytime() const { return getByte(2); }
  bool ledOntime(uint8_t v) const { return setByte(3,v); }
  uint8_t ledOntime() const { return getByte(3); }
  bool transmitTryMax(uint8_t v) const { return setByte(4,v); }
  uint8_t transmitTryMax() const { return getByte(4); }

   void defaults () {
      aesActive (0);
      msgForPosC (TILTED_MSG);
      msgForPosB (OPEN_MSG);
      msgForPosA (CLOSED_MSG);
      eventDelaytime (0);
      ledOntime (100);
   }
};

#define DEBOUNCETIME millis2ticks(200)

template <class HALTYPE,int PEERCOUNT>
class RHSChannel : public Channel<HALTYPE,RHSList1,EmptyList,List4,PEERCOUNT>, public Alarm {
  // pin mapping can be changed by bootloader config data
  // map pins to pos     00   01   10   11
  uint8_t posmap[4] = {PosA,PosC,PosB,PosA};
  volatile bool isr;
  uint8_t state, count;
  bool sabotage;

public:
  typedef Channel<HALTYPE,RHSList1,EmptyList,List4,PEERCOUNT> BaseChannel;

  RHSChannel () : BaseChannel(), Alarm(DEBOUNCETIME), isr(false), state(255), count(0), sabotage(false) {}
  virtual ~RHSChannel () {}

  void setup(Device<HALTYPE>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev,number,addr);
    // TODO try to read pin to position mapping from bootloader
  }

  uint8_t status () const {
    return state;
  }

  uint8_t flags () const {
    uint8_t flags = sabotage ? 0x02 : 0x00;
    flags |= this->device().battery().low() ? 0x80 : 0x00;
    return flags;
  }

  void handleISR () {
    if( isr == false ) {
      isr = true;
      set(DEBOUNCETIME);
      sysclock.add(*this);
    }
  }

  uint8_t readPin(uint8_t pinnr) {
    uint8_t value=0;
#ifdef MODE_POLL
    pinMode(pinnr,INPUT_PULLUP);
    value = digitalRead(pinnr);
    pinMode(pinnr,OUTPUT);
    digitalWrite(pinnr,LOW);
#else
    value = digitalRead(pinnr);
#endif
    return value;
  }

  void trigger (__attribute__ ((unused)) AlarmClock& clock)  {
#ifdef MODE_POLL
    set(seconds2ticks(1));
    clock.add(*this);
#else
    if( isr == true ) {
#endif
      uint8_t newstate = state;
      uint8_t pinstate = readPin(SENS2_PIN) << 1 | readPin(SENS1_PIN);
      uint8_t pos = posmap[pinstate & 0x03];
      uint8_t msg = NO_MSG;
      switch( pos ) {
      case PosA:
        msg = this->getList1().msgForPosA();
        break;
      case PosB:
        msg = this->getList1().msgForPosB();
        break;
      case PosC:
        msg = this->getList1().msgForPosC();
        break;
      default:
        break;
      }

      if( msg == CLOSED_MSG) newstate = CLOSED_STATE;
      else if( msg == OPEN_MSG) newstate = OPEN_STATE;
      if( msg == TILTED_MSG) newstate = TILTED_STATE;

      if( newstate != state ) {
        state = newstate;
        SensorEventMsg& msg = (SensorEventMsg&)BaseChannel::device().message();
        msg.init(this->device().nextcount(),this->number(),count++,state,this->device().battery().low());
        this->device().sendPeerEvent(msg,*this);
      }

      bool sab = readPin(SABOTAGE_PIN) == LOW;
      if( sabotage != sab && ((const RHSList0&)this->device().getList0()).sabotageMsg() == true ) {
        sabotage = sab;
        this->changed(true); // trigger StatusInfoMessage to central
      }
#ifdef MODE_POLL
#else
      isr = false;
    }
#endif
  }
};


class RHSType : public MultiChannelDevice<Hal,RHSChannel<Hal,PEERS_PER_CHANNEL>,1,RHSList0> {
  #define CYCLETIME seconds2ticks(60UL*60*24) // at least one message per day
  class CycleInfoAlarm : public Alarm {
    RHSType& dev;
  public:
    CycleInfoAlarm (RHSType& d) : Alarm (CYCLETIME), dev(d) {}
    virtual ~CycleInfoAlarm () {}

    void trigger (AlarmClock& clock)  {
      set(CYCLETIME);
      clock.add(*this);
      dev.channel(1).changed(true); // force StatusInfoMessage to central
    }
  } cycle;
public:
  typedef MultiChannelDevice<Hal,RHSChannel<Hal,PEERS_PER_CHANNEL>,1,RHSList0> DevType;
  RHSType(uint16_t addr) : DevType(addr), cycle(*this) {}
  virtual ~RHSType () {}

  virtual void configChanged () {
    // activate cycle info message
    if( this->getList0().cycleInfoMsg() == true ) {
      DPRINTLN("Activate Cycle Msg");
      sysclock.cancel(cycle);
      cycle.set(CYCLETIME);
      sysclock.add(cycle);
    }
    else {
      DPRINTLN("Deactivate Cycle Msg");
      sysclock.cancel(cycle);
    }
    // set battery low/critical values
    battery().low(getConfigByte(CFG_BAT_LOW_BYTE));
    battery().critical(getConfigByte(CFG_BAT_CRITICAL_BYTE));
    // set the battery mode
    if( getConfigByte(CFG_STEPUP_BYTE) == CFG_STEPUP_ON ) {
      DPRINTLN("Use StepUp");
      battery().hasStepUp(true);
    }

  }
};

RHSType sdev(0x20);
ConfigButton<RHSType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
#ifdef MODE_POLL
  // start polling
  sdev.channel(1).set(millis2ticks(100));
  sysclock.add(sdev.channel(1));
#else
  channelISR(sdev.channel(1),SENS1_PIN,INPUT_PULLUP,CHANGE);
  channelISR(sdev.channel(1),SENS2_PIN,INPUT_PULLUP,CHANGE);
  channelISR(sdev.channel(1),SABOTAGE_PIN,INPUT_PULLUP,CHANGE);
  // trigger first send of state
  sdev.channel(1).handleISR();
#endif
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

