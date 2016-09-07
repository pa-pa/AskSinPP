
#include <Led.h>
#include <Debug.h>

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

class TempChannel : public Channel<List1,EmptyList,List4,PEERS_PER_CHANNEL>, public Alarm {

public:
  TempChannel () : Channel(), Alarm(5) {}
  virtual ~TempChannel () {}

  virtual void trigger (AlarmClock& clock) {
    // reactivate for next measure
    tick = 50;
    clock.add(*this);
    DPRINT("Measure...\n");

    bool sendtopeer=false;
    for( int i=0; i<peers(); ++i ){
      Peer p = peer(i);
      if( p.valid() == true ) {
        // TODO send message to peer
        sendtopeer = true;
      }
    }
    if( sendtopeer == false ) {
      // TODO send message to master
    }
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


MultiChannelDevice<TempChannel,1> sdev(0x20);

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

  sled.set(StatusLed::welcome);
}

void loop() {
  aclock.runready();
  sdev.pollRadio();
}
