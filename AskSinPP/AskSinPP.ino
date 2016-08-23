

#include "Debug.h"

#include "AlarmClock.h"
#include "Peer.h"
#include "EEProm.h"
#include "SwitchDevice.h"

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


SwitchDevice sdev(0x20);

Ping ping(10);

void setup () {
#ifdef ARDUINO
  Serial.begin(57600);
#endif

  sdev.setDeviceID(HMID(0x12,0x34,0x56));
  sdev.setSerial("papa000000");
  if( eeprom.setup() == true ) {
    sdev.firstinit();
  }
  aclock.init();
  

  aclock.add(ping);

  SwitchChannel& sw1 = (SwitchChannel&)sdev.channel(0);
  SwitchChannelList sd(sw1.listAddress(1));
  sd.defaults();
  eeprom.dump(sd.address(),sd.size());

  Peer p(1,2,3,0);
  sdev.addPeer(0,p);
  SwitchStateList ssl(sdev.channel(0).listAddress(3,p));
  eeprom.dump(ssl.address(),ssl.size());

  ssl.sh().onTime(0x20 + 5);
  ssl.sh().offTime(0x20 + 2);
  eeprom.dump(ssl.address(),ssl.size());
  sw1.jumpToTarget(ssl.sh());
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

