//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#include <PinChangeInt.h>
#include <TimerOne.h>
#include <AskSinPP.h>

#include <Debug.h>
#include <Activity.h>

#include <Led.h>
#include <AlarmClock.h>
#include <MultiChannelDevice.h>
#include <ChannelList.h>
#include <Message.h>
#include <Button.h>
#include <Radio.h>
#include <BatterySensor.h>

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
// Arduino pin for the PIR
// A0 == PIN 14 on Pro Mini
#define PIR_PIN 14
#define PIR_ENABLE_PIN 15


#define BATTERY_LOW 22
#define BATTERY_CRITICAL 19


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
  bool captureWithinInterval () const { return isBitSet(1,0x08); }
  bool captureWithinInterval (bool value) const { return setBit(1,0x08,value); }
  uint8_t brightnessFilter () const { return getByte(1,0xf0,4); }
  bool brightnessFilter (uint8_t value) const { return setByte(1,value,0xf0,4); }

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

// BatterySensor battery;
// BatterySensorExt battery;
BatterySensorUni battery(15,7); // A1 & D7

class MotionEventMsg : public Message {
public:
  void init(uint8_t msgcnt,uint8_t ch,uint8_t counter,uint8_t brightness,uint8_t next) {
    Message::init(0xd,msgcnt,0x41,Message::BIDI,ch & 0x3f,counter);
    pload[0] = brightness;
    pload[1] = (next+4) << 4;
  }
};

class MotionChannel : public Channel<MotionList1,EmptyList,List4,PEERS_PER_CHANNEL>, public Alarm {

  class QuietMode : public Alarm {
  public:
    bool  enabled;
    bool  motion;
    MotionChannel& channel;
    QuietMode (MotionChannel& c) : Alarm(0), enabled(false), motion(false), channel(c) {}
    virtual ~QuietMode () {}
    virtual void trigger (AlarmClock& clock) {
      DPRINTLN("minInterval End");
      enabled = false;
      channel.pirPowerOn();
      if( motion == true ) {
        motion = false;
        channel.motionDetected();
      }
    }
  };

  // send the brightness every 4 minutes to the master
  class Cycle : public Alarm {
  public:
    MotionChannel& channel;
    Cycle (MotionChannel& c) : Alarm(seconds2ticks(4*60)), channel(c) {}
    virtual ~Cycle () {}
    virtual void trigger (AlarmClock& clock) {
      tick = seconds2ticks(4*60);
      clock.add(*this);
      Device& d = channel.device();
      d.sendInfoActuatorStatus(d.getMasterID(),d.nextcount(),channel);
    }
  };

  // return timer ticks
  uint32_t getMinInterval () {
    switch( getList1().minInterval() ) {
      case 0: return seconds2ticks(15);
      case 1: return seconds2ticks(30);
      case 2: return seconds2ticks(60);
      case 3: return seconds2ticks(120);
    }
    return seconds2ticks(240);
  }

private:
  MotionEventMsg   msg;
  uint8_t          msgcnt;
  uint8_t          counter;
  QuietMode        quiet;
  Cycle            cycle;

public:
  MotionChannel () : Channel(), Alarm(0), msgcnt(0), counter(0), quiet(*this), cycle(*this) {
    aclock.add(cycle);
#ifdef PIR_ENABLE_PIN
    pinMode(PIR_ENABLE_PIN,OUTPUT);
#endif
    pirPowerOn();
  }
  virtual ~MotionChannel () {}

  uint8_t status () const {
    return brightness();
  }

  uint8_t flags () const {
    return battery.low() ? 0x80 : 0x00;
  }

  uint8_t brightness () const {
    static uint8_t bvalue = 25;
    static uint8_t bx = -5;
    bvalue += bx;
    if( bvalue == 0 || bvalue == 255 ) {
      bx = -bx;
    }
    return bvalue;
  }

  void pirPowerOn () {
#ifdef PIR_ENABLE_PIN
    digitalWrite(PIR_ENABLE_PIN,HIGH);
#endif
  }

  void pirPowerOff () {
#ifdef PIR_ENABLE_PIN
    digitalWrite(PIR_ENABLE_PIN,LOW);
#endif
  }

  // this runs synch to application
  virtual void trigger (AlarmClock& clock) {
    if( quiet.enabled == false ) {
      DPRINTLN("Motion");
      // start timer to end quiet interval
      quiet.tick = getMinInterval();
      quiet.enabled = true;
      aclock.add(quiet);
      // blink led
      if( sled.active() == false ) {
        sled.ledOn( centis2ticks(getList1().ledOntime()) / 2);
      }
      msg.init(++msgcnt,number(),++counter,brightness(),getList1().minInterval());
      device().sendPeerEvent(msg,*this);
      // if we should not capture during interval - power off
      if ( getList1().captureWithinInterval() == false ) {
        pirPowerOff();
      }
    }
    else if ( getList1().captureWithinInterval() == true ) {
      // we have had a motion during quiet interval
      quiet.motion = true;
      // we can now power off the hardware
      pirPowerOff();
    }
  }

  // runs in interrupt
  void motionDetected () {
    // cancel may not needed but anyway
    aclock.cancel(*this);
    // activate motion message handler
    aclock.add(*this);
  }
};


MultiChannelDevice<MotionChannel,1> sdev(0x20);
void motionISR () { sdev.channel(1).motionDetected(); }


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
        sled.set(StatusLed::key_long);
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
  if( eeprom.setup(sdev.checksum()) == true ) {
    sdev.firstinit();
  }

  sled.init(LED_PIN);

  cfgBtn.init(CONFIG_BUTTON_PIN);
  attachPinChangeInterrupt(CONFIG_BUTTON_PIN,cfgBtnISR,CHANGE);
  radio.init();

#ifdef USE_OTA_BOOTLOADER
  sdev.init(radio,OTA_HMID_START,OTA_SERIAL_START);
  sdev.setModel(OTA_MODEL_START);
#else
  sdev.init(radio,DEVICE_ID,DEVICE_SERIAL);
  sdev.setModel(0x00,0x4a);
#endif
  sdev.setFirmwareVersion(0x16);
  // TODO check sub type and infos
  sdev.setSubType(Device::MotionDetector);
  sdev.setInfo(0x01,0x01,0x00);

  radio.enableGDO0Int();
  aclock.init();

  pinMode(PIR_PIN,INPUT);
#if PIR_PIN == 3
  attachInterrupt(digitalPinToInterrupt(3), motionISR, RISING);
#else
  attachPinChangeInterrupt(PIR_PIN,motionISR,RISING);
#endif
  sled.set(StatusLed::welcome);
  // set low voltage to 2.2V
  // measure battery every 1h
  //battery.init(BATTERY_LOW,seconds2ticks(60UL*60));
  // init for external measurement
  //battery.init(BATTERY_LOW,seconds2ticks(60UL*60),refvoltage,divider);
  // UniversalSensor setup
  battery.init(BATTERY_LOW,seconds2ticks(60UL*60));
  battery.critical(BATTERY_CRITICAL);
}

void loop() {
  bool worked = aclock.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    // deep discharge protection
    // if we drop below critical battery level - switch off all and sleep forever
    if( battery.critical() ) {
      sdev.channel(1).pirPowerOff();
      radio.setIdle();
      // this call will never return
      activity.sleepForever();
    }
    // if nothing to do - go sleep
    activity.savePower<Sleep>();
  }
}
