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

#if defined ARDUINO_ARCH_STM32 && defined STM32WL
  #include <EEPROM.h>
#endif

#if defined ARDUINO_ARCH_ESP32 || defined ARDUINO_ARCH_RP2040
  #include "AlarmClock.h"
  #include <EEPROM.h>
#endif

#ifdef ARDUINO_ARCH_EFM32
  #include "AlarmClock.h"
  #include "eeprom_emulation.h"
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
  #elif defined(MCU_STM32F103RC) // 256k
    #define FlashPageSize 0x400
    #define FlashStartAddress 0x0803fc00  // Page255
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
#elif defined ARDUINO_ARCH_STM32 && defined STM32L1xx
  // this works for STM32L151C8, todo: check for other variants with more flash
  #define EEADDR_EEPROM_START  0x08080000
  #define EEINFO_EEPROM_SIZE   4096
  #define E2END EEINFO_EEPROM_SIZE

  void eeprom_read_block(void* buf, const void* addr, size_t size) {
    // check if address is within our eeprom
    uint32_t offset = (uintptr_t)addr;
    if (offset >= EEINFO_EEPROM_SIZE) return;
    offset += EEADDR_EEPROM_START;
    uint8_t* ptr = (uint8_t*)buf;
    // get the requested bytes
    for (uint16_t i = 0; i < size; i++) {
      uint32_t address = offset + i;
      ptr[i] = *(volatile uint8_t*)address;
    }
  }

  void eeprom_write_block(const void* buf, void* addr, size_t size) {
    // check if address is within our eeprom
    uint32_t offset = (uintptr_t)addr;
    if (offset >= EEINFO_EEPROM_SIZE) return;
    offset += EEADDR_EEPROM_START;
    uint8_t* ptr = (uint8_t*)buf;
    //DPRINT("offset: "); DHEX(offset); DPRINT(", data: "); DHEXLN(ptr,16);
    // write the given bytes
    HAL_FLASHEx_DATAEEPROM_Unlock();
    for (uint16_t i = 0; i < size; i++) {
      uint32_t address = offset + i;
      HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, address, ptr[i]);
    }
    HAL_FLASHEx_DATAEEPROM_Lock();
}

#elif defined ARDUINO_ARCH_STM32 && defined STM32WL
  // standard lib provides 4096 byte emulated eeprom in flash
  // check if eeprom lib also works for stm32l1xx so that we can allign sections

  #define EEINFO_EEPROM_SIZE   EEPROM.length()
  #define E2END EEINFO_EEPROM_SIZE

  void eeprom_read_block(void* buf, const void* addr, size_t size) {
    uint8_t* ptr = (uint8_t*)buf;
    uint32_t offset = (uintptr_t)addr;

    for (uint16_t i = 0; i < size; i++) {
      uint16_t address = offset + i;
      ptr[i] = EEPROM.read(address);
    }
  }
  
  void eeprom_write_block(const void* buf, void* addr, size_t size) {
    // we use update instead of write because it is much faster if bytes are already correct in the eeprom
    uint8_t* ptr = (uint8_t*)buf;
    uint32_t offset = (uintptr_t)addr;

    for (uint16_t i = 0; i < size; i++) {
      uint16_t address = offset + i;
      EEPROM.update(address, ptr[i]);
    }
    DPRINT('-');
  }

#elif defined ARDUINO_ARCH_ESP32 || defined ARDUINO_ARCH_RP2040
  //ESP32 Arduino libraries emulate EEPROM using a sector (4 kilobytes) of flash memory.
  #define EEINFO_EEPROM_SIZE  4096
  #define E2END EEINFO_EEPROM_SIZE
#ifdef ARDUINO_ARCH_RP2040
  #define IRAM_ATTR
#endif

  void IRAM_ATTR initEEPROM() {
    static bool initDone = false;
    if (initDone == false) {
      initDone = true;
      DPRINT(F("Init EEPROM - "));
      EEPROM.begin(EEINFO_EEPROM_SIZE);
      DPRINTLN(F("DONE"));
    }
  }

  unsigned char IRAM_ATTR eeprom_read_byte(unsigned char * pos)  {
    initEEPROM();
    uint8_t result = EEPROM.read(int(pos));
    //DPRINT("eeprom_read_byte (");DDEC(int(pos));DPRINT(") ");DHEXLN(result);
    return result;
  }

  void IRAM_ATTR eeprom_read_block(void * __dst, const void * __src, size_t __n) {
    initEEPROM();
    sysclock.disable();
    for (int i = 0; i < __n; i++) {
      *((char *)__dst + i) = eeprom_read_byte((uint8_t *)__src + i);
    }
    sysclock.enable();
  }

  void IRAM_ATTR eeprom_write_block( const void * src, const void * dst,  size_t __n) {
    initEEPROM();

    //https://esp32.com/viewtopic.php?t=13861
    //due to a bug, we have to disable the timer before committing to the EEPROM
    sysclock.disable();
    int pos = int(dst);
    for (int i = 0; i < __n; i++) {
      byte data = *((unsigned  char*)src + i);
      EEPROM.write(pos + i, data);
    }
    EEPROM.commit();
    sysclock.enable();
  }
#elif defined ARDUINO_ARCH_EFM32
  #define EEPROM_PAGES  3  // allocate at least 3 pages
  #define E2END         1024
  #define VECTOR_SIZE (16+30)
  EE_Variable_TypeDef eeprom_var[(E2END >> 1)];
  uint32_t vectorTable[VECTOR_SIZE] __attribute__ ((aligned(256)));
  __attribute__ ((section(".ram")))
  inline void moveInterruptVectorToRam(void) {
    memcpy(vectorTable, (uint32_t*)SCB->VTOR, sizeof(uint32_t) * VECTOR_SIZE);
    SCB->VTOR = (uint32_t)vectorTable;
  }

  void  initEEPROM() {
    static bool initDone = false;
    if (initDone == false) {
      initDone = true;
      DPRINT(F("Init EEPROM - Pages:")); DDEC(EEPROM_PAGES);
      moveInterruptVectorToRam();
      MSC_Init();
      EE_Init(EEPROM_PAGES);
      for (uint16_t i = 0; i< (E2END >> 1);i++) EE_DeclareVariable(&eeprom_var[i]);
      DPRINTLN(F(" DONE"));
    }
  }

  void eeprom_write_byte(uint16_t addr, byte dat) {
    uint16_t readValue;
    EE_Read(&eeprom_var[addr >> 1], &readValue);
    //DPRINT("eeprom_write_byte ");DHEX(addr); DPRINT(" : ");DHEXLN(dat);
    noInterrupts();
    EE_Write(&eeprom_var[addr >> 1], (addr % 2 == 0) ? ((readValue >> 8) << 8) + dat :  (dat << 8 ) + (readValue & 0xFF));
    interrupts();
  }

  byte eeprom_read_byte(unsigned char * pos)  {
    uint16_t readValue;
    EE_Read(&eeprom_var[(int)pos >> 1], &readValue);
    byte val = ((int)pos % 2 == 0) ? readValue & 0xFF : readValue >> 8;
    //DPRINT("eeprom_read_byte ");DHEX((uint8_t)pos); DPRINT(" : ");DHEXLN(val);
    return val;
  }

  void  eeprom_read_block(void * __dst, const void * __src, size_t __n) {
    initEEPROM();
    for (size_t i = 0; i < __n; i++) {
      *((char *)__dst + i) = eeprom_read_byte((uint8_t *)__src + i);
    }
  }

  void  eeprom_write_block( const void * src, const void * dst,  size_t __n) {
    initEEPROM();
    int pos = int(dst);
    for (size_t i = 0; i < __n; i++) {
      byte data = *((unsigned  char*)src + i);
      eeprom_write_byte(pos + i, data);
    }
  }
#endif


#if not defined ARDUINO
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
#ifdef ARDUINO_ARCH_STM32F1
    return 1024;
#else
    return E2END + 1; // last EEPROM address + 1
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


#if defined(TwoWire_h) || defined(_WIRE_H_) || defined(_TWOWIRE_H_) || defined(_WIREBASE_H_)

#ifndef BUFFER_LENGTH
  #ifdef TWI_BUFFER_SIZE // MightyCore
    #define BUFFER_LENGTH TWI_BUFFER_SIZE
  #else // fall back to default value
    #define BUFFER_LENGTH 32
  #endif
#endif

// with help of https://github.com/JChristensen/extEEPROM

template <uint8 ID,uint16_t EEPROM_NUM_PAGES,uint8_t EEPROM_PAGESIZE>
class at24cX {
public:
  at24cX () {}

  bool present () {
    Wire.beginTransmission(ID);
    Wire.write((uint8_t)0);  //high addr byte
    Wire.write((uint8_t)0);  //low addr byte
    return Wire.endTransmission() == 0;
  }

  uint16_t size () {
    return EEPROM_NUM_PAGES * EEPROM_PAGESIZE;
  }

  void store () {}

  uint8 getByte (uint16_t addr) {
    uint8 b = 0;
    Wire.beginTransmission(ID);
    Wire.write(addr >> 8);
    Wire.write(addr & 0xff);
    if( Wire.endTransmission() == 0 ) {
      Wire.requestFrom(ID,(uint8_t)1);
      b = Wire.read();
    }
    return b;
  }

  bool setByte (uint16_t addr, uint8 d) {
    bool success = true;
    Wire.beginTransmission(ID);
    Wire.write(addr >> 8);
    Wire.write(addr & 0xff);
    Wire.write(d);
    success = Wire.endTransmission() == 0;
    // wait for write operation finished
    if( success == true ) {
      success = waitComplete();
    }
    return success;
  }

  uint16_t calcBlockSize(uint16_t addr, uint16_t size) {
    uint16_t block = EEPROM_PAGESIZE - (addr % EEPROM_PAGESIZE);
    // BUFFER_LENGTH from Wire.h - 2 byte address
    block = (BUFFER_LENGTH - 2) < block ? BUFFER_LENGTH - 2 : block;
    return (size < block) ? size : block;
  }


  bool setData (uint16_t addr,uint8* buf,uint16_t size) {
    // DPRINT("setData: ");DHEX(addr);DPRINT(" ");DDECLN(size);
    bool success = true;
    while( success == true && size > 0 ) {
      uint16_t towrite = calcBlockSize(addr, size);
      // DPRINT("  write: ");DHEX(addr);DPRINT(" ");DDECLN(towrite);
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
      if( success == true ) {
        success = waitComplete();
      }
      else {
        DPRINTLN(F("ERROR EEPROM WRITE"));
      }
      size -= towrite;
      addr += towrite;
    }
    return success;
  }

  bool getData (uint16_t addr,uint8* buf,uint16_t size) {
    bool success = true;
    while( success == true && size > 0 ) {
      uint16_t toread = calcBlockSize(addr, size);
      //DPRINT("Read: ");DHEX(addr);DPRINT(" ");DHEXLN(toread);
      Wire.beginTransmission(ID);
      Wire.write(addr >> 8);
      Wire.write(addr & 0xff);
      success = Wire.endTransmission() == 0;
      if( success == true ) {
        Wire.requestFrom(ID,(uint8_t)toread);
        uint8_t done = 0;
        while( done < toread ) {
          done++;
          *buf++ = (uint8_t)Wire.read();
        }
      }
      size -= toread;
      addr += toread;
    }
    return success;
  }

  bool clearData (uint16_t addr, uint16_t size) {
    // DPRINT("clearData: ");DHEX(addr);DPRINT(" ");DDECLN(size);
    bool success = true;
    while( success == true && size > 0 ) {
      uint16_t towrite = calcBlockSize(addr, size);
      // DPRINT("  clear: ");DHEX(addr);DPRINT(" ");DDECLN(towrite);
      Wire.beginTransmission(ID);
      Wire.write(addr >> 8);
      Wire.write(addr & 0xff);
      uint8_t done = 0;
      while( done < towrite ) {
        done++;
        Wire.write((uint8_t)0);
      }
      success = Wire.endTransmission() == 0;
      // wait for write operation finished
      if( success == true ) {
        success = waitComplete();
      }
      else {
        DPRINTLN(F("ERROR EEPROM CLEAR"));
      }
      size -= towrite;
      addr += towrite;
    }
//    DPRINTLN("clearData done");
    return success;
  }

  bool waitComplete () {
    //wait up to 50ms for the write to complete
    for (uint8_t i=25; i; --i) {
      _delay_ms(2); //no point in waiting too fast
      if( present() == true ) {
        return true;
      }
    }
    DPRINTLN(F("ERROR EEPROM WAIT"));
    return false;
  }

};


#if defined ARDUINO_ARCH_EFM32
template <uint8 ID,uint16_t EEPROM_NUM_PAGES,uint16_t EEPROM_PAGESIZE>
class m24mXX {
public:
  m24mXX () {}

  bool present () {
    Wire.beginTransmission(ID);
    Wire.write((uint8_t)0);  //high addr byte
    Wire.write((uint8_t)0);  //low addr byte
    return Wire.endTransmission() == 0;
  }

  size_t size () {
    return EEPROM_NUM_PAGES * EEPROM_PAGESIZE;
  }

  void store () {}

  uint8_t getBusyStatus(void) {
    uint8_t retVal = 0;
    Wire.beginTransmission((uint8_t)((ID << 3) ));
    retVal = Wire.endTransmission();
    return retVal;
  }

  uint8 getByte (uint16_t addr) {
    uint8_t b = 0;

    Wire.beginTransmission((uint8_t)((ID << 3) | ((addr >> 16) & 0x01)));
    Wire.write((uint8_t)((addr >> 8) & 0xFF));
    Wire.write((uint8_t)(addr & 0xFF));

    if ( Wire.endTransmission() == 0 ) {
      Wire.requestFrom(((ID << 3)  | ((addr >> 16) & 0x01)), 1);
      if (Wire.available()) {
        b = Wire.read();
      }
    }
    return b;
  }

  bool setByte (uint16_t addr, uint8 d) {
    bool success = false;
    Wire.beginTransmission((uint8_t)((ID << 3)  | ((addr >> 16) & 0x01)));
    Wire.write((uint8_t)((addr >> 8) & 0xFF));
    Wire.write((uint8_t)(addr & 0xFF));
    Wire.write(d);
    success = Wire.endTransmission();
    while (getBusyStatus() != 0) {
      _delay_ms(2);
    }
    return success;
  }

  bool setData (uint16_t addr,uint8* buf,size_t size) {
    Wire.beginTransmission((uint8_t)((ID << 3)  | ((addr >> 16) & 0x01)));
    Wire.write((uint8_t)((addr >> 8) & 0xFF));
    Wire.write((uint8_t)(addr & 0xFF));
    size_t bytesWritten = Wire.write(buf, size);
    Wire.endTransmission();
    while (getBusyStatus() != 0) {
      _delay_ms(2);
    }
    return bytesWritten == size;
  }

  bool getData (uint16_t addr,uint8* buf,uint16_t size) {
    Wire.beginTransmission((uint8_t)((ID << 3) | ((addr >> 16) & 0x01)));
    Wire.write((uint8_t)((addr >> 8) & 0xFF));
    Wire.write((uint8_t)(addr & 0xFF));

    Wire.endTransmission(0);
    Wire.requestFrom(((ID << 3)  | ((addr >> 16) & 0x01)), size);

    uint32_t index;
    for (index = 0; index < size; index++ ) {
      if (Wire.available()) {
        buf[index] = Wire.read();
      }
    }

    return index + 1 == size;
  }

  bool clearData (uint16_t addr, size_t size) {
    DPRINT("clearData");//DPRINT(": ");DHEX(addr);DPRINT(" ");DDEC(size);DPRINT("...");
    bool success = true;
    for (uint16_t i = 0; i < size; i++) {
      setData(addr+i,0,1);
    }
    DPRINTLN(" - done");
    return success;
  }
};
#endif

template <uint8_t ID,uint16_t PAGES,uint8_t EEPROM_PAGESIZE>
class CachedAt24cX : public at24cX<ID,PAGES, EEPROM_PAGESIZE> {
  uint8_t  pagecache[EEPROM_PAGESIZE];
  uint16_t pageaddr;
  bool     dirty;
public:
  typedef at24cX<ID,PAGES, EEPROM_PAGESIZE> Base;
  CachedAt24cX () : pageaddr(0xffff), dirty(false) {}

  void store () {
    writecache();
  }

protected:
  void writecache () {
    if( pageaddr != 0xffff && dirty == true ) {
      // DPRINT("WRITECACHE "); DHEXLN(pageaddr);
      Base::setData(pageaddr, pagecache, EEPROM_PAGESIZE);
      dirty = false;
    }
  }

  uint8_t* fillcache(uint16_t addr) {
    uint16_t paddr = addr & ~(EEPROM_PAGESIZE -1);
    if( pageaddr != paddr ) {
      writecache();
      pageaddr = paddr;
      // DPRINT("FILLCACHE "); DHEXLN(pageaddr);
      Base::getData(pageaddr,pagecache, EEPROM_PAGESIZE);
      dirty = false;
    }
    return pagecache;
  }

  void clearcache () {
    writecache();
    // DPRINT("CLEARCACHE\n");
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
      _delay_ms(200);
#ifdef ARDUINO_ARCH_EFM32
      if ( !EE_Init(EEPROM_PAGES) ) {
        EE_Format(EEPROM_PAGES);
      }
#endif
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

extern void* __gb_store;

class Storage : public StorageWrapper<STORAGEDRIVER > {
public:
  Storage () {
    __gb_store = this;
  }
};


static inline Storage& storage () {
  return *((Storage*)__gb_store);
}

#define STORAGE_CFG_START 0x04
class StorageConfig {
  uint8_t  size;
public:
  StorageConfig (uint8_t s) : size(s) {}

  uint8_t checksum () const {
    uint8_t sum = 0x5e;
    for( uint8_t i=0; i<size-1; ++i ) {
      sum ^= getByte(i);
    }
    return sum;
  }

  void validate () {
    setByte(size-1,checksum());
  }

  bool valid () const {
    return getByte(size-1) == checksum();
  }

  void clear () {
    storage().clearData(STORAGE_CFG_START, size);
  }

  uint8_t getSize () const {
    return size-1;
  }

  uint8_t getByte (uint8_t offset) const {
    return storage().getByte(STORAGE_CFG_START+offset);
  }

  void setByte(uint8_t offset,uint8_t data) {
    storage().setByte(STORAGE_CFG_START+offset,data);
  }
};

/**
 * This class is used to access the free EEPROM
 */
class UserStorage {
  uint16_t start;
public:
  UserStorage (uint16_t s) : start(s) {}

  uint16_t getAddress () const {
    return start;
  }

  uint16_t getSize () const {
    return storage().size() - getAddress();
  }

  void clear () {
    storage().clearData(getAddress(),getSize());
  }

  uint8_t getByte (uint16_t offset) const {
    return storage().getByte(getAddress()+offset);
  }

  void setByte(uint16_t offset,uint8_t data) {
    storage().setByte(getAddress()+offset,data);
  }

  void setData (uint16_t offset,uint8_t* buf,uint16_t size) {
    storage().setData(getAddress()+offset,buf,size);
  }

  void getData (uint16_t offset,uint8_t* buf,uint16_t size) {
    storage().getData(getAddress()+offset,buf,size);
  }

  template <class T>
  void setData (uint16_t offset,const T& obj) {
    setData(offset,(uint8_t*)&obj,sizeof(T));
  }

  template <class T>
  void getData (uint16_t offset,T* obj) {
    getData(offset,(uint8_t*)obj,sizeof(T));
  }

};

}

#endif
