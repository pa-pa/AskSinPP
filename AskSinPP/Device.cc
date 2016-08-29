
#include "Device.h"


void Device::process(Message& msg) {
  /*
  if( msg.to() == devid || (msg.to() == HMID::boardcast && isBoardcastMsg(msg))) {
    DPRINT(F("-> "));
    msg.dump();
    if( msg.type() == 0x01 ) { // CONFIG_
      // PAIR_SERIAL
      if( msg.subcommand() == 0x0a && memcmp(msg.data(),serial,10)==0 ) {
        sendDeviceInfo(msg.from(),msg.count());
      }
      // CONFIG_STATUS_REQUEST
      else if (msg.subcommand() == 0x0e ) {
        uint8_t channel = msg.command();
        sendInfoActuatorStatus(msg.from(),msg.count(),channel);
      }
    }
  }
  else {
    DPRINT(F("ignore "));
    msg.dump();
  }
  */
}

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

void Device::sendAck (Message& msg) {
  msg.initWithCount(0x0a,0x02,0x00,0x00);
  send(msg,msg.from());
}

void Device::sendAck (Message& msg,uint8_t channel,uint8_t state,uint8_t action) {
  msg.initWithCount(0x0e,0x02,0x00,0x01);
  msg.subcommand(channel);
  *msg.data() = state;
  *(msg.data()+1) = action; // TODO battery status
  *(msg.data()+2) = radio->rssi();
  send(msg,msg.from());
}

void Device::sendNack (Message& msg) {
  msg.initWithCount(0x0a,0x02,0x00,0x80);
  send(msg,msg.from());
}

void Device::sendDeviceInfo (const HMID& to,uint8_t count) {
  msg.init(0x1c,count,0x00, to.valid() ? Message::BIDI : 0x00,0x00,0x00);
  uint8_t* data = msg.data();
  data[0] = firmversion;
  memcpy(&data[1],model,sizeof(model));
  memcpy(&data[3],serial,10);
  data[13] = subtype;
  memcpy(&data[14],devinfo,sizeof(devinfo));
  send(msg,to);
}

void Device::sendInfoActuatorStatus (const HMID& to,uint8_t count,uint8_t ch,uint8_t status) {
  msg.init(0x0b+3,count,0x10,Message::BIDI,0x06,ch);
  *msg.data() = status;
  *(msg.data()+1) = 0x00;
  *(msg.data()+2) = radio->rssi();
  send(msg,to);
}
