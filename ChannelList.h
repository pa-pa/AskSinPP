
#ifndef __CHANNELLIST_H__
#define __CHANNELLIST_H__

#include "EEProm.h"

class BaseList {
  uint16_t  addr;

public:
  BaseList (uint16_t a) : addr(a) {}

  uint16_t address () const { return addr; }

  bool valid () const { return addr != 0; }

  uint8_t getByte (uint8_t offset) const {
    return eeprom.getByte(addr + offset);
  }

  uint8_t getByte (uint8_t offset, uint8_t mask, uint8_t shift) const {
    return (getByte(offset) & mask) >> shift;
  }

  bool setByte (uint8_t offset, uint8_t data) const {
    return eeprom.setByte(addr + offset, data);
  }

  bool setByte (uint8_t offset, uint8_t data, uint8_t mask, uint8_t shift) const {
    uint8_t tmp = getByte(offset) & ~mask;
    tmp |= (data << shift) & mask;
    return setByte(offset, tmp);
  }

  bool isBitSet (uint8_t offset, uint8_t bit) const {
    return (eeprom.getByte(addr + offset) & bit) == bit;
  }

  bool setBit (uint8_t offset, uint8_t bit, bool value) const {
    if( value == true ) {
      return eeprom.setBits(addr + offset, bit);
    }
    return eeprom.clearBits(addr + offset, bit);
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
    DPRINT(" - ");
    eeprom.dump(address(),getSize());
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
    DPRINT(" - ");
    eeprom.dump(address(),size());
  }

  operator GenericList () const {
    return GenericList(address(),size(),getRegister,getOffset);
  }
};


class EmptyListData {
public:
  static uint8_t getOffset(uint8_t reg) { return 0xff; }
  static uint8_t getRegister(uint8_t reg) { return 0xff; }
};

class EmptyList : public ChannelList<EmptyListData> {
public:
  EmptyList(uint16_t a) : ChannelList(a) {}
};

#endif
