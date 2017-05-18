//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

// define all device properties
#define DEVICE_ID HMID(0x09,0x56,0x34)
#define DEVICE_SERIAL "papa222111"
#define DEVICE_MODEL  0x00,0x30
#define DEVICE_FIRMWARE 0x18
#define DEVICE_TYPE DeviceType::ThreeStateSensor
#define DEVICE_INFO 0x01,0x01,0x00
#define DEVICE_CONFIG

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

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef DualStatusLed<LED2_PIN,LED1_PIN> LedType;
typedef BatterySensor<22,19> BatteryType;
typedef AskSin<LedType,BatteryType,RadioType,Sleep<false>> BaseHal;
class Hal : public BaseHal {
public:
  void init () {
    BaseHal::init();
    // set low voltage to 2.2V
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
class RHSChannel : public Channel<HALTYPE,RHSList1,EmptyList,List4,PEERCOUNT>, Alarm {

  volatile bool isr;
  uint8_t state;
  bool sabotage;

public:
  typedef Channel<HALTYPE,RHSList1,EmptyList,List4,PEERCOUNT> BaseChannel;

  RHSChannel () : BaseChannel(), Alarm(DEBOUNCETIME), isr(false), state(CLOSED_STATE), sabotage(false) {}
  virtual ~RHSChannel () {}

  uint8_t status () const {
    return state;
  }

  uint8_t flags () const {
    return BaseChannel::device().battery().low() ? 0x80 : 0x00;
  }

  void handleISR () {
    if( isr == false ) {
      isr = true;
      set(DEBOUNCETIME);
      sysclock.add(*this);
    }
  }

  void trigger (__attribute__ ((unused)) AlarmClock& clock)  {
    if( isr == true ) {
      uint8_t newstate = CLOSED_STATE;
      uint8_t p1 = digitalRead(SENS1_PIN);
      uint8_t p2 = digitalRead(SENS2_PIN);
      if( p1 == HIGH && p2 == HIGH) {
        newstate = CLOSED_STATE;
      }
      else if( p1 == LOW && p2 == HIGH ) {
        newstate = OPEN_STATE;
      }
      else if( p1 == HIGH && p2 == LOW ) {
        newstate = TILTED_STATE;
      }
      bool sab = digitalRead(SABOTAGE_PIN) == LOW;
      if( newstate != state || sabotage != sab ) {
        state = newstate;
        sabotage = sab;
//        BaseChannel::changed(true);
        SensorEventMsg& msg = (SensorEventMsg&)BaseChannel::device().message();
        msg.init(BaseChannel::device().nextcount(),BaseChannel::number(),state,BaseChannel::device().battery().low());
        // TODO sabotage ???
        BaseChannel::device().sendPeerEvent(msg,*this);
      }
      isr = false;
    }
  }
};


typedef MultiChannelDevice<Hal,RHSChannel<Hal,PEERS_PER_CHANNEL>,1,RHSList0> RHSType;
RHSType sdev(0x20);

ConfigButton<RHSType> cfgBtn(sdev);


#define CYCLETIME seconds2ticks(60*60) // a message every hour
class CycleInfoAlarm : public Alarm {
public:
  CycleInfoAlarm () : Alarm (CYCLETIME) {}
  virtual ~CycleInfoAlarm () {}

  void trigger (AlarmClock& clock)  {
    set(CYCLETIME);
    clock.add(*this);
    sdev.channel(1).changed(true); // force StatusInfoMessage to central
  }
};
CycleInfoAlarm cycleAlarm;

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  channelISR(sdev.channel(1),SENS1_PIN,CHANGE);
  channelISR(sdev.channel(1),SENS2_PIN,CHANGE);
  channelISR(sdev.channel(1),SABOTAGE_PIN,CHANGE);
  // activate cycle info message
  if( sdev.getList0().cycleInfoMsg() == true ) {
    sysclock.add(cycleAlarm);
  }
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

