//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __PEER_H__
#define __PEER_H__

#include "HMID.h"
#include "Debug.h"

namespace as {

class Peer : public HMID {
  uint8_t chan;
public:
  Peer() : chan(0) {}
  Peer (const HMID& id,uint8_t ch) : HMID(id), chan(ch) {}
  Peer (uint8_t i1, uint8_t i2, uint8_t i3, uint8_t ch) : HMID(i1,i2,i3), chan(ch) {}
  Peer (uint8_t* ptr) : HMID(*ptr,*(ptr+1),*(ptr+2)), chan(*(ptr+4)) {}
  Peer (const Peer& other) {
    *(HMID*)this = (const HMID&)other;
    chan = other.chan;
  }

  Peer& operator = (const Peer& other) {
    *(HMID*)this = (const HMID&)other;
    chan = other.chan;
    return *this;
  }
  bool operator == (const Peer& other) const {
    return chan==other.chan && *(HMID*)this == (HMID&)other;
  }

  static uint8_t size() { return sizeof(Peer); }

  uint8_t channel () const {
    return chan;
  }

  bool even () const {
    return (chan & 0x01) == 0x00;
  }

  bool odd () const {
    return (chan & 0x01) == 0x01;
  }

  void dump () {
    DHEX((uint8_t*)this,sizeof(Peer));
  }
};

}

#endif
