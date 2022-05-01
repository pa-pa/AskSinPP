//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef ARDUINO
  #include "Arduino.h"
#endif

// #define NDEBUG

#ifdef NDEBUG

  #include <stdint.h>

  #define DPRINT(str)
  #define DPRINTLN(str)
  #define DHEX(b...)
  #define DHEXLN(b)
  #define DDEC(b)
  #define DDECLN(b)

  #define DINIT(baudrate,msg)
  #define DDEVINFO(dev)

#else

#ifdef ARDUINO

// can be used to move the debug output to Serial1 (useful on STM32 BluePill)
// must be defined in main .ino before any eader inclusion!
#ifndef DSERIAL
  #define DSERIAL Serial
#endif

  template <class T>
  inline void DPRINT(T str) { DSERIAL.print(str); }
  template <class T>
  inline void DPRINTLN(T str) { DPRINT(str); DPRINT(F("\r\n")); }
  inline void DHEX(uint8_t b) {
    if( b<0x10 ) DSERIAL.print('0');
    DSERIAL.print(b,HEX);
  }
  inline void DHEX(uint16_t b) { 
    if( b<0x10 ) DSERIAL.print(F("000")); 
    else if( b<0x100 ) DSERIAL.print(F("00"));
    else if( b<0x1000 ) DSERIAL.print(F("0"));
    DSERIAL.print(b,HEX);
  }
  inline void DHEX(uint32_t b) {
    if( b<0x10 ) DSERIAL.print(F("0000000"));
    else if( b<0x100 ) DSERIAL.print(F("000000"));
    else if( b<0x1000 ) DSERIAL.print(F("00000"));
    else if( b<0x10000 ) DSERIAL.print(F("0000"));
    else if( b<0x100000 ) DSERIAL.print(F("000"));
    else if( b<0x1000000 ) DSERIAL.print(F("00"));
    else if( b<0x10000000 ) DSERIAL.print(F("0"));
    DSERIAL.print(b,HEX);
  }

  template<typename TYPE>
  inline void DDEC(TYPE b) {
    DSERIAL.print(b,DEC);
  }

  #define DINIT(baudrate,msg) \
    DSERIAL.begin(baudrate); \
    DPRINTLN(msg);

  #define DDEVINFO(dev) \
    HMID devid; \
    dev.getDeviceID(devid); \
    DPRINT(F("ID: "));devid.dump(); \
    uint8_t serial[11]; \
    dev.getDeviceSerial(serial); \
    serial[10]=0; \
    DPRINT(F("  Serial: "));DPRINTLN((char*)serial);

#else

  #include <iostream>
  #include <iomanip>

  #ifndef F
    #define F(str) str
  #endif

  template <class T>
  inline void DPRINT(T str) { std::cout << str << std::flush; }
  template <class T>
  inline void DPRINTLN(T str) { std::cout << str << std::endl;  }
  inline void DHEX(uint8_t b) { std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)b; }
  inline void DHEX(uint16_t b) { std::cout << std::setw(4) << std::setfill('0') << std::hex << (int)b; }
  inline void DHEX(uint32_t b)  { std::cout << std::setw(8) << std::setfill('0') << std::hex << (int)b; }

  template<typename TYPE>
  inline void DDEC(TYPE b) { std::cout << std::dec << (int)b; }

#endif // ARDUINO

  inline void DHEX(const uint8_t* b,uint8_t l) {
    for( int i=0; i<l; i++, b++) {
      DHEX(*b); DPRINT(F(" "));
    }
  }
  inline void DHEXLN(uint8_t b) { DHEX(b); DPRINT(F("\r\n")); }
  inline void DHEXLN(uint16_t b) { DHEX(b); DPRINT(F("\r\n")); }
  inline void DHEXLN(uint32_t b) { DHEX(b); DPRINT(F("\r\n")); }
  template<typename TYPE>
  inline void DDECLN(TYPE b) { DDEC(b); DPRINT(F("\r\n")); }
  inline void DHEXLN(const uint8_t* b,uint8_t l) { DHEX(b,l); DPRINT(F("\r\n")); }

  #define DDEVLISTS(dev) \
    uint8_t cnls = dev.channels(); \
    DPRINT(F("channels: ")); DPRINTLN(cnls); \
    GenericList l = dev.getList0(); \
    DPRINT(F("cnl0, lst0: ")); l.dump(); DPRINT(F("\r\n")); \
    for (uint8_t i = 1; i <= cnls; ++i) { \
      l = dev.channel(i).getList1(); \
      DPRINT(F("cnl")); DPRINT(i); DPRINT(F(", lst1: ")); l.dump(); \
      uint8_t peers = dev.channel(i).peers(); \
      DPRINT(F("peers: ")); DPRINTLN(peers); \
      for (uint8_t j = 0; j < peers; ++j) { \
        Peer p = dev.channel(i).peerat(j); \
        GenericList l = dev.channel(i).getList3(j); \
        DPRINT(j); DPRINT(F(": ")); p.dump(); DPRINT(F(": ")); l.dump(); DPRINT(F("\r\n")); \
      } \
    } \


#endif

#endif
