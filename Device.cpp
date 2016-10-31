//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#include "Device.h"
#include "Led.h"

namespace as {

void Device::process(Message& msg) {}

bool Device::send(Message& msg,const HMID& to) {
  msg.to(to);
  msg.from(devid);
  msg.setRpten(); // has to be set always
  bool result = false;
  uint8_t maxsend = 6;
  while( result == false && maxsend > 0 ) {
    DPRINT(F("<- "));
    msg.dump();
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
