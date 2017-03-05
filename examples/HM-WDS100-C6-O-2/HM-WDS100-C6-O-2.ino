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
#include <BatterySensor.h>

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#ifdef USE_OTA_BOOTLOADER
  #define OTA_MODEL_START  0x7ff0 // start address of 2 byte model id in bootloader
  #define OTA_SERIAL_START 0x7ff2 // start address of 10 byte serial number in bootloader
  #define OTA_HMID_START   0x7ffc // start address of 3 byte device id in bootloader
#else
  // device ID
  #define DEVICE_ID HMID(0x09,0x21,0x43)
  // serial number
  #define DEVICE_SERIAL "papa666666"
#endif

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

// number of available peers per channel
#define PEERS_PER_CHANNEL 2

// all library classes are placed in the namespace 'as'
using namespace as;

/**
 * Configure the used hardware
 */
typedef SPI<10,11,12,13,2> ArduinoSPI;
typedef AskSin<StatusLed,BatterySensor,Radio<ArduinoSPI> > Hal;
Hal hal;

class Wds100List0Data : public List0Data {
  uint8_t LiveModeRx        : 1;   // 0x01 - 01
  uint8_t CycleInfoMsgDis   : 1;   // 0x11 - 17
  uint8_t LocalResetDisbale : 1;   // 0x18 - 24

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x01: return sizeof(List0Data) + 0;
      case 0x11: return sizeof(List0Data) + 1;
      case 0x18: return sizeof(List0Data) + 2;
      default:   break;
    }
    return List0Data::getOffset(reg);
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case sizeof(List0Data) + 0:  return 0x01;
      case sizeof(List0Data) + 1:  return 0x11;
      case sizeof(List0Data) + 2:  return 0x18;
      default: break;
    }
    return List0Data::getRegister(offset);
  }
};

class Wds100List0 : public ChannelList<Wds100List0Data> {
public:
  Wds100List0(uint16_t a) : ChannelList(a) {}

  // from List0
  HMID masterid () { return HMID(getByte(1),getByte(2),getByte(3)); }
  void masterid (const HMID& mid) { setByte(1,mid.id0()); setByte(2,mid.id1()); setByte(3,mid.id2()); };

  bool liveModeRx () const { return getByte(sizeof(List0Data) + 0); }
  bool liveModeRx (bool value) const { return setByte(sizeof(List0Data) + 0,value); }
  uint8_t cycleInfoMsgDis () const { return getByte(sizeof(List0Data) + 1); }
  bool cycleInfoMsgDis (uint8_t value) const { return setByte(sizeof(List0Data) + 1,value); }
  bool localResetDisable () const { return isBitSet(sizeof(List0Data) + 2,0x01); }
  bool localResetDisable (bool value) const { return setBit(sizeof(List0Data) + 2,0x01,value); }

  void defaults () {
    ((List0*)this)->defaults();
    liveModeRx(false);
    localResetDisable(false);
    cycleInfoMsgDis(0);
  }
};

class Wds100List1Data {
public:
  uint8_t  SunshineThreshold     :8;   // 0x05
  uint8_t  WindSpeedResultSource :8;   // 0x0a

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x05: return 0;
      case 0x0a: return 1;
      default: break;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x05;
      case 1:  return 0x0a;
      default: break;
    }
    return 0xff;
  }
};

class Wds100List1 : public ChannelList<Wds100List1Data> {
public:
  Wds100List1(uint16_t a) : ChannelList(a) {}

  uint8_t sunshineThreshold () const { return getByte(0); }
  bool sunshineThreshold (uint8_t value) const { return setByte(0,value); }
  uint8_t windSpeedResultSource () const { return getByte(1); }
  bool windSpeedResultSource (uint8_t value) const { return setByte(1,value); }

  bool aesActive () const { return false; }

  void defaults () {
    sunshineThreshold(0);
    windSpeedResultSource(0);
  }
};

class Wds100EventMsg : public Message {
public:
  void init(uint8_t msgcnt,uint16_t temp,uint8_t humidity,
      bool raining,uint16_t raincounter,
      uint16_t windspeed,uint8_t winddir,uint8_t winddirrange,
      uint8_t sunshineduration,uint8_t brightness) {

    uint8_t temp1 = (temp >> 8) & 0x7f;
    Message::init(0x13,msgcnt,0x70,Message::BIDI,temp1,temp & 0xff);
    pload[0] = humidity;
    pload[1] = ((raincounter >> 8) & 0xff) | (raining << 7);
    pload[2] = raincounter & 0xff;
    pload[3] = ((windspeed >> 8) & 0xff) | (winddirrange << 6);
    pload[4] = windspeed & 0xff;
    pload[5] = winddir;
    pload[6] = sunshineduration;
    pload[7] = brightness;
  }
};


class Wds100Channel : public Channel<Hal,Wds100List1,EmptyList,List4,PEERS_PER_CHANNEL>, public Alarm {

  Wds100EventMsg     msg;
  uint8_t     msgcnt;

private:

public:
  Wds100Channel () : Channel(), Alarm(seconds2ticks(120)), msgcnt(0) {}
  virtual ~Wds100Channel () {}

  uint8_t status () const {
    return 0;
  }

  uint8_t flags () const {
    // ??? no idea if this is correct
    return hal.battery.low() ? 0x80 : 0x00;
  }

  virtual void trigger (AlarmClock& clock) {
    tick = seconds2ticks(120); // next message in 120 seconds
    clock.add(*this);
    // fake some values
    uint16_t temp = 22;
    uint8_t humidity = 80;
    bool raining = false;
    uint16_t raincounter = 1000;
    uint16_t windspeed = 5000;
    uint8_t winddir = 45;
    uint8_t winddirrange = 0;
    uint8_t sunshineduration = 10;
    uint8_t brightness = 0xab;
    msg.init(msgcnt++,temp,humidity,raining,raincounter,windspeed,winddir,winddirrange,sunshineduration,brightness);
    device().sendPeerEvent(msg,*this);
  }
};


MultiChannelDevice<Hal,Wds100Channel,1> sdev(0x20);


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
  sdev.setModel(0x00,0xae);
#endif
  // TODO check with version to use
  sdev.setFirmwareVersion(0x11);
  sdev.setSubType(DeviceType::THSensor);
  sdev.setInfo(0x03,0x01,0x00);

  hal.radio.enable();
  aclock.init();

  hal.led.set(StatusLed::welcome);
  // set low voltage to 2.2V
  // measure battery every 1h
  hal.battery.init(22,seconds2ticks(60UL*60),aclock);

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
