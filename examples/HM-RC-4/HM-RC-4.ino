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
#include <SPI.h>  // after including SPI Library - we can use LibSPI class
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
  #define DEVICE_ID HMID(0x78,0x90,0x12)
  // serial number
  #define DEVICE_SERIAL "papa333333"
#endif

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
#define LED_PIN2 5
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8
// Arduino pins for the buttons
// A0,A1,A2,A3 == PIN 14,15,16,17 on Pro Mini
#define BTN1_PIN 14
#define BTN2_PIN 15
#define BTN3_PIN 16
#define BTN4_PIN 17


// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

/**
 * Configure the used hardware
 */
// typedef AvrSPI<10,11,12,13> RadioSPI;
typedef LibSPI<10> RadioSPI;
class Hal : public AskSin<DualStatusLed,BatterySensor,Radio<RadioSPI,2> > {
public:
  AlarmClock btncounter;  // extra clock to count button press events
} hal;


class BtnList1Data {
public:
  uint8_t Unused1          : 4;     // 0x04
  uint8_t LongPressTime    : 4;     // 0x04
  uint8_t AesActive        : 1;     // 0x08, s:0, e:1
  uint8_t DoublePressTime  : 4;     // 0x09
  uint8_t Unused2          : 4;     // 0x09

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x04: return 0;
      case 0x08: return 1;
      case 0x09: return 2;
      default: break;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x04;
      case 1:  return 0x08;
      case 2:  return 0x09;
      default: break;
    }
    return 0xff;
  }
};

class BtnList1 : public ChannelList<BtnList1Data> {
public:
  BtnList1(uint16_t a) : ChannelList(a) {}

  uint8_t longPressTime () const { return getByte(0,0xf0,4); }
  bool longPressTime (uint8_t value) const { return setByte(0,value,0xf0,4); }

  bool aesActive () const { return isBitSet(1,0x01); }
  bool aesActive (bool s) const { return setBit(1,0x01,s); }

  uint8_t doublePressTime () const { return getByte(2,0x0f,0); }
  bool doublePressTime (uint8_t value) const { return setByte(2,value,0x0f,0); }


  void defaults () {
    longPressTime(1);
    aesActive(false);
    doublePressTime(0);
  }
};

class BtnEventMsg : public Message {
public:
  void init(uint8_t msgcnt,uint8_t ch,uint8_t counter,bool lg,bool lowbat) {
    uint8_t flags = lg ? 0x40 : 0x00;
    if( lowbat == true ) {
      flags |= 0x80; // low battery
    }
    Message::init(0xb,msgcnt,0x40, Message::BIDI,(ch & 0x3f) | flags,counter);
  }
};



class BtnChannel : public Channel<Hal,BtnList1,EmptyList,List4,PEERS_PER_CHANNEL>, public Button {

private:
  uint8_t       msgcnt;
  uint8_t       repeatcnt;
  volatile bool isr;

public:
  BtnChannel () : Channel(), msgcnt(0), repeatcnt(0), isr(false) {}
  virtual ~BtnChannel () {}

  Button& button () { return *(Button*)this; }

  uint8_t status () const {
    return 0;
  }

  uint8_t flags () const {
    return 0;
  }

  virtual void state(uint8_t s) {
    DHEX(number());
    Button::state(s);
    if( s == released ) {
      repeatcnt=0;
      BtnEventMsg& msg = (BtnEventMsg&)device().message();
      msg.init(++msgcnt,number(),repeatcnt,false,hal.battery.low());
      device().sendPeerEvent(msg,*this);
      --hal.btncounter;
    }
    else if( s == longpressed ) {
      BtnEventMsg& msg = (BtnEventMsg&)device().message();
      msg.init(++msgcnt,number(),repeatcnt++,true,hal.battery.low());
      device().sendPeerEvent(msg,*this);
      --hal.btncounter;
    }
  }

  void pinchanged () {
    isr = true;
  }

  bool checkpin () {
    bool result = isr;
    if( isr == true ) {
      isr = false;
      Button::check();
    }
    return result;
  }
};


MultiChannelDevice<Hal,BtnChannel,4> sdev(0x20);

class CfgButton : public Button {
public:
  CfgButton () {
    setLongPressTime(seconds2ticks(3));
  }
  virtual void state (uint8_t s) {
    uint8_t old = Button::state();
    Button::state(s);
    if( s == released ) {
      sdev.startPairing();
    }
    else if( s == longpressed ) {
      if( old == longpressed ) {
        sdev.reset(); // long pressed again - reset
      }
      else {
        sdev.led().set(StatusLed::key_long);
      }
    }
  }
};

CfgButton cfgBtn;
void cfgBtnISR () { cfgBtn.check(); }

void btn1ISR () { sdev.channel(1).pinchanged(); }
void btn2ISR () { sdev.channel(2).pinchanged(); }
void btn3ISR () { sdev.channel(3).pinchanged(); }
void btn4ISR () { sdev.channel(4).pinchanged(); }

void setup () {
#ifndef NDEBUG
  Serial.begin(57600);
  DPRINTLN(ASKSIN_PLUS_PLUS_IDENTIFIER);
#endif
  if( storage.setup(sdev.checksum()) == true ) {
    sdev.firstinit();
  }

  sdev.channel(1).button().init(BTN1_PIN);
  sdev.channel(2).button().init(BTN2_PIN);
  sdev.channel(3).button().init(BTN3_PIN);
  sdev.channel(4).button().init(BTN4_PIN);
  enableInterrupt(BTN1_PIN,btn1ISR,CHANGE);
  enableInterrupt(BTN2_PIN,btn2ISR,CHANGE);
  enableInterrupt(BTN3_PIN,btn3ISR,CHANGE);
  enableInterrupt(BTN4_PIN,btn4ISR,CHANGE);

  cfgBtn.init(CONFIG_BUTTON_PIN);
  enableInterrupt(CONFIG_BUTTON_PIN,cfgBtnISR,CHANGE);
  hal.radio.init();

#ifdef USE_OTA_BOOTLOADER
  sdev.init(hal,OTA_HMID_START,OTA_SERIAL_START);
  sdev.setModel(OTA_MODEL_START);
#else
  sdev.init(hal,DEVICE_ID,DEVICE_SERIAL);
  sdev.setModel(0x00,0x08);
#endif
  sdev.setFirmwareVersion(0x11);
  sdev.setSubType(DeviceType::Remote);
  sdev.setInfo(0x04,0x00,0x00);

  hal.radio.enable();
  aclock.init();

  hal.led.init(LED_PIN2,LED_PIN);
  hal.led.set(StatusLed::welcome);
  // get new battery value after 50 key press
  hal.battery.init(22,50,hal.btncounter);
}

void loop() {
  bool pinchanged = false;
  for( int i=1; i<=sdev.channels(); ++i ) {
    if( sdev.channel(i).checkpin() == true) {
      pinchanged = true;
    }
  }
  bool worked = aclock.runready() || hal.btncounter.runready();
  bool poll = sdev.pollRadio();
  if( pinchanged == false && worked == false && poll == false ) {
    hal.activity.savePower<Sleep>(hal);
  }
}
