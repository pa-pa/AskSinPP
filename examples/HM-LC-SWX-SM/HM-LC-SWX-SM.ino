
#include <Led.h>
#include <Debug.h>

#include <AlarmClock.h>
#include <MultiChannelDevice.h>
#include <SwitchList1.h>
#include <SwitchList3.h>
#include <SwitchStateMachine.h>
#include <Message.h>
#include <Button.h>
#include <PinChangeInt.h>
#include <TimerOne.h>
#include <Radio.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

// number of relays - possible values 1,2,4
// will map to HM-LC-SW1-SM, HM-LC-SW2-SM, HM-LC-SW4-SM
#define RELAY_COUNT 2

// relay output pins compatible to the HM_Relay project
#define RELAY1_PIN 5
#define RELAY2_PIN 6
#define RELAY3_PIN 7
#define RELAY4_PIN 3

// number of available peers per channel
#define PEERS_PER_CHANNEL 4

// device ID
#define DEVICE_ID HMID(0x12,0x34,0x56)
// serial number
#define DEVICE_SERIAL "papa000000"

// all library classes are placed in the namespace 'as'
using namespace as;

class SwitchChannel : public Channel<SwitchList1,SwitchList3,EmptyList,PEERS_PER_CHANNEL>, public SwitchStateMachine {

public:
  SwitchChannel () : Channel() {}
  virtual ~SwitchChannel() {}

  uint8_t pin () {
    switch( number() ) {
      case 2: return RELAY2_PIN;
      case 3: return RELAY3_PIN;
      case 4: return RELAY4_PIN;
    }
    return RELAY1_PIN;
  }

  void setup(Device* dev,uint8_t number,uint16_t addr) {
    Channel::setup(dev,number,addr);
    uint8_t p=pin();
    pinMode(p,OUTPUT);
    digitalWrite(p,LOW);
  }

  virtual void switchState(uint8_t oldstate,uint8_t newstate) {
    if( newstate == AS_CM_JT_ON ) {
      digitalWrite(pin(),HIGH);
    }
    else if ( newstate == AS_CM_JT_OFF ) {
      digitalWrite(pin(),LOW);
    }
    changed(true);
  }

};


MultiChannelDevice<SwitchChannel,RELAY_COUNT> sdev(0x20);

class CfgButton : public Button {
public:
  virtual void state (uint8_t s) {
    uint8_t old = Button::state();
    Button::state(s);
    if( s == Button::released && old == Button::pressed ) {
      sdev.channel(1).toggleState();
    }
    else if( s== longpressed ) {
      sdev.startPairing();
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

  sdev.init(radio,DEVICE_ID,DEVICE_SERIAL);
  sdev.setFirmwareVersion(0x16);
  // set model id matching number of relays
#if RELAY_COUNT == 2
  sdev.setModel(0x00,0x0a);
#elif RELAY_COUNT == 4
  sdev.setModel(0x00,0x03);
#else // RELAY_COUNT == 1
  sdev.setModel(0x00,0x02);
#endif
  sdev.setSubType(0x00);
  sdev.setInfo(0x41,0x01,0x00);

  radio.enableGDO0Int();

  aclock.init();

  sled.set(StatusLed::welcome);

}

void loop() {
  aclock.runready();
  sdev.pollRadio();
}
