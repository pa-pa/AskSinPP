
#include "Debug.h"

#include "AlarmClock.h"
#include "Peer.h"
#include "EEProm.h"
#include "MultiChannelDevice.h"
#include "Channel.h"
#include "SwitchList1.h"
#include "SwitchList3.h"
#include "SwitchStateMachine.h"
#include "StatusLed.h"

#include "CC1101.h"
#include "Message.h"
#include "Button.h"
#include "PinChangeInt.h"


class SwitchChannel : public Channel<SwitchList1,SwitchList3,EmptyList,4>, public SwitchStateMachine {

public:
  SwitchChannel () : Channel() {}
  virtual ~SwitchChannel() {}

  uint8_t pin () {
    if( number() < 4 ) {
      return 4+number();
    }
    return 3;
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


StatusLed sled(4);
MultiChannelDevice<SwitchChannel,2> sdev(0x20);

class CfgButton : public Button {
public:
  virtual void state (uint8_t s) {
    Button::state(s);
    if( s == Button::pressed ) {
      sdev.startPairing();
    }
    else if( s== longpressed ) {
      sled.set(StatusLed::key_long);
    }
    else if( s == Button::longlongpressed ) {
      sdev.reset();
    }
  }
};

CfgButton cfgBtn;
void cfgBtnISR () { cfgBtn.pinChange(); }

void setup () {
#ifdef ARDUINO
  Serial.begin(57600);
#endif
  // B0 == PIN 8 on Pro Mini
  cfgBtn.init(8);
  attachPinChangeInterrupt(8,cfgBtnISR,CHANGE);
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

#ifndef ARDUINO
  // simulate timer
  usleep(100000);
  --aclock;
#endif

}

#ifndef ARDUINO
int main () {
  setup();
  while( true )
    loop();
  return 0;
}
#endif

