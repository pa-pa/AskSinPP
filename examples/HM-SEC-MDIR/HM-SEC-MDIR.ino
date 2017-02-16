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

#include <AskSinPP.h>
#include <PinChangeInt.h>
#include <TimerOne.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <BatterySensor.h>

#include <TSL2561.h>
#include <Wire.h>

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


#define BATTERY_LOW 22
#define BATTERY_CRITICAL 19


// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

/**
 * Configure the used hardware
 */
typedef AskSin<StatusLed,BatterySensor,CC1101> Hal;
Hal hal;

// Create an SFE_TSL2561 object, here called "light":
TSL2561 light;

void motionISR ();

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

class MotionEventMsg : public Message {
public:
  void init(uint8_t msgcnt,uint8_t ch,uint8_t counter,uint8_t brightness,uint8_t next) {
    Message::init(0xd,msgcnt,0x41,Message::BIDI|Message::WKMEUP,ch & 0x3f,counter);
    pload[0] = brightness;
    pload[1] = (next+4) << 4;
  }
};

class MotionChannel : public Channel<Hal,MotionList1,EmptyList,List4,PEERS_PER_CHANNEL>, public Alarm {

  class QuietMode : public Alarm {
  public:
    bool  enabled;
    bool  motion;
    MotionChannel& channel;
    QuietMode (MotionChannel& c) : Alarm(0), enabled(false), motion(false), channel(c) {}
    virtual ~QuietMode () {}
    virtual void trigger (AlarmClock& clock) {
      DPRINTLN(F("minInterval End"));
      enabled = false;
      if( motion == true ) {
        motion = false;
        channel.motionDetected();
      }
    }
  };

  // send the brightness every 4 minutes to the master
  #define LIGHTCYCLE seconds2ticks(4*60)
  class Cycle : public Alarm {
  public:
    MotionChannel& channel;
    Cycle (MotionChannel& c) : Alarm(LIGHTCYCLE), channel(c) {}
    virtual ~Cycle () {}
    virtual void trigger (AlarmClock& clock) {
      tick = LIGHTCYCLE;
      clock.add(*this);
      channel.sendState();
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
    pinMode(PIR_PIN,INPUT);
    pirInterruptOn();
  }
  virtual ~MotionChannel () {}

  uint8_t status () const {
    return brightness();
  }

  uint8_t flags () const {
    return hal.battery.low() ? 0x80 : 0x00;
  }

  void sendState () {
    pirInterruptOff();
    Device<Hal>& d = device();
    d.sendInfoActuatorStatus(d.getMasterID(),d.nextcount(),*this);
    pirInterruptOn();
  }

  uint8_t brightness () const {
    static uint16_t maxvalue = 0;
    uint8_t value = 0;
    unsigned int data0, data1;
    if (light.getData(data0,data1)) {
      double lux;    // Resulting lux value
      light.getLux (data0,data1,lux);
      uint16_t current = (uint16_t)lux;
      DPRINT(F("light: ")); DHEXLN(current);
      if( maxvalue < current ) {
        maxvalue = current;
      }
      value = 200UL * current / maxvalue;
    }
    return value;
  }

  void pirInterruptOn () {
#if PIR_PIN == 3
    attachInterrupt(digitalPinToInterrupt(3), motionISR, RISING);
#else
    attachPinChangeInterrupt(PIR_PIN,motionISR,RISING);
#endif
  }

  void pirInterruptOff () {
#if PIR_PIN == 3
    detachInterrupt(digitalPinToInterrupt(3));
#else
    detachPinChangeInterrupt(PIR_PIN);
#endif
  }

  // this runs synch to application
  virtual void trigger (AlarmClock& clock) {
    if( quiet.enabled == false ) {
      DPRINTLN(F("Motion"));
      // start timer to end quiet interval
      quiet.tick = getMinInterval();
      quiet.enabled = true;
      aclock.add(quiet);
      // blink led
      if( hal.led.active() == false ) {
        hal.led.ledOn( centis2ticks(getList1().ledOntime()) / 2);
      }
      msg.init(++msgcnt,number(),++counter,brightness(),getList1().minInterval());
      device().sendPeerEvent(msg,*this);
    }
    else if ( getList1().captureWithinInterval() == true ) {
      // we have had a motion during quiet interval
      quiet.motion = true;
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


MultiChannelDevice<Hal,MotionChannel,1> sdev(0x20);
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
  if( eeprom.setup(sdev.checksum()) == true ) {
    sdev.firstinit();
  }

  light.begin();
  // If gain = false (0), device is set to low gain (1X)
  // If gain = high (1), device is set to high gain (16X)
  // If time = 0, integration will be 13.7ms
  // If time = 1, integration will be 101ms
  // If time = 2, integration will be 402ms
  // If time = 3, use manual start / stop to perform your own integration
  light.setTiming(0,2); //gain,time);
  light.setPowerUp();

  hal.led.init(LED_PIN);

  cfgBtn.init(CONFIG_BUTTON_PIN);
  attachPinChangeInterrupt(CONFIG_BUTTON_PIN,cfgBtnISR,CHANGE);
  hal.radio.init();

#ifdef USE_OTA_BOOTLOADER
  sdev.init(hal,OTA_HMID_START,OTA_SERIAL_START);
  sdev.setModel(OTA_MODEL_START);
#else
  sdev.init(hal,DEVICE_ID,DEVICE_SERIAL);
  sdev.setModel(0x00,0x4a);
#endif
  sdev.setFirmwareVersion(0x16);
  // TODO check sub type and infos
  sdev.setSubType(DeviceType::MotionDetector);
  sdev.setInfo(0x01,0x01,0x00);

  hal.radio.enableGDO0Int();
  aclock.init();

  hal.led.set(StatusLed::welcome);
  // set low voltage to 2.2V
  // measure battery every 1h
  //battery.init(BATTERY_LOW,seconds2ticks(60UL*60));
  // init for external measurement
  //battery.init(BATTERY_LOW,seconds2ticks(60UL*60),refvoltage,divider);
  // UniversalSensor setup
  hal.battery.init(BATTERY_LOW,seconds2ticks(60UL*60),aclock);
  hal.battery.critical(BATTERY_CRITICAL);
}

void loop() {
  bool worked = aclock.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    // deep discharge protection
    // if we drop below critical battery level - switch off all and sleep forever
    if( hal.battery.critical() ) {
      // this call will never return
      hal.activity.sleepForever(hal);
    }
    // if nothing to do - go sleep
    hal.activity.savePower<Sleep>(hal);
  }
}
