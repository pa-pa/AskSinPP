
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "HMID.h"
#include "Channel.h"
#include "Message.h"
#include "CC1101.h"

class Device {
  HMID  devid;
  HMID  master;
  char serial[11];

  uint8_t firmversion;
  uint8_t model[2];
  uint8_t subtype;
  uint8_t devinfo[3];

  CC1101* radio;
  uint8_t msgcount;

protected:
  Message msg;

public:
  virtual ~Device () {}

  void setRadio(CC1101& r) {
    msgcount=0;
    radio = &r;
  }

  CC1101& getRadio () {
    return *radio;
  }

  void setFirmwareVersion (uint8_t v) {
    firmversion = v;
  }

  void setModel (uint8_t m1, uint8_t m2) {
    model[0] = m1;
    model[1] = m2;
  }

  void setSubType (uint8_t st) {
    subtype = st;
  }

  void setInfo (uint8_t i1, uint8_t i2, uint8_t i3) {
    devinfo[0] = i1;
    devinfo[1] = i2;
    devinfo[2] = i3;
  }

  void setMasterID (const HMID& id) {
    master = id;
  }

  const HMID& getMasterID () const {
    return master;
  }

  void setDeviceID (const HMID& id) {
    devid=id;
  }

  const HMID& getDeviceID () const {
    return devid;
  }

  void setSerial (const char* ser) {
    memcpy(serial,ser,10);
    serial[10] = 0;
  }

  const char* getSerial () const {
    return serial;
  }

  void pollRadio () {
    uint8_t num = getRadio().read(msg);
    if( num > 0 ) {
      process(msg);
    }
  }

  virtual void process(Message& msg);

  bool isBoardcastMsg(Message msg) {
    return msg.isPairSerial();
  }

  bool send(Message& msg,const HMID& to);

  bool waitForAck(Message& msg,uint8_t timeout);

  void sendAck (Message& msg) {
    msg.initWithCount(0x0a,0x02,0x00,0x00);
    send(msg,msg.from());
  }

  void sendNack (Message& msg) {
    msg.initWithCount(0x0a,0x02,0x00,0x80);
    send(msg,msg.from());
  }

  template <class ChannelType>
  void sendAck (Message& msg,const ChannelType& ch) {
    msg.initWithCount(0x0e,0x02,0x00,0x01);
    msg.subcommand(ch.number());
    *msg.data() = ch.state();
    *(msg.data()+1) = 0; // TODO battery status
    *(msg.data()+2) = radio->rssi();
    send(msg,msg.from());
  }

  void sendDeviceInfo () {
    sendDeviceInfo(HMID::boardcast,++msgcount);
  }

  void sendDeviceInfo (const HMID& to,uint8_t count);

  template <class ChannelType>
  void sendInfoActuatorStatus (const HMID& to,uint8_t count,const ChannelType& ch) {
    msg.init(0x0b+3,count,0x10,Message::BIDI,0x06,ch.number());
    *msg.data() = ch.status();
    *(msg.data()+1) = 0x00;
    *(msg.data()+2) = radio->rssi();
    send(msg,to);
  }

  template <class ListType>
  void sendInfoParamResponsePairs(const HMID& to,uint8_t count,const ListType& list) {
    uint8_t  current=0;
    uint8_t* buf=msg.data()-1;
    for( int i=0; i<list.size(); ++i ) {
      *buf++ = list.getRegister(i);
      *buf++ = list.getByte(i);
      current++;
      if( current == 8 ) {
        msg.initWithCount(0x0b-1+(8*2),0x10,Message::BIDI,0x02);
        msg.count(count);
        // reset to zero
        current=0;
        if( send(msg,to) == false ) {
          // exit loop in case of error
          break;
        }
      }
    }
    if( current > 0) {
      *buf++ = 0;
      *buf++ = 0;
      current++;
      msg.initWithCount(0x0b-1+(current*2),0x10,Message::BIDI,0x02);
      msg.count(count);
      send(msg,to);
    }
  }
};

#endif
