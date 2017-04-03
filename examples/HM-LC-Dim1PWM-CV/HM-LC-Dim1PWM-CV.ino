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
//  #define USE_AES
//  #define HM_DEF_KEY 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
//  #define HM_DEF_KEY_INDEX 0
#endif

#include <EnableInterrupt.h>
// #include <SPI.h>  // when we include SPI.h - we can use LibSPI class
#include <AskSinPP.h>
#include <TimerOne.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <Dimmer.h>

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#ifdef USE_OTA_BOOTLOADER
  #define OTA_MODEL_START  0x7ff0 // start address of 2 byte model id in bootloader
  #define OTA_SERIAL_START 0x7ff2 // start address of 10 byte serial number in bootloader
  #define OTA_HMID_START   0x7ffc // start address of 3 byte device id in bootloader
#else
  // device ID
  #define DEVICE_ID HMID(0x11,0x12,0x22)
  // serial number
  #define DEVICE_SERIAL "papa111222"
#endif

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8


#define DIMMER_PIN 3

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


// setup the device with channel type and number of channels
MultiChannelDevice<Hal,DimmerChannel<Hal,PEERS_PER_CHANNEL>,3> sdev(0x20);

class CfgButton : public Button {
public:
  CfgButton () {
    setLongPressTime(seconds2ticks(3));
  }
  virtual void state (uint8_t s) {
    uint8_t old = Button::state();
    Button::state(s);
    if( s == Button::released ) {
      sdev.channel(1).toggleState();
    }
    else if( s == longreleased ) {
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
  // first initialize EEProm if needed
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
  sdev.setModel(0x00,0x67);
#endif
  sdev.setFirmwareVersion(0x25);
  sdev.setSubType(DeviceType::Dimmer);
  sdev.setInfo(0x41,0x01,0x00);

  hal.radio.enable();

  aclock.init();

  hal.led.set(StatusLed::welcome);

  // TODO - random delay

  pinMode(3,OUTPUT);
  analogWrite(3,200);
}

void loop() {
  bool worked = aclock.runready();
  bool poll = sdev.pollRadio();
  analogWrite(3,sdev.channel(1).status());
  if( worked == false && poll == false ) {
//    hal.activity.savePower<Idle>(hal);
  }
}
