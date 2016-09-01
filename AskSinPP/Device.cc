
#include "Device.h"
#include "StatusLed.h"

void Device::process(Message& msg) {}

bool Device::send(Message& msg,const HMID& to) {
  msg.to(to);
  msg.from(devid);
  DPRINT(F("<- "));
  msg.dump();
  bool result = radio->write(msg,msg.burstRequired());
  if( result == true && msg.ackRequired() ) {
    result = waitForAck(msg,30); // 300ms
//    // TODO - retransmit if no ack
    DPRINT(F("waitAck: ")); DHEX((uint8_t)result); DPRINTLN(F(""));
  }
  if( sled.active() == false ) {
    sled.set(StatusLed::send);
  }
  return result;
}

