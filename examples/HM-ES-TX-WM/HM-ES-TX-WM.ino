//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

/*
 * Setup defines to configure the library.
 * Note: If you are using the Eclipse Arduino IDE you will need to set the
 * defines in the project properties.
 */
#ifndef __IN_ECLIPSE__
  #define USE_AES
  #define HM_DEF_KEY 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
  #define HM_DEF_KEY_INDEX 0
#endif

#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <TimerOne.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#ifdef USE_OTA_BOOTLOADER
  #define OTA_MODEL_START  0x7ff0 // start address of 2 byte model id in bootloader
  #define OTA_SERIAL_START 0x7ff2 // start address of 10 byte serial number in bootloader
  #define OTA_HMID_START   0x7ffc // start address of 3 byte device id in bootloader
#else
  // device ID
  #define DEVICE_ID HMID(0x90,0x12,0x34)
  // serial number
  #define DEVICE_SERIAL "papa555555"
#endif

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8
// Arduino pin for the counter impulse
// A0 == PIN 14 on Pro Mini
#define INPUT_PIN 3
// we send the counter every 3 minutes
#define MSG_CYCLE seconds2ticks(3*60)

// number of available peers per channel
#define PEERS_PER_CHANNEL 2

// all library classes are placed in the namespace 'as'
using namespace as;

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> RadioSPI;
typedef AskSin<StatusLed,NoBattery,Radio<RadioSPI,2> > Hal;
Hal hal;

class MeterList0Data : public List0Data {
  uint8_t LocalResetDisbale : 1;   // 0x18 - 24
  uint8_t Baudrate          : 8;   // 0x23 - 35
  uint8_t SerialFormat      : 8;   // 0x24 - 36
  uint8_t MeterPowerMode    : 8;   // 0x25 - 37
  uint8_t MeterProtocolMode : 8;   // 0x26 - 38
  uint8_t SamplesPerCycle   : 8;   // 0x27 - 39

public:
  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x18: return sizeof(List0Data) + 0;
      case 0x23: return sizeof(List0Data) + 1;
      case 0x24: return sizeof(List0Data) + 2;
      case 0x25: return sizeof(List0Data) + 3;
      case 0x26: return sizeof(List0Data) + 4;
      case 0x27: return sizeof(List0Data) + 5;
      default:   break;
    }
    return List0Data::getOffset(reg);
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case sizeof(List0Data) + 0:  return 0x18;
      case sizeof(List0Data) + 1:  return 0x23;
      case sizeof(List0Data) + 2:  return 0x24;
      case sizeof(List0Data) + 3:  return 0x25;
      case sizeof(List0Data) + 4:  return 0x26;
      case sizeof(List0Data) + 5:  return 0x27;
      default: break;
    }
    return List0Data::getRegister(offset);
  }
};

class MeterList0 : public ChannelList<MeterList0Data> {
public:
  MeterList0(uint16_t a) : ChannelList(a) {}

  // from List0
  HMID masterid () { return HMID(getByte(1),getByte(2),getByte(3)); }
  void masterid (const HMID& mid) { setByte(1,mid.id0()); setByte(2,mid.id1()); setByte(3,mid.id2()); };

  bool localResetDisable () const { return isBitSet(sizeof(List0Data) + 0,0x01); }
  bool localResetDisable (bool value) const { return setBit(sizeof(List0Data) + 0,0x01,value); }
  uint8_t baudrate () const { return getByte(sizeof(List0Data) + 1); }
  bool baudrate (uint8_t value) const { return setByte(sizeof(List0Data) + 1,value); }
  uint8_t serialFormat () const { return getByte(sizeof(List0Data) + 2); }
  bool serialFormat (uint8_t value) const { return setByte(sizeof(List0Data) + 2,value); }
  uint8_t powerMode () const { return getByte(sizeof(List0Data) + 3); }
  bool powerMode (uint8_t value) const { return setByte(sizeof(List0Data) + 3,value); }
  uint8_t protocolMode () const { return getByte(sizeof(List0Data) + 4); }
  bool protocolMode (uint8_t value) const { return setByte(sizeof(List0Data) + 4,value); }
  uint8_t samplesPerCycle () const { return getByte(sizeof(List0Data) + 5); }
  bool samplesPerCycle (uint8_t value) const { return setByte(sizeof(List0Data) + 5,value); }

  void defaults () {
    ((List0*)this)->defaults();
  }
};

class MeterList1Data {
public:
  uint8_t  AesActive          :1;   // 0x08, s:0, e:1
  uint8_t  MeterType          :8;   // 0x95
  uint8_t  MeterSensibilityIR :8;   // 0x9c
  uint32_t TxThresholdPower   :24;  // 0x7C - 0x7E
  uint8_t  PowerString[16];         // 0x36 - 0x46 : 06 - 21
  uint8_t  EnergyCounterString[16]; // 0x47 - 0x57 : 22 - 37
  uint16_t MeterConstantIR    :16;  // 0x96 - 0x97 : 38 - 39
  uint16_t MeterConstantGas   :16;  // 0x98 - 0x99 : 40 - 41
  uint16_t MeterConstantLed   :16;  // 0x9a - 0x9b : 42 - 43

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x08: return 0;
      case 0x95: return 1;
      case 0x9c: return 2;
      case 0x7c: return 3;
      case 0x7d: return 4;
      case 0x7e: return 5;
      default: break;
    }
    if( reg >= 0x36 && reg <= 0x57 ) {
      return reg - 0x36 + 6;
    }
    if( reg >= 0x96 && reg <= 0x9b ) {
      return reg - 0x96 + 38;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x08;
      case 1:  return 0x95;
      case 2:  return 0x9c;
      case 3:  return 0x7c;
      case 4:  return 0x7d;
      case 5:  return 0x7e;
      default: break;
    }
    if( offset >= 6 && offset <= 37 ) {
      return offset - 6 + 0x36;
    }
    if( offset >= 38 && offset <= 43 ) {
      return offset - 38 + 0x96;
    }
    return 0xff;
  }
};

class MeterList1 : public ChannelList<MeterList1Data> {
public:
  MeterList1(uint16_t a) : ChannelList(a) {}

  bool aesActive () const { return isBitSet(0,0x01); }
  bool aesActive (bool s) const { return setBit(0,0x01,s); }
  uint8_t meterType () const { return getByte(1); }
  bool meterType (uint8_t value) const { return setByte(1,value); }
  uint8_t meterSensibilty () const { return getByte(2); }
  bool meterSensibilty (uint8_t value) const { return setByte(2,value); }
  uint32_t thresholdPower () const { return ((uint32_t)getByte(3)<<16) + ((uint16_t)getByte(4)<<8) + getByte(5); }
  bool thresholdPower (uint32_t value) const { return setByte(3,(value>>16)&0xff) && setByte(4,(value>>8)&0xff) && setByte(5,value&0xff); }

  uint16_t constantIR () const { return ((uint16_t)getByte(38)<<8) + getByte(39); }
  bool constantIR (uint16_t value) const { return setByte(38,(value>>8)&0xff) && setByte(39,value&0xff); }
  uint16_t constantGas () const { return ((uint16_t)getByte(40)<<8) + getByte(41); }
  bool constantGas (uint16_t value) const { return setByte(40,(value>>8)&0xff) && setByte(41,value&0xff); }
  uint16_t constantLed () const { return ((uint16_t)getByte(42)<<8) + getByte(43); }
  bool constantLed (uint16_t value) const { return setByte(42,(value>>8)&0xff) && setByte(43,value&0xff); }

  void defaults () {
    aesActive(false);
    meterType(0xff);
    meterSensibilty(0);
    thresholdPower(100*100);
    constantIR(100);
    constantGas(10);
    constantLed(10000);
  }
};

BatterySensor battery;

class GasPowerEventMsg : public Message {
public:
  void init(uint8_t msgcnt,bool boot,uint32_t counter,uint32_t power) {
    uint8_t cnt1 = (counter >> 24) & 0x7f;
    if( boot == true ) {
      cnt1 |= 0x80;
    }
    Message::init(0x10,msgcnt,0x54,BIDI|WKMEUP,cnt1,(counter >> 16) & 0xff);
    pload[0] = (counter >> 8) & 0xff;
    pload[1] = counter & 0xff;
    pload[2] = (power >> 16) & 0xff;
    pload[3] = (power >> 8) & 0xff;
    pload[4] = power & 0xff;
  }
};

class GasPowerEventCycleMsg : public GasPowerEventMsg {
public:
  void init(uint8_t msgcnt,bool boot,uint32_t counter,uint32_t power) {
    GasPowerEventMsg::init(msgcnt,boot,counter,power);
    typ = 0x53;
  }
};

class PowerEventMsg : public Message {
public:
  void init(uint8_t msgcnt,bool boot,uint32_t counter,uint32_t power) {
    uint8_t cnt1 = (counter >> 16) & 0x7f;
    if( boot == true ) {
      cnt1 |= 0x80;
    }
    Message::init(0x0f,msgcnt,0x5f,BIDI|WKMEUP,cnt1,(counter >> 8) & 0xff);
    pload[0] = counter & 0xff;
    pload[1] = (power >> 16) & 0xff;
    pload[2] = (power >> 8) & 0xff;
    pload[3] = power & 0xff;
  }
};

class PowerEventCycleMsg : public PowerEventMsg {
public:
  void init(uint8_t msgcnt,bool boot,uint32_t counter,uint32_t power) {
    PowerEventMsg::init(msgcnt,boot,counter,power);
    typ = 0x5e;
  }
};

class MeterChannel : public Channel<Hal,MeterList1,EmptyList,List4,PEERS_PER_CHANNEL>, public Alarm {

  const uint32_t maxVal = 838860700;
  uint64_t counterSum;
  volatile uint32_t counter; // declare as volatile because of usage withing interrupt
  Message     msg;
  uint8_t     msgcnt;
  bool        boot;
  
private:

public:
  MeterChannel () : Channel(), Alarm(MSG_CYCLE), counterSum(0), counter(0), msgcnt(0), boot(true) {}
  virtual ~MeterChannel () {}

  uint8_t status () const {
    return 0;
  }

  uint8_t flags () const {
    return battery.low() ? 0x80 : 0x00;
  }

  void next () {
    // only count rotations/flashes and calculate real value when sending, to prevent inaccuracy
    counter++;

    hal.led.ledOn(millis2ticks(300));
    
    #ifndef NDEBUG
      DHEXLN(counter);
    #endif
  }

  virtual void trigger (AlarmClock& clock) {
    tick = MSG_CYCLE;
    clock.add(*this);

    uint32_t consumptionSum;
    uint32_t actualConsumption;

    MeterList1 l1 = getList1();
    uint8_t metertype = l1.meterType(); // cache metertype to reduce eeprom access

    // copy value, to be consistent during calculation (counter may change when an interrupt is triggered)
    uint32_t c = counter;
    counter = 0;
    counterSum = counterSum + c;
    
    uint16_t sigs = 1;
    switch( metertype ) {
      case 1: sigs = l1.constantGas(); break;
      case 2: sigs = l1.constantIR(); break;
      case 4: sigs = l1.constantLed(); break;
      default: break;
    }

    switch( metertype ) {
    case 1:
      consumptionSum = counterSum * sigs;
      actualConsumption = (c * sigs * 10) / (MSG_CYCLE / seconds2ticks(60));

      // TODO handle overflow
      
      ((GasPowerEventCycleMsg&)msg).init(msgcnt++,boot,consumptionSum,actualConsumption);
      break;
    case 2: 
    case 4: 
      // calculate sum
      consumptionSum = (10000 * counterSum / sigs);
      // TODO handle overflow
      
      // calculate consumption whithin the last MSG_CYCLE period
      actualConsumption = (60 * 100000 * c) / (sigs * (MSG_CYCLE / seconds2ticks(60)));
      ((PowerEventCycleMsg&)msg).init(msgcnt++,boot,consumptionSum,actualConsumption);
      break;
    default:
      break;
    }

    device().sendPeerEvent(msg,*this);
    boot = false;
  }
};


MultiChannelDevice<Hal,MeterChannel,2,MeterList0> sdev(0x20);

template <uint8_t pin, void (*isr)(), uint16_t millis>
class ISRWrapper : public Alarm {
  uint8_t curstate;
public:
  ISRWrapper () : Alarm(0), curstate(HIGH) {
    pinMode(pin,INPUT_PULLUP);
  }
  virtual ~ISRWrapper () {}

  bool checkstate () {
    uint8_t oldstate = curstate;
    curstate = digitalRead(pin);
    return curstate != oldstate;
  }

  uint8_t state () const {
    return curstate;
  }

  void attach() {
    enableInterrupt(pin,isr,CHANGE);
  }

  void detach () {
    disableInterrupt(pin);
  }

  void debounce () {
    detach();
    tick = millis2ticks(millis);
    aclock.add(*this);
  }

  virtual void trigger (AlarmClock& clock) {
    attach();
  }
};
void meterISR();
ISRWrapper<INPUT_PIN,meterISR,200> gasISR;
void meterISR () {
  gasISR.debounce();
  if( gasISR.checkstate() ) {
    if( gasISR.state() == LOW ) {
      sdev.channel(1).next();
    }
  }
}


class CfgButton : public Button {
public:
  CfgButton () {
    setLongPressTime(seconds2ticks(3));
  }
  virtual void state (uint8_t s) {
    uint8_t old = Button::state();
    Button::state(s);
    if( s == Button::released ) {
      sdev.startPairing();
    }
    else if( s == longpressed ) {
      if( old == longpressed ) {
        sdev.reset(); // long pressed again - reset
      }
      else {
        hal.led.set(StatusLed::key_long);
      }
    }
  }
};

CfgButton cfgBtn;
void cfgBtnISR () { cfgBtn.check(); }

void setup () {
#ifndef NDEBUG
  Serial.begin(57600);
  DPRINTLN(ASKSIN_PLUS_PLUS_IDENTIFIER);
#endif
  if( storage.setup(sdev.checksum()) == true ) {
    sdev.firstinit();
  }

  hal.led.init(LED_PIN);

  cfgBtn.init(CONFIG_BUTTON_PIN);
  enableInterrupt(CONFIG_BUTTON_PIN,cfgBtnISR,CHANGE);
  hal.radio.init();

#ifdef USE_OTA_BOOTLOADER
  sdev.init(hal,OTA_HMID_START,OTA_SERIAL_START);
  sdev.setModel(OTA_MODEL_START);
#else
  sdev.init(hal,DEVICE_ID,DEVICE_SERIAL);
  sdev.setModel(0x00,0xde);
#endif
  sdev.setFirmwareVersion(0x11);
  sdev.setSubType(DeviceType::PowerMeter);
  sdev.setInfo(0x03,0x01,0x00);

  hal.radio.enable();
  aclock.init();

  gasISR.attach();

  hal.led.set(StatusLed::welcome);
  // set low voltage to 2.2V
  // measure battery every 1h
  battery.init(22,seconds2ticks(60UL*60),aclock);

  // add channel 1 to timer to send event
  aclock.add(sdev.channel(1));
}

void loop() {
  bool worked = aclock.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Sleep>(hal);
  }
}
