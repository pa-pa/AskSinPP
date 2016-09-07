
#include <Led.h>
#include <Debug.h>

#include <AlarmClock.h>
#include <MultiChannelDevice.h>
#include <ChannelList.h>
#include <Message.h>
#include <Button.h>
#include <PinChangeInt.h>
#include <TimerOne.h>
#include <Radio.h>

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#ifdef USE_OTA_BOOTLOADER
  #define OTA_MODEL_START  0x7ff0 // start address of 2 byte model id in bootloader
  #define OTA_SERIAL_START 0x7ff2 // start address of 10 byte serial number in bootloader
  #define OTA_HMID_START   0x7ffc // start address of 3 byte device id in bootloader
#else
  // device ID
  #define DEVICE_ID HMID(0x56,0x78,0x90)
  // serial number
  #define DEVICE_SERIAL "papa222222"
#endif

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8


// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

class MotionList1Data {
public:
  uint8_t EventFilterPeriod : 4;     // 0x01
  uint8_t EventFilterNumber : 4;     // 0x01
  uint8_t MinInterval       : 3;     // 0x02
  uint8_t CaptureWithinInterval : 1; // 0x02
  uint8_t BrightnessFilter  : 4;     // 0x02
  uint8_t AesActive         :1;      // 0x08, s:0, e:1
  uint8_t LedOntime;                 // 0x20

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x01: return 0;
      case 0x02: return 1;
      case 0x08: return 2;
      case 0x20: return 3;
      default: break;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x01;
      case 1:  return 0x02;
      case 2:  return 0x08;
      case 3:  return 0x20;
      default: break;
    }
    return 0xff;
  }
};

class MotionList1 : public ChannelList<MotionList1Data> {
public:
  MotionList1(uint16_t a) : ChannelList(a) {}

  uint8_t eventFilterPeriod () const { return getByte(0,0x0f,0); }
  bool eventFilterPeriod (uint8_t value) const { return setByte(0,value,0x0f,0); }
  uint8_t eventFilterNumber () const { return getByte(0,0xf0,4); }
  bool eventFilterNumber (uint8_t value) const { return setByte(0,value,0xf0,4); }

  uint8_t minInterval () const { return getByte(1,0x07,0); }
  bool minInterval (uint8_t value) const { return setByte(1,value,0x07,0); }
  bool captureWithinInterval () const { return isBitSet(2,0x80); }
  bool captureWithinInterval (bool value) const { return setBit(2,0x80,value); }
  uint8_t brightnessFilter () const { return getByte(0,0xf0,4); }
  bool brightnessFilter (uint8_t value) const { return setByte(0,value,0xf0,4); }

  bool aesActive () const { return isBitSet(2,0x01); }
  bool aesActive (bool s) const { return setBit(2,0x01,s); }

  uint8_t ledOntime () const { return getByte(3); }
  bool ledOntime (uint8_t value) const { return setByte(3,value); }

  void defaults () {
    eventFilterPeriod(1);
    eventFilterNumber(1);
    minInterval(4);
    captureWithinInterval(false);
    brightnessFilter(7);
    aesActive(false);
    ledOntime(100);
  }
};

class MotionEventMsg : public Message {
public:
  void init(uint8_t msgcnt,uint8_t ch,uint8_t counter,uint8_t brightness,uint8_t next) {
    Message::init(12,msgcnt,0x41, Message::BIDI,ch,brightness);
    pload[0] = next;
  }
};



class MotionChannel : public Channel<MotionList1,EmptyList,List4,PEERS_PER_CHANNEL>, public Alarm {

private:
  MotionEventMsg   msg;
  uint8_t          msgcnt;
  uint8_t          counter;

public:
  MotionChannel () : Channel(), Alarm(0), msgcnt(0), counter(0) {}
  virtual ~MotionChannel () {}

  uint8_t status () const {
    return brightness();
  }

  uint8_t flags () const {
    return 0;
  }

  uint8_t brightness () const {
    return 255;
  }

  virtual void trigger (AlarmClock& clock) {
    DPRINTLN("Motion");
    msg.init(++msgcnt,number(),++counter,brightness(),0);
    bool sendtopeer=false;
    for( int i=0; i<peers(); ++i ){
      Peer p = peer(i);
      if( p.valid() == true ) {
        device().send(msg,p);
        sendtopeer = true;
      }
    }
    if( sendtopeer == false ) {
      device().send(msg,device().getMasterID());
    }
  }

  void motionDetected () {
    aclock.add(*this);
  }

};

MultiChannelDevice<MotionChannel,1> sdev(0x20);
void motionISR () { sdev.channel(1).motionDetected(); }


class CfgButton : public Button {
public:
  virtual void state (uint8_t s) {
    uint8_t old = Button::state();
    Button::state(s);
    if( s == Button::released && old == Button::pressed ) {
      sdev.startPairing();
    }
    else if( s== longpressed ) {
    }
    else if( s == Button::longlongpressed ) {
      sdev.reset();
    }
  }
};

CfgButton cfgBtn;
void cfgBtnISR () { cfgBtn.pinChange(); }

void setup () {
#ifndef NDEBUG
  Serial.begin(57600);
#endif
  sled.init(LED_PIN);

  cfgBtn.init(CONFIG_BUTTON_PIN);
  attachPinChangeInterrupt(CONFIG_BUTTON_PIN,cfgBtnISR,CHANGE);
  radio.init();

  if( eeprom.setup() == true ) {
    sdev.firstinit();
  }

#ifdef USE_OTA_BOOTLOADER
  sdev.init(radio,OTA_HMID_START,OTA_SERIAL_START);
  sdev.setModel(OTA_MODEL_START);
#else
  sdev.init(radio,DEVICE_ID,DEVICE_SERIAL);
  sdev.setModel(0x00,0x3d);
#endif
  sdev.setFirmwareVersion(0x10);
  sdev.setSubType(0x70);
  sdev.setInfo(0x03,0x01,0x00);

  radio.enableGDO0Int();
  aclock.init();

  attachPinChangeInterrupt(14,motionISR,FALLING);

  sled.set(StatusLed::welcome);
}

void loop() {
  aclock.runready();
  sdev.pollRadio();
}
