//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "Arduino.h"

//#define NDEBUG

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
  inline void DHEX(uint32_t b) {  }
  inline void DHEXLN(uint32_t b) {  }

  inline void DDEC(uint8_t b) {  }
  inline void DDECLN(uint8_t b) {  }
  inline void DDEC(uint16_t b) {  }
  inline void DDECLN(uint16_t b) {  }

#else


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

  inline void DHEX(uint32_t b) {
    if( b<0x10 ) Serial.print(F("0000000"));
    else if( b<0x100 ) Serial.print(F("000000"));
    else if( b<0x1000 ) Serial.print(F("00000"));
    else if( b<0x10000 ) Serial.print(F("0000"));
    else if( b<0x100000 ) Serial.print(F("000"));
    else if( b<0x1000000 ) Serial.print(F("00"));
    else if( b<0x10000000 ) Serial.print(F("0"));
    Serial.print(b,HEX);
  }
  inline void DHEXLN(uint32_t b) { DHEX(b); DPRINT(F("\n")); }

  inline void DDEC(uint8_t b) {
    Serial.print(b,DEC);
  }
  inline void DDECLN(uint8_t b) { DDEC(b); DPRINT(F("\n")); }
  inline void DDEC(uint16_t b) {
    Serial.print(b,DEC);
  }
  inline void DDECLN(uint16_t b) { DDEC(b); DPRINT(F("\n")); }

#endif

#endif
