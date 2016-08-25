#ifndef __LIST0_H__
#define __LIST0_H__

#include "ChannelList.h"
#include "HMID.h"

class List0Data {
public:
  uint8_t data    :8;     // 0x02, s:0, e:8
  uint8_t master1 :8;     // 0x0A, s:0, e:8
  uint8_t master2 :8;     // 0x0B, s:0, e:8
  uint8_t master3 :8;     // 0x0C, s:0, e:8

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x02: return 0;
      case 0x0A: return 1;
      case 0x0B: return 2;
      case 0x0C: return 3;
      default:   break;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x02;
      case 1:  return 0x0A;
      case 2:  return 0x0B;
      case 3:  return 0x0C;
      default: break;
    }
    return 0xff;
  }
};

class List0 : public ChannelList {
public:
  List0(uint16_t a) : ChannelList(a) {}
  virtual ~List0() {}

  HMID masterid () { return HMID(getByte(1),getByte(2),getByte(3)); }
  void masterid (const HMID& mid) {
    setByte(1,mid.id0());
    setByte(2,mid.id1());
    setByte(3,mid.id2());
  };

  virtual uint8_t getOffset (uint8_t reg) const { return List0Data::getOffset(reg); }

  void defaults () {
    setByte(0,0x01);
    setByte(1,0x00);
    setByte(2,0x00);
    setByte(3,0x00);
  }

  static uint8_t size () {
    return sizeof(List0Data);
  }
};

#endif
