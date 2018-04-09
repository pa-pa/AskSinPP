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

// define device configuration bytes
#define DEVICE_CONFIG CFG_STEPUP_OFF,22,19

// 24 0030 4D455130323134373633 80 910101

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

#define SENS1_PIN 14
#define SENS2_PIN 15
#define SABOTAGE_PIN 16

// activate additional open detection by using a third sensor pins
// #define SENS3_PIN 16
// #define SABOTAGE_PIN 0

// number of available peers per channel
#define PEERS_PER_CHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x09,0x56,0x34},       // Device ID
    "papa222111",           // Device Serial
    {0x00,0xC3},            // Device Model
    0x22,                   // Firmware Version
    as::DeviceType::ThreeStateSensor, // Device Type
    {0x01,0x00}             // Info Bytes
};

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
  void init (const HMID& id) {
    BaseHal::init(id);
    // measure battery every 1h
    battery.init(seconds2ticks(60UL*60),sysclock);
  }
} hal;

DEFREGISTER(Reg0,DREG_INTKEY,DREG_CYCLICINFOMSG,MASTERID_REGS,DREG_TRANSMITTRYMAX,DREG_SABOTAGEMSG)
class RHSList0 : public RegList0<Reg0> {
public:
  RHSList0(uint16_t addr) : RegList0<Reg0>(addr) {}
  void defaults () {
    clear();
    cycleInfoMsg(true);
    transmitDevTryMax(6);
    sabotageMsg(true);
  }
};

DEFREGISTER(Reg1,CREG_AES_ACTIVE,CREG_MSGFORPOS,CREG_EVENTDELAYTIME,CREG_LEDONTIME,CREG_TRANSMITTRYMAX)
class RHSList1 : public RegList1<Reg1> {
public:
  RHSList1 (uint16_t addr) : RegList1<Reg1>(addr) {}
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

#ifdef SENS3_PIN
class ThreePinPosition : public Position {
  // pin mapping can be changed by bootloader config data
  // map pins to pos     00   01   10   11
  uint8_t posmap[4] = {State::PosC,State::PosC,State::PosB,State::PosA};
  uint8_t sens1, sens2, sens3;
public:
  ThreePinPosition () : sens1(0), sens2(0), sens3(0) { _present = true; }

  void init (uint8_t pin1,uint8_t pin2,uint8_t pin3, const uint8_t* pmap) {
    memcpy(posmap,pmap,4);
    init(pin1,pin2,pin3);
  }

  void init (uint8_t pin1,uint8_t pin2,uint8_t pin3) {
    sens1=pin1;
    sens2=pin2;
    sens3=pin3;
  }

  void measure (__attribute__((unused)) bool async=false) {
    if (readPin(sens1) == LOW) {
      _position = posmap[1];
    } else if (readPin(sens2) == LOW) {
      _position = posmap[2];
    } else if (readPin(sens3) == LOW) {
      _position = posmap[3];
    }
    else {
      _position = posmap[0];
    }
  }

  uint8_t readPin(uint8_t pinnr) {
    uint8_t value=0;
    pinMode(pinnr,INPUT_PULLUP);
    value = digitalRead(pinnr);
    pinMode(pinnr,OUTPUT);
    digitalWrite(pinnr,LOW);
    return value;
  }
};

template <class HALTYPE,class List0Type,class List1Type,class List4Type,int PEERCOUNT>
class ThreePinChannel : public ThreeStateGenericChannel<ThreePinPosition,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> {
public:
  typedef ThreeStateGenericChannel<ThreePinPosition,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> BaseChannel;

  ThreePinChannel () : BaseChannel() {};
  ~ThreePinChannel () {}

  void init (uint8_t pin1,uint8_t pin2,uint8_t pin3,uint8_t sabpin,const uint8_t* pmap) {
    BaseChannel::init(sabpin);
    BaseChannel::possens.init(pin1,pin2,pin3,pmap);
  }

};

typedef ThreePinChannel<Hal,RHSList0,RHSList1,DefList4,PEERS_PER_CHANNEL> ChannelType;
#else
typedef ThreeStateChannel<Hal,RHSList0,RHSList1,DefList4,PEERS_PER_CHANNEL> ChannelType;
#endif

class RHSType : public ThreeStateDevice<Hal,ChannelType,1,RHSList0> {
public:
  typedef ThreeStateDevice<Hal,ChannelType,1,RHSList0> TSDevice;
  RHSType(const DeviceInfo& info,uint16_t addr) : TSDevice(info,addr) {}
  virtual ~RHSType () {}

  virtual void configChanged () {
    TSDevice::configChanged();
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

RHSType sdev(devinfo,0x20);
ConfigButton<RHSType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  const uint8_t posmap[4] = {Position::State::PosB,Position::State::PosC,Position::State::PosA,Position::State::PosB};
#ifdef SENS3_PIN
  sdev.channel(1).init(SENS1_PIN,SENS2_PIN,SENS3_PIN,SABOTAGE_PIN,posmap);
#else
  sdev.channel(1).init(SENS1_PIN,SENS2_PIN,SABOTAGE_PIN,posmap);
#endif
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
    hal.activity.savePower<Sleep<> >(hal);
  }
}

