
#ifndef __CHANNELLIST_H__
#define __CHANNELLIST_H__

#include "EEProm.h"

template<class DataType>
class ChannelList {
protected:
  uint16_t  addr;

  ~ChannelList () {}
public:
  ChannelList (uint16_t a) : addr(a) {}

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

  uint8_t getOffset (uint8_t reg) const {
    return DataType::getOffset(reg);
  }

  uint8_t getRegister (uint8_t offset) const {
    return DataType::getRegister(offset);
  }

  bool writeRegister (uint8_t reg, uint8_t value) const {
    bool result = false;
    uint8_t offset = getOffset(reg);
    if( offset != 0xff ) {
      result = setByte(offset,value);
    }
    return result;
  }

  uint8_t readRegister (uint8_t reg) {
    uint8_t value = 0;
    uint8_t offset = getOffset(reg);
    if( offset != 0xff ) {
      value = getByte(offset);
    }
    return value;
  }

  static uint8_t size () {
    return sizeof(DataType);
  }

  void dump () {
    DHEX(address());
    DPRINT(" - ");
    eeprom.dump(address(),size());
  }
};

#endif
