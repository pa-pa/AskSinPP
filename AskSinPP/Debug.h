
#ifndef __DEBUG_H__
#define __DEBUG_H__

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  inline const char* F(const char* c) { return c; }
#endif

#define NDEBUG

#ifdef NDEBUG

  template <class T>
  inline void DPRINT(T str) { }
  template <class T>
  inline void DPRINTLN(T str) {  }
  inline void DHEX(uint8_t b) {  }
  inline void DHEX(const uint8_t* b,uint8_t l) {  }
  inline void DHEXLN(uint8_t b) {  }
  inline void DHEX(uint16_t b) {  }
  inline void DHEXLN(uint16_t b) {  }

#else

#if defined(ARDUINO) && ARDUINO >= 100

  template <class T>
  inline void DPRINT(T str) { Serial.print(str); }
  template <class T>
  inline void DPRINTLN(T str) { DPRINT(str); DPRINT(F("\n")); }
  inline void DHEX(uint8_t b) {
    if( b<0x10 ) Serial.print('0');
    Serial.print(b,HEX);
  }
  inline void DHEX(const uint8_t* b,uint8_t l) {
    for( int i=0; i<l; i++, b++) {
      DHEX(*b); DPRINT(F(" "));
    }
    DPRINT(F("\n"));
  }
  inline void DHEXLN(uint8_t b) { DHEX(b); DPRINT(F("\n")); }
  inline void DHEX(uint16_t b) { 
    if( b<0x10 ) Serial.print(F("000")); 
    else if( b<0x100 ) Serial.print(F("00"));
    else if( b<0x1000 ) Serial.print(F("0"));
    Serial.print(b,HEX);
  }
  inline void DHEXLN(uint16_t b) { DHEX(b); DPRINT(F("\n")); }

#else

  #include <unistd.h>
  #include <iostream>
  #include <iomanip>

  template <class T>
  inline void DPRINT(T str) { std::cout << str; }
  template <class T>
  inline void DPRINTLN(T str) { DPRINT(str); std::cout << std::endl; }
  inline void DHEX(uint8_t b) { std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)b; }
  inline void DHEXLN(uint8_t b) { DHEX(b); std::cout << std::endl; }
  inline void DHEX(uint16_t b) { std::cout << std::setw(4) << std::setfill('0') << std::hex << (int)b; }
  inline void DHEXLN(uint16_t b) { DHEX(b); std::cout << std::endl; }

#endif

#endif

#endif
