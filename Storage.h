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

class InternalEprom {

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
  InternalEprom () {
#ifdef ARDUINO_ARCH_STM32F1
    // copy data from FLASH into RAM
    uint16_t* towrite = (uint16_t*)data;
    uint16_t *toread = (uint16_t*)(uintptr_t)FlashStartAddress;
    for( size_t i=0; i<sizeof(data)/2; ++i ) {
      *(towrite + i) = *(toread + i);
    }
#endif
  }

  bool present () {
    return true;
  }

  uint16_t size () {
    return 1024;
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

  bool setData (uint16_t addr,uint8_t* buf,uint16_t size) {
    eeprom_write_block(buf,(void*)(uintptr_t)addr,size);
    return true;
  }

  bool getData (uint16_t addr,uint8_t* buf,uint16_t size) {
    eeprom_read_block(buf,(const void*)(uintptr_t)addr,size);
    return true;
  }

  bool clearData (uint16_t addr, uint16_t size) {
    for( uint16_t i=0; i<size; ++i) {
      setByte(addr+i,0);
    }
    return true;
  }
};


#ifdef TwoWire_h
template <uint8_t ID,uint16_t PAGES,uint8_t PAGESIZE>
class at24cX {
public:
  at24cX () {}

  bool present () {
    Wire.beginTransmission(ID);
    return Wire.endTransmission() == 0;
  }

  uint16_t size () {
    return PAGES * PAGESIZE;
  }

  void store () {}

  uint8_t getByte (uint16_t addr) {
    uint8_t b = 0;
    Wire.beginTransmission(ID);
    Wire.write(addr >> 8);
    Wire.write(addr & 0xff);
    if( Wire.endTransmission() == 0 ) {
      Wire.requestFrom(ID,(uint8_t)1);
      if( Wire.available() ) {
        b = (uint8_t)Wire.read();
      }
    }
    return b;
  }

  bool setByte (uint16_t addr, uint8_t d) {
    bool success = false;
    Wire.beginTransmission(ID);
    Wire.write(addr >> 8);
    Wire.write(addr & 0xff);
    Wire.write(d);
    success = Wire.endTransmission();
    // wait for write operation finished
    while( present() == false ) {
      _delay_ms(1);
    }
    _delay_ms(1);
    return success;
  }

  bool setData (uint16_t addr,uint8_t* buf,uint16_t size) {
    bool success = false;
    while( size > 0 ) {
      uint16_t towrite = PAGESIZE - (addr % PAGESIZE);
      if( size < towrite) towrite = size;
//      DPRINT("Write: ");DHEX(addr);DPRINT(" ");DHEXLN(towrite);
      Wire.beginTransmission(ID);
      Wire.write(addr >> 8);
      Wire.write(addr & 0xff);
      uint8_t done = 0;
      while( done < towrite ) {
        done++;
        Wire.write(*buf++);
      }
      success = Wire.endTransmission() == 0;
      // wait for write operation finished
      while( present() == false ) {
        _delay_ms(1);
      }
      _delay_ms(1);
      size -= towrite;
      addr += towrite;
    }
    return success;
  }

  bool getData (uint16_t addr,uint8_t* buf,uint16_t size) {
    bool success = false;
    while( size > 0 ) {
      uint16_t toread = PAGESIZE - (addr % PAGESIZE);
      if( size < toread) toread = size;
      //DPRINT("Read: ");DHEX(addr);DPRINT(" ");DHEXLN(toread);
      Wire.beginTransmission(ID);
      Wire.write(addr >> 8);
      Wire.write(addr & 0xff);
      if( Wire.endTransmission() == 0 ) {
        success = true;
        Wire.requestFrom(ID,(uint8_t)toread);
        uint8_t done = 0;
        while( done < toread ) {
          done++;
          if( Wire.available() ) {
            *buf++ = (uint8_t)Wire.read();
          }
        }
      }
      size -= toread;
      addr += toread;
    }
    return success;
  }

  bool clearData (uint16_t addr, uint16_t size) {
    bool success = false;
    while( size > 0 ) {
      uint16_t towrite = PAGESIZE - (addr % PAGESIZE);
      if( size < towrite) towrite = size;
//      DPRINT("Write: ");DHEX(addr);DPRINT(" ");DHEXLN(towrite);
      Wire.beginTransmission(ID);
      Wire.write(addr >> 8);
      Wire.write(addr & 0xff);
      uint8_t done = 0;
      while( done < towrite ) {
        done++;
        Wire.write(0);
      }
      success = Wire.endTransmission() == 0;
      // wait for write operation finished
      while( present() == false ) {
        _delay_ms(1);
      }
      // _delay_ms(20);
      size -= towrite;
      addr += towrite;
    }
    return success;
  }

};

template <uint8_t ID,uint16_t PAGES,uint8_t PAGESIZE>
class CachedAt24cX : public at24cX<ID,PAGES,PAGESIZE> {
  uint8_t  pagecache[PAGESIZE];
  uint16_t pageaddr;
  bool     dirty;
public:
  typedef at24cX<ID,PAGES,PAGESIZE> Base;
  CachedAt24cX () : pageaddr(0xffff), dirty(false) {
    DPRINT("CachedAt24c"); DDECLN((uint8_t)sizeof(pagecache));
  }

  void store () {
    writecache();
  }

protected:
  void writecache () {
    if( pageaddr != 0xffff && dirty == true ) {
      //DPRINT("WRITECACHE "); DHEXLN(pageaddr);
      Base::setData(pageaddr, pagecache, PAGESIZE);
      dirty = false;
    }
  }

  uint8_t* fillcache(uint16_t addr) {
    uint16_t paddr = addr & ~(PAGESIZE-1);
    if( pageaddr != paddr ) {
      writecache();
      pageaddr = paddr;
      //DPRINT("FILLCACHE "); DHEXLN(pageaddr);
      Base::getData(pageaddr,pagecache,PAGESIZE);
      dirty = false;
    }
    return pagecache;
  }

  void clearcache () {
    writecache();
    //DPRINT("CLEARCACHE\n");
    pageaddr = 0xffff;
  }

public:
  uint8_t getByte (uint16_t addr) {
    fillcache(addr);
    return pagecache[addr - pageaddr];
  }

  bool setByte (uint16_t addr, uint8_t d) {
    fillcache(addr);
    pagecache[addr - pageaddr] = d;
    dirty = true;
    return true;
  }

  bool getData (uint16_t addr,uint8_t* buf,uint16_t size) {
    writecache();
    return Base::getData(addr, buf, size);
  }

  bool setData (uint16_t addr,uint8_t* buf,uint16_t size) {
    clearcache();
    return Base::setData(addr, buf, size);
  }

  bool clearData (uint16_t addr, uint16_t size) {
    clearcache();
    return Base::clearData(addr, size);
  }

};

// define some known EEPROM types
typedef CachedAt24cX<0x50,128,32> at24c32;
typedef CachedAt24cX<0x50,256,32> at24c64;

#endif


template <class DRIVER=InternalEprom>
class StorageWrapper : public DRIVER {

public:
  StorageWrapper () {}

  bool setup (uint16_t checksum=0) {
    bool firststart = false;
    uint32_t mem;
    DRIVER::getData(0x0,(uint8_t*)&mem,4);
    uint32_t magic = 0xCAFE0000 | checksum;
    if(magic != mem) {
      DHEXLN(mem);
      DPRINT(F("Init Storage: "));
      DHEXLN(magic);
      // init eeprom
      DRIVER::setData(0x0,(uint8_t*)&magic,4);
      firststart = true;
    }
    return firststart;
  }

  bool setBits (uint16_t addr, uint8_t bits) {
    DRIVER::setByte(addr,DRIVER::getByte(addr) | bits);
    return true;
  }

  bool clearBits (uint16_t addr, uint8_t bits) {
    DRIVER::setByte(addr,DRIVER::getByte(addr) & ~bits);
    return true;
  }

  bool setData (uint16_t addr,uint8_t* buf,uint16_t size) {
    return DRIVER::setData(addr,buf,size);
  }

  template <class T>
  bool setData (uint16_t addr,const T& obj) {
    return DRIVER::setData(addr,(uint8_t*)&obj,sizeof(T));
  }

  bool getData (uint16_t addr,uint8_t* buf,uint16_t size) {
    return DRIVER::getData(addr,buf,size);
  }

  template <class T>
  bool getData (uint16_t addr,T* obj) {
    return DRIVER::getData(addr,(uint8_t*)obj,sizeof(T));
  }

  bool clearData (uint16_t addr, uint16_t size) {
    return DRIVER::clearData(addr,size);
  }

  void reset () {
    // clear magic
    clearData(0x0,4);
  }

  void dump (uint16_t start, uint16_t num) {
    for( uint16_t i=0; i<num; ++i, ++start ) {
      DHEX(DRIVER::getByte(start));
      DPRINT(F(" "));
    }
    DPRINT(F("\n"));
  }

};

#ifndef STORAGEDRIVER
#define STORAGEDRIVER InternalEprom
#endif
typedef StorageWrapper<STORAGEDRIVER > Storage;

inline Storage& storage () {
  static Storage instance;
  return instance;
}

}

#endif
