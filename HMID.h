
#ifndef __HMID_H__
#define __HMID_H__

#include "Atomic.h"

namespace as {

class HMID {
  uint8_t id[3];
public:
  HMID () {
    id[0]=id[1]=id[2]=0;
  }
  HMID (uint8_t i1, uint8_t i2, uint8_t i3) {
    id[0]=i1;
    id[1]=i2;
    id[2]=i3;
  }
  HMID (uint8_t* ptr) {
    id[0]=*ptr;
    id[1]=*(ptr+1);
    id[2]=*(ptr+2);
  }
  HMID (const HMID& other) {
    id[0]=other.id[0];
    id[1]=other.id[1];
    id[2]=other.id[2];
  }
  HMID& operator = (const HMID& other) {
    id[0]=other.id[0];
    id[1]=other.id[1];
    id[2]=other.id[2];
    return *this;
  }
  bool operator == (const HMID& other) const {
    return id[0]==other.id[0] && id[1]==other.id[1] && id[2]==other.id[2];
  }
  bool operator != (const HMID& other) const {
    return (operator == (other)) == false;
  }
  bool valid() const {
    return id[0]!=0 || id[1]!=0 || id[2]!=0;
  }
  uint8_t id0 () const { return id[0]; };
  uint8_t id1 () const { return id[1]; };
  uint8_t id2 () const { return id[2]; };

  static HMID boardcast;
};

}

#endif
