
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "HMID.h"
#include "Channel.h"

class Device {
  HMID  devid;
  HMID  master;
  char serial[11];

  uint8_t firmversion;
  uint8_t model[2];
  uint8_t subtype;
  uint8_t devinfo[3];

protected:
  ~Device () {}

public:

  void setFirmwareVersion (uint8_t v) {
    firmversion = v;
  }

  void setModel (uint8_t m1, uint8_t m2) {
    model[0] = m1;
    model[1] = m1;
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

};

#endif
