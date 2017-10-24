//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __CHANNELLIST_H__
#define __CHANNELLIST_H__

#include "Storage.h"
#include "HMID.h"

namespace as {

class BaseList {
  uint16_t  addr;

public:
  BaseList (uint16_t a) : addr(a) {}

  uint16_t address () const { return addr; }

  bool valid () const { return addr != 0; }

  uint8_t getByte (uint8_t offset) const {
    return storage.getByte(addr + offset);
  }

  uint8_t getByte (uint8_t offset, uint8_t mask, uint8_t shift) const {
    return (getByte(offset) & mask) >> shift;
  }

  bool setByte (uint8_t offset, uint8_t data) const {
    return storage.setByte(addr + offset, data);
  }

  bool setByte (uint8_t offset, uint8_t data, uint8_t mask, uint8_t shift) const {
    uint8_t tmp = getByte(offset) & ~mask;
    tmp |= (data << shift) & mask;
    return setByte(offset, tmp);
  }

  bool isBitSet (uint8_t offset, uint8_t bit) const {
    return (storage.getByte(addr + offset) & bit) == bit;
  }

  bool setBit (uint8_t offset, uint8_t bit, bool value) const {
    if( value == true ) {
      return storage.setBits(addr + offset, bit);
    }
    return storage.clearBits(addr + offset, bit);
  }

  bool setData (uint8_t offset,uint8_t* buf,uint16_t size) const {
    return storage.setData(addr + offset,buf,size);
  }

  bool getData (uint8_t offset,uint8_t* buf,uint16_t size) const {
    return storage.getData(addr + offset,buf,size);
  }

  void clear (uint8_t offset,uint16_t size) {
    storage.clearData(addr + offset,size);
  }

  void init (const uint8_t* data,uint16_t size) {
    for(uint16_t idx=0; idx<size; ++idx) {
      storage.setByte(addr + idx,pgm_read_byte(data + idx));
    }
  }
};

class GenericList : public BaseList {
  uint8_t   size;
  uint8_t (*getregister) (uint8_t off);
  uint8_t (*getoffset) (uint8_t reg);
public:
  GenericList () : BaseList(0), size(0), getregister(0), getoffset(0) {}
  GenericList (uint16_t a,uint8_t s,uint8_t (*getreg) (uint8_t off), uint8_t (*getoff) (uint8_t reg)) : BaseList(a), size(s), getregister(getreg), getoffset(getoff) {}

  uint8_t getOffset (uint8_t reg) const {
    return getoffset(reg);
  }

  uint8_t getRegister (uint8_t offset) const {
    return getregister(offset);
  }

  bool writeRegister (uint8_t reg, uint8_t value) const {
    bool result = false;
    uint8_t offset = getOffset(reg);
    if( offset != 0xff ) {
      result = setByte(offset,value);
    }
    return result;
  }

  uint8_t readRegister (uint8_t reg) const {
    uint8_t value = 0;
    uint8_t offset = getOffset(reg);
    if( offset != 0xff ) {
      value = getByte(offset);
    }
    return value;
  }

  uint8_t getSize () const {
    return size;
  }

  void dump () const {
    DHEX(address());
    DPRINT(F(" - "));
    storage.dump(address(),getSize());
  }


};

template<class DataType>
class ChannelList : public BaseList {
protected:
  ~ChannelList () {}
public:
  ChannelList (uint16_t a) : BaseList(a) {}

  static uint8_t getOffset (uint8_t reg) {
    return DataType::getOffset(reg);
  }

  static uint8_t getRegister (uint8_t offset) {
    return DataType::getRegister(offset);
  }

  static uint8_t size () {
    return sizeof(DataType);
  }

  bool writeRegister (uint8_t reg, uint8_t value) const {
    bool result = false;
    uint8_t offset = getOffset(reg);
    if( offset != 0xff ) {
      result = setByte(offset,value);
    }
    return result;
  }

  uint8_t readRegister (uint8_t reg) const {
    uint8_t value = 0;
    uint8_t offset = getOffset(reg);
    if( offset != 0xff ) {
      value = getByte(offset);
    }
    return value;
  }

  void dump () const {
    DHEX(address());
    DPRINT(F(" - "));
    storage.dump(address(),size());
  }

  operator GenericList () const {
    return GenericList(address(),size(),getRegister,getOffset);
  }
};


class EmptyListData {
public:
  static uint8_t getOffset(__attribute__((unused)) uint8_t reg) { return 0xff; }
  static uint8_t getRegister(__attribute__((unused)) uint8_t reg) { return 0xff; }
};

class EmptyList : public ChannelList<EmptyListData> {
public:
  EmptyList(uint16_t a) : ChannelList(a) {}

  void defaults () {}
  void single () {}
  void even () {}
  void odd () {}
};


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

class List0 : public ChannelList<List0Data> {
public:
  List0(uint16_t a) : ChannelList(a) {}

  HMID masterid () { return HMID(getByte(1),getByte(2),getByte(3)); }
  void masterid (const HMID& mid) {
    setByte(1,mid.id0());
    setByte(2,mid.id1());
    setByte(3,mid.id2());
  };
  bool aesActive () const { return false; }
  bool sabotageMsg () const { return false; }
  bool localResetDisable () const { return false; }

  void defaults () {
    setByte(0,0x01);
    setByte(1,0x00);
    setByte(2,0x00);
    setByte(3,0x00);
  }
};


class List1Data {
public:
  uint8_t  AesActive :1;     // 0x08, s:0, e:1
  uint8_t  notused   :7;

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x08: return 0;
      default: break;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x08;
      default: break;
    }
    return 0xff;
  }
};

class List1 : public ChannelList<List1Data> {
public:
  List1(uint16_t a) : ChannelList(a) {}

  bool aesActive () const { return isBitSet(0,0x01); }
  bool aesActive (bool s) const { return setBit(0,0x01,s); }

  void defaults () {
    setByte(0,0x00);
  }
};


class List4Data {
public:
  uint8_t  burst     :1;     // 0x01, s:0, e:1
  uint8_t  notused   :7;

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x01: return 0;
      default: break;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x01;
      default: break;
    }
    return 0xff;
  }
};

class List4 : public ChannelList<List4Data> {
public:
  List4(uint16_t a) : ChannelList(a) {}

  bool burst () const { return isBitSet(0,0x01); }
  bool burst (bool s) const { return setBit(0,0x01,s); }

  void defaults () {
    setByte(0,0x00);
  }
};

}

#endif
