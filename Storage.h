//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "Debug.h"

namespace as {

class Storage {
public:
  Storage () {}

  bool setup (uint16_t checksum=0);

  uint8_t getByte (uint16_t addr) {
    uint8_t b;
    eeprom_read_block((void*)&b,(const void*)addr,1);
    return b;
  }

  bool setByte (uint16_t addr, uint8_t d) {
    eeprom_write_block((const void*)&d,(void*)addr,1);    
    return true;
  }

  bool setBits (uint16_t addr, uint8_t bits) {
    setByte(addr,getByte(addr) | bits);
    return true;
  }

  bool clearBits (uint16_t addr, uint8_t bits) {
    setByte(addr,getByte(addr) & ~bits);
    return true;
  }

  bool setData (uint16_t addr,uint8_t* buf,uint16_t size) {
    eeprom_write_block((const void*)buf,(void*)addr,size);    
    return true;
  }

  template <class T>
  bool setData (uint16_t addr,const T& obj) {
    return setData(addr,(uint8_t*)&obj,sizeof(T));
  }

  bool getData (uint16_t addr,uint8_t* buf,uint16_t size) {
    eeprom_read_block((void*)buf,(const void*)addr,size);
    return true;
  }

  template <class T>
  bool getData (uint16_t addr,T* obj) {
    return getData(addr,(uint8_t*)obj,sizeof(T));
  }

  bool clearData (uint16_t addr, uint16_t size) {
    for( uint16_t i=0; i<size; ++i ) {
      setByte(addr+i,0);
    }
    return true;
  }

  void dump (uint16_t start, uint16_t num);

  static uint16_t crc16 (uint16_t crc,uint8_t d);

};

extern Storage storage;

}

#endif
