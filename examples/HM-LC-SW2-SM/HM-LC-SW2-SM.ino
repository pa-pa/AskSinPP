
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


#define PEERS_PER_CHANNEL 4
#define LED_PIN 4
#define CONFIG_BUTTON_PIN 8
#define RELAY1_PIN 5
#define RELAY2_PIN 6


using namespace as;

class SwitchChannel : public Channel<SwitchList1,SwitchList3,EmptyList,PEERS_PER_CHANNEL>, public SwitchStateMachine {

public:
  SwitchChannel () : Channel() {}
  virtual ~SwitchChannel() {}

  uint8_t pin () {
    if( number() == 1 ) {
      return RELAY1_PIN;
    }
    return RELAY2_PIN;
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


MultiChannelDevice<SwitchChannel,2> sdev(0x20);

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

  // B0 == PIN 8 on Pro Mini
  cfgBtn.init(CONFIG_BUTTON_PIN);
  attachPinChangeInterrupt(CONFIG_BUTTON_PIN,cfgBtnISR,CHANGE);
  radio.init();

  if( eeprom.setup() == true ) {
    sdev.firstinit();
  }

  sdev.init(radio,HMID(0x12,0x34,0x56),"papa000000");
  sdev.setFirmwareVersion(0x16);
  sdev.setModel(0x00,0x0a);
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
