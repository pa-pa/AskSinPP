
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "HMID.h"
#include "Channel.h"

class Device {
  HMID  devid;
  HMID  master;
  char serial[11];

protected:
  ~Device () {}

public:

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
