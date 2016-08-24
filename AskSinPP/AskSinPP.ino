

#include "Debug.h"

#include "AlarmClock.h"
#include "Peer.h"
#include "EEProm.h"
#include "MultiChannelDevice.h"
#include "Channel.h"
#include "SwitchList1.h"
#include "SwitchList3.h"
#include "SwitchStateMachine.h"

#include "CC1101.h"
#include "Message.h"

// ping every second
class Ping : public Alarm {
public:
  Ping(uint16_t t) : Alarm(t) {}
  virtual ~Ping (){}
  virtual void trigger (AlarmClock& clock) {
    tick = 10;
    clock.add(*this);
    DPRINTLN(F("Ping"));
  };
};

class SwitchChannel : public Channel<SwitchList1,SwitchList3,4>, public SwitchStateMachine {

public:
  SwitchChannel () : Channel() {

  }
  virtual ~SwitchChannel() {}

  virtual void switchState(uint8_t oldstate,uint8_t newstate,uint8_t dly) {
    DPRINT(F("Channel: "));
    DHEX(number());
    DPRINT(F(" - Switch State: "));
    DHEX(newstate);
    DPRINT(F(" ["));
    DHEX(dly);
    DPRINT(F("] - "));
    if( newstate == AS_CM_JT_ON ) {
      DPRINTLN(F("ON"));
    }
    else if( newstate == AS_CM_JT_OFF ) {
      DPRINTLN(F("OFF"));
    }
    else {
      DPRINTLN(F("..."));
    }
  }

};

CC radio;
#define enableIRQ_GDO0()          ::attachInterrupt(0, radioISR, FALLING);
#define disableIRQ_GDO0()         ::detachInterrupt(0);
Message msg;
void radioISR(void)
{
  // Disable interrupt
  disableIRQ_GDO0();

  uint8_t num = radio.rcvData(msg.buffer());
  if( num > 0 ) {
    msg.decode();
    DPRINT(F("-> "));
    msg.dump();
  }

  // Enable interrupt
  enableIRQ_GDO0();
}


MultiChannelDevice<SwitchChannel,4> sdev(0x20);
Ping ping(10);

void setup () {
#ifdef ARDUINO
  Serial.begin(57600);
#endif

  radio.init();
  enableIRQ_GDO0();

  sdev.setDeviceID(HMID(0x12,0x34,0x56));
  sdev.setSerial("papa000000");
  sdev.setFirmwareVersion(0x16);
  sdev.setModel(0x00,0x03);
  sdev.setSubType(0x00);
  sdev.setInfo(0x41,0x01,0x00);

  if( eeprom.setup() == true ) {
    sdev.firstinit();
  }
  aclock.init();
  // add the "ping"
  // aclock.add(ping);

  SwitchChannel& c0 = sdev.channel(0);
  SwitchList1 sd = c0.getList1();
  eeprom.dump(sd.address(),sd.size());

  DPRINTLN(F("2 Peers"));
  Peer odd(1,2,3,1);
  Peer even(1,2,3,2);
  SwitchChannel c1 = sdev.channel(1);
  if( sdev.addPeer(1,odd,even) == true ) {
    SwitchList3 ssl = c1.getList3(odd);
    eeprom.dump(ssl.address(),ssl.size());
    ssl = c1.getList3(even);
    eeprom.dump(ssl.address(),ssl.size());

    c1.jumpToTarget(c1.getList3(even).sh());
    c1.jumpToTarget(c1.getList3(odd).sh());
  }

  DPRINTLN(F("\n\n1 Peer"));
  Peer p(1,2,3,0);
  sdev.addPeer(0,p);
  SwitchList3 ssl = c0.getList3(p);
  eeprom.dump(ssl.address(),ssl.size());
  DPRINTLN(F("Change Delays"));
  ssl.sh().onTime(0x20 + 5);
  ssl.sh().offTime(0x20 + 2);
  eeprom.dump(ssl.address(),ssl.size());
//  c0.jumpToTarget(ssl.sh());

}

void loop() {

  aclock.runready();

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

