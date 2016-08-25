
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
    for( uint8_t i=0; i<10; ++i, ++ser ) {
      serial[i] = (uint8_t)*ser;
    }
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

  void send(Message& msg,const HMID& to) {
    msg.to(to);
    msg.from(devid);
    DPRINT(F("<- "));
    msg.dump();
    radio->write(msg,0);
  }

  virtual void process(Message& msg) {
    static HMID boardcastAdr(0,0,0);
    if( msg.to() == devid || (msg.to() == boardcastAdr && isBoardcastMsg(msg))) {
      DPRINT(F("-> "));
      msg.dump();
      // if we get a PairSerial message we send the device info
      if( msg.isPairSerial() == true ) {
        sendDeviceInfo();
      }
    }
    else {
      DPRINT(F("ignore "));
      msg.dump();
    }
  }

  bool isBoardcastMsg(Message msg) {
    return msg.isPairSerial();
  }

  void sendDeviceInfo () {
    uint8_t count = 0;
    HMID to;
    if( msg.isPairSerial() == true ) {
      count = msg.count();
      to = msg.from();
    }
    else {
      count = msgcount++;
    }
    msg.init(count,0x00, to.valid() ? Message::BIDI : 0x00,0x00,0x00);
    msg.append(firmversion);
    msg.append(model,sizeof(model));
    msg.append((uint8_t*)serial,10);
    msg.append(subtype);
    msg.append(devinfo,sizeof(devinfo));
    send(msg,to);
  }

};

#endif
