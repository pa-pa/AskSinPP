//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-03-20 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

/*
 * Setup defines to configure the library.
 * Note: If you are using the Eclipse Arduino IDE you will need to set the
 * defines in the project properties.
 */
//  #define USE_AES
//  #define HM_DEF_KEY 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
//  #define HM_DEF_KEY_INDEX 0

#include <SPI.h>  // when we include SPI.h - we can use LibSPI class
#include <AskSinPP.h>

#include <MultiChannelDevice.h>
#include <SwitchChannel.h>

// number of relays - possible values 1,2,4
// will map to HM-LC-SW1-SM, HM-LC-SW2-SM, HM-LC-SW4-SM
#define RELAY_COUNT 4

// define model is matching the number of relays
#if RELAY_COUNT == 2
  #define SW_MODEL 0x0a
#elif RELAY_COUNT == 4
  #define SW_MODEL 0x03
#else
  #define SW_MODEL 0x02
#endif
// device ID
#define DEVICE_ID HMID(0x12,0x34,0x56)
// serial number
#define DEVICE_SERIAL "papa000000"

// use builtin led
#define LED_PIN LED_BUILTIN
// Arduino pin for the config button
// use button on maple mini board
#define CONFIG_BUTTON_PIN PB8


#define RELAY1_PIN PC13
#define RELAY2_PIN PC14
#define RELAY3_PIN PC15
#define RELAY4_PIN PA0

// number of available peers per channel
#define PEERS_PER_CHANNEL 4


// all library classes are placed in the namespace 'as'
using namespace as;

/**
 * Configure the used hardware
 */
typedef LibSPI<PA4> RadioSPI;
typedef AskSin<StatusLed,NoBattery,Radio<RadioSPI,PB0> > Hal;
Hal hal;

// map number of channel to pin
// this will be called by the SwitchChannel class
uint8_t SwitchPin (uint8_t number) {
  switch( number ) {
    case 2: return RELAY2_PIN;
    case 3: return RELAY3_PIN;
    case 4: return RELAY4_PIN;
  }
  return RELAY1_PIN;
}

// setup the device with channel type and number of channels
MultiChannelDevice<Hal,SwitchChannel<Hal,PEERS_PER_CHANNEL>,RELAY_COUNT> sdev(0x20);

class CfgButton : public StateButton<LOW,HIGH,INPUT_PULLDOWN> {
public:
  CfgButton () {
    setLongPressTime(seconds2ticks(3));
  }
  virtual void state (uint8_t s) {
    uint8_t old = StateButton::state();
    StateButton::state(s);
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
    storage.store();
  }

  hal.led.init(LED_PIN);

  cfgBtn.init(CONFIG_BUTTON_PIN);
  enableInterrupt(CONFIG_BUTTON_PIN,cfgBtnISR,CHANGE);
  hal.radio.init();

  sdev.init(hal,DEVICE_ID,DEVICE_SERIAL);
  sdev.setModel(0x00,SW_MODEL);

  sdev.setFirmwareVersion(0x16);
  sdev.setSubType(DeviceType::Switch);
  sdev.setInfo(0x41,0x01,0x00);

  hal.radio.enable();

  aclock.init();

  hal.led.set(StatusLed::welcome);

  // TODO - random delay
}

void loop() {
  bool worked = aclock.runready();
  bool poll = sdev.pollRadio();
//  if( worked == false && poll == false ) {
//    hal.activity.savePower<Idle>(hal);
//  }
}
