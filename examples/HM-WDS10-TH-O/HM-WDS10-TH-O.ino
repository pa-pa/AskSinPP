
#include <Debug.h>
// we want to sleep to save power
#define POWER_SLEEP 1
#include <Activity.h>

#include <Led.h>
#include <AlarmClock.h>
#include <MultiChannelDevice.h>
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
  #define DEVICE_ID HMID(0x34,0x56,0x78)
  // serial number
  #define DEVICE_SERIAL "papa111111"
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

class WeatherEventMsg : public Message {
public:
  void init(uint8_t msgcnt,int16_t temp,uint8_t humidity, bool batlow) {
    uint8_t t1 = (temp >> 8) & 0x7f;
    uint8_t t2 = temp & 0xff;
    if( batlow == true ) {
      t1 |= 0x80; // set bat low bit
    }
    Message::init(0xc,msgcnt,0x70,Message::BIDI,t1,t2);
    pload[0] = humidity;
  }
};

class WeatherChannel : public Channel<List1,EmptyList,List4,PEERS_PER_CHANNEL>, public Alarm {

  WeatherEventMsg msg;
  uint8_t         msgcnt;
  int16_t         temp;
  uint8_t         humidity;

public:
  WeatherChannel () : Channel(), Alarm(5), msgcnt(0), temp(0), humidity(50) {}
  virtual ~WeatherChannel () {}

  virtual void trigger (AlarmClock& clock) {
    // reactivate for next measure
    tick = delay();
    clock.add(*this);
    DPRINT("Measure...\n");
    measure();

    msg.init(msgcnt,temp,humidity,false);
    device().sendPeerEvent(msg,*this);
  }

  // here we do the measurement
  void measure () {
    static int16_t tdx = -7;
    static int8_t  hdx = 1;
    temp += tdx;
    humidity += hdx;
    if( temp >= 40*10 || temp <= -15*10 ) tdx = -tdx;
    if( humidity == 99 || humidity == 5) hdx = -hdx;
  }

  // here we calc when to send next value
  uint32_t delay () {
    // for testing we use delay of 5sec
    return 5 * 10;
  }

  void setup(Device* dev,uint8_t number,uint16_t addr) {
    Channel::setup(dev,number,addr);
    aclock.add(*this);
  }

  uint8_t status () const {
    return 0;
  }

  uint8_t flags () const {
    return 0;
  }

};


MultiChannelDevice<WeatherChannel,1> sdev(0x20);

class CfgButton : public Button {
public:
  virtual void state (uint8_t s) {
    uint8_t old = Button::state();
    Button::state(s);
    if( s == Button::released && old == Button::pressed ) {
      sdev.startPairing();
    }
    else if( s == longpressed ) {
      if( old == longpressed ) {
        sdev.reset(); // long pressed again - reset
      }
    }
  }
};

CfgButton cfgBtn;
void cfgBtnISR () { cfgBtn.check(); }

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

  sled.set(StatusLed::welcome);
}

void loop() {
  bool worked = aclock.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    activity.savePower();
  }
}
