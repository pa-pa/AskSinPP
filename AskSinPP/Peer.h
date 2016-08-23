
#ifndef __PEER_H__
#define __PEER_H__

#include "HMID.h"

class Peer : public HMID {
  uint8_t channel;
public:
  Peer() : channel(0) {}
  Peer (HMID id,uint8_t ch) : HMID(id), channel(ch) {}
  Peer (uint8_t i1, uint8_t i2, uint8_t i3, uint8_t ch) : HMID(i1,i2,i3), channel(ch) {}
  Peer (uint8_t* ptr) : HMID(*ptr,*(ptr+1),*(ptr+2)), channel(*(ptr+4)) {}
  Peer (const Peer& other) {
    *(HMID*)this = (const HMID&)other;
    channel = other.channel;
  }

  Peer& operator = (const Peer& other) {
    *(HMID*)this = (const HMID&)other;
    channel = other.channel;
    return *this;
  }
  bool operator == (const Peer& other) const {
    return channel==other.channel && *(HMID*)this == (HMID&)other;
  }

  static uint8_t size() { return sizeof(Peer); }
};

#endif
