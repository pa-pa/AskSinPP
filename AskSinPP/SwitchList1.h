#ifndef __SWITCHCHANNELDATA_H__
#define __SWITCHCHANNELDATA_H__

#include "ChannelList.h"

class SwitchList1Data {
public:
  uint8_t sign             :1;     // 0x08, s:0, e:1
  uint8_t                  :7;     //
  uint8_t transmitTryMax   :8;     // 0x30, s:0, e:8
  uint8_t powerUpAction    :1;     // 0x56, s:0, e:1
  uint8_t                  :7;     // 0x56, s:1, e:8
  uint8_t statusInfoMinDly :5;     // 0x57, s:0, e:5
  uint8_t statusInfoRandom :3;     // 0x57, s:5, e:8

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x08: return 0;
      case 0x30: return 1;
      case 0x56: return 2;
      case 0x57: return 3;
      default:   break;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x08;
      case 1:  return 0x30;
      case 2:  return 0x56;
      case 3:  return 0x57;
      default: break;
    }
    return 0xff;
  }
};

class SwitchList1 : public ChannelList {
public:
  SwitchList1(uint16_t a) : ChannelList(a) {}
  virtual ~SwitchList1() {}

  bool sign () const { return isBitSet(0,0x01); }
  bool sign (bool s) const { return setBit(0,0x01,s); }

  uint8_t transmitTryMax () { return getByte(1); }
  bool transmitTryMax (uint8_t v) { return setByte(1,v); }

  bool powerUpAction () const { return isBitSet(2,0x01); }
  bool powerUpAction (bool s) const { return setBit(2,0x01,s); }

  uint8_t statusInfoMinDly () const { return getByte(3) & 0x1f; }
  bool statusInfoMinDly (uint8_t value) { return setByte(3,value,0x1f,0); }
  uint8_t statusInfoRandom () const { return getByte(3) >> 5; }
  bool statusInfoRandom (uint8_t value) { return setByte(3,value,0xe0,5); }

  virtual uint8_t getOffset (uint8_t reg) const { return SwitchList1Data::getOffset(reg); }

  void defaults () {
    setByte(0,0);
    sign(false);
    transmitTryMax(6);
    powerUpAction(false);
    statusInfoMinDly(0);
    statusInfoRandom(0);
  }

  static uint8_t size () {
    return sizeof(SwitchList1Data);
  }
};

#endif
