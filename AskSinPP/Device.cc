
#include "Device.h"


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
  return result;
}

bool Device::waitForAck(Message& msg,uint8_t timeout) {
  do {
    if( radio->readAck(msg) == true ) {
      return true;
    }
    delay(10); // wait 10ms
    timeout--;
  }
  while( timeout > 0 );
  return false;
}

void Device::sendDeviceInfo (const HMID& to,uint8_t count) {
  DeviceInfoMsg& pm = msg.deviceInfo();
  pm.init(to,count);
  pm.fill(firmversion,model,serial,subtype,devinfo);
  send(msg,to);
}

