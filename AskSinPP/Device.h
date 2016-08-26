
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

  void sendAck (Message& msg);
  void sendAck (Message& msg,uint8_t channel,uint8_t state,uint8_t action);
  void sendNack (Message& msg);
  void sendDeviceInfo () {
    sendDeviceInfo(HMID::boardcast,++msgcount);
  }
  void sendDeviceInfo (const HMID& to,uint8_t count);

};

#endif
