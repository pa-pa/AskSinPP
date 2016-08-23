
#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "Debug.h"
#ifndef ARDUINO
  #include <string.h>
#endif

class EEProm {
  #ifndef ARDUINO
  uint8_t data[2048]; // simulate 2k eeprom
  #endif
public:
  EEProm () {
  #ifndef ARDUINO
    memset(data,0,sizeof(data));
  #endif
  }

  bool setup ();

  uint8_t getByte (uint16_t addr) {
  #ifndef ARDUINO
    return data[addr];
  #else
    uint8_t b;
    eeprom_read_block((void*)&b,(const void*)addr,1);
    return b;
  #endif
  }

  bool setByte (uint16_t addr, uint8_t d) {
  #ifndef ARDUINO
    data[addr] = d;
  #else
    eeprom_write_block((const void*)&d,(void*)addr,1);    
  #endif
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
  #ifndef ARDUINO
    for( uint16_t i=0; i<size; ++i, ++buf ) {
      setByte(addr+i,*buf);
    }
  #else
    eeprom_write_block((const void*)buf,(void*)addr,size);    
  #endif
    return true;
  }

  template <class T>
  bool setData (uint16_t addr,const T& obj) {
    return setData(addr,(uint8_t*)&obj,sizeof(T));
  }

  bool getData (uint16_t addr,uint8_t* buf,uint16_t size) {
  #ifndef ARDUINO
    for( uint16_t i=0; i<size; ++i, ++buf ) {
      *buf = getByte(addr+i);
    }
  #else
    eeprom_read_block((void*)buf,(const void*)addr,size);
  #endif
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
};

extern EEProm eeprom;

#endif
