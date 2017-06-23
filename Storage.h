//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "Debug.h"

#ifdef ARDUINO_ARCH_STM32F1
  #include "flash_stm32.h"
#endif

namespace as {

class Storage {

#ifdef ARDUINO_ARCH_STM32F1
  #if MCU_STM32F103CB
    #define FlashPageSize 0x400
    #define FlashStartAddress 0x0801fc00  // Page127
  #elif defined(MCU_STM32F103C8)
    #define FlashPageSize 0x400
    #define FlashStartAddress 0x0800fc00  // Page63
  #else
    #error Unknown CPU type
  #endif

  // we mirror 1 Flash Page into RAM
  uint8_t data[FlashPageSize];

  void eeprom_read_block(void* buf,const void* addr,size_t size) {
    uintptr_t offset = (uintptr_t)addr;
    if( offset + size < sizeof(data) ) {
      memcpy(buf,&data[offset],size);
    }
  }
  void eeprom_write_block(const void* buf,void* addr,size_t size) {
    uintptr_t offset = (uintptr_t)addr;
    if( offset + size < sizeof(data) ) {
      memcpy(&data[offset],buf,size);
    }
  }
#endif

#ifndef ARDUINO
  // we mirror 1 Flash Page into RAM
  uint8_t data[1024];

  inline void memcpy(void* dest,const void* src,size_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for( size_t i=0; i<size; ++i ) {
      *d++ = *s++;
    }
  }

  void eeprom_read_block(void* buf,const void* addr,size_t size) {
    uintptr_t offset = (uintptr_t)addr;
    if( offset + size < sizeof(data) ) {
      memcpy(buf,&data[offset],size);
    }
  }
  void eeprom_write_block(const void* buf,void* addr,size_t size) {
    uintptr_t offset = (uintptr_t)addr;
    if( offset + size < sizeof(data) ) {
      memcpy(&data[offset],buf,size);
    }
  }
#endif

public:
  Storage () {
#ifdef ARDUINO_ARCH_STM32F1
    // copy data from FLASH into RAM
    uint16_t* towrite = (uint16_t*)data;
    uint16_t *toread = (uint16_t*)(uintptr_t)FlashStartAddress;
    for( size_t i=0; i<sizeof(data)/2; ++i ) {
      *(towrite + i) = *(toread + i);
    }
#endif
  }

  void store () {
#ifdef ARDUINO_ARCH_STM32F1
    // copy data from RAM to FLASH
    FLASH_Unlock(); //unlock flash writing
    FLASH_ErasePage(FlashStartAddress);
    uint16_t* toread = (uint16_t*)data;
    for( size_t i=0; i<sizeof(data)/2; ++i ) {
      FLASH_ProgramHalfWord(FlashStartAddress+i+i,*(toread+i));
    }
    FLASH_Lock();
#endif
  }

  bool setup (uint16_t checksum=0);

  uint8_t getByte (uint16_t addr) {
    uint8_t b = 0;
    eeprom_read_block(&b,(void*)(uintptr_t)addr,1);
    return b;
  }

  bool setByte (uint16_t addr, uint8_t d) {
    uint8_t b = d;
    eeprom_write_block(&b,(void*)(uintptr_t)addr,1);
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
    eeprom_write_block(buf,(void*)(uintptr_t)addr,size);
    return true;
  }

  template <class T>
  bool setData (uint16_t addr,const T& obj) {
    return setData(addr,(uint8_t*)&obj,sizeof(T));
  }

  bool getData (uint16_t addr,uint8_t* buf,uint16_t size) {
    eeprom_read_block(buf,(const void*)(uintptr_t)addr,size);
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
