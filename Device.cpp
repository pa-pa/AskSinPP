
#include "Device.h"

#include "Led.h"

namespace as {

void Device::process(Message& msg) {}

bool Device::send(Message& msg,const HMID& to) {
  msg.to(to);
  msg.from(devid);
  DPRINT(F("<- "));
  msg.dump();
  bool result = false;
  uint8_t maxsend = 6;
  while( result == false && maxsend > 0 ) {
    maxsend--;
    result = radio->write(msg,msg.burstRequired());
    if( result == true && msg.ackRequired() == true && to.valid() == true ) {
      result = waitForAck(msg,30); // 300ms
      DPRINT(F("waitAck: ")); DHEX((uint8_t)result); DPRINTLN(F(""));
    }
  }
  if( sled.active() == false ) {
    sled.set(StatusLed::send);
  }
  return result;
}


}
