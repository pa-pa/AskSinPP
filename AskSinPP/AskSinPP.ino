
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


class SwitchChannel : public Channel<SwitchList1,SwitchList3,4>, public SwitchStateMachine {

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

  virtual void switchState(uint8_t oldstate,uint8_t newstate,uint8_t dly) {
    if( newstate == AS_CM_JT_ON ) {
      digitalWrite(pin(),HIGH);
    }
    else if ( newstate == AS_CM_JT_OFF ) {
      digitalWrite(pin(),LOW);
    }
  }

};


StatusLed led(4);
MultiChannelDevice<SwitchChannel,4> sdev(0x20);

#include "PinChangeInt.h"

void intPin() {
  led.set(StatusLed::key_long);
}

void setup () {
#ifdef ARDUINO
  Serial.begin(57600);
#endif
  // B0 == PIN 8 on Pro Mini
  pinMode(8, INPUT_PULLUP);
  attachPinChangeInterrupt(8, intPin, CHANGE);

  radio.init();

  if( eeprom.setup() == true ) {
    sdev.firstinit();
  }

  sdev.init(radio,HMID(0x12,0x34,0x56),"papa000000");
  sdev.setFirmwareVersion(0x16);
  sdev.setModel(0x00,0x02);
  sdev.setSubType(0x00);
  sdev.setInfo(0x41,0x01,0x00);

  radio.enableGDO0Int();

  aclock.init();

  led.set(StatusLed::welcome);

  SwitchChannel& ch1 = sdev.channel(1);
  SwitchList1 sd = ch1.getList1();
  eeprom.dump(sd.address(),sd.size());

  DPRINTLN(F("2 Peers"));
  Peer odd(1,2,3,1);
  Peer even(1,2,3,2);
  SwitchChannel ch2 = sdev.channel(2);
  if( sdev.addPeer(2,odd,even) == true ) {
    SwitchList3 ssl = ch2.getList3(odd);
    eeprom.dump(ssl.address(),ssl.size());
    ssl = ch2.getList3(even);
    eeprom.dump(ssl.address(),ssl.size());

    ch2.jumpToTarget(ch2.getList3(even).sh());
    delay(1000);
    ch2.jumpToTarget(ch2.getList3(odd).sh());
  }

  DPRINTLN(F("\n\n1 Peer"));
  Peer p(1,2,3,0);
  sdev.addPeer(1,p);
  SwitchList3 ssl = ch1.getList3(p);
  eeprom.dump(ssl.address(),ssl.size());
  DPRINTLN(F("Change Delays"));
  ssl.sh().onTime(0x20 + 5);
  ssl.sh().offTime(0x20 + 2);
  eeprom.dump(ssl.address(),ssl.size());
  ch1.jumpToTarget(ssl.sh());

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

