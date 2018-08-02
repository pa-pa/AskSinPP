//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ASKSINPP_h__
#define __ASKSINPP_h__

#define ASKSIN_PLUS_PLUS_VERSION "3.0.2"

#define ASKSIN_PLUS_PLUS_IDENTIFIER F("AskSin++ V" ASKSIN_PLUS_PLUS_VERSION " (" __DATE__ " " __TIME__ ")")

#include <stdint.h>

#ifdef ARDUINO_ARCH_STM32F1
  #define _delay_us(us) delayMicroseconds(us)
  inline void _delay_ms(uint32_t ms) { do { delayMicroseconds(1000); } while( (--ms) > 0); }

  #define NOT_AN_INTERRUPT -1
  #define digitalPinToInterrupt(pin) (pin)
  #define enableInterrupt(pin,handler,mode) attachInterrupt(pin,handler,mode)
  #define disableInterrupt(pin) detachInterrupt(pin)
  #define memcmp_P(src,dst,count) memcmp((src),(dst),(count))
#else
  typedef uint8_t uint8;
  typedef uint16_t uint16;
  #ifdef ARDUINO_ARCH_ATMEGA32
    inline uint8_t digitalPinToInterrupt(uint8_t pin) { return pin == 11 ? 1 : ( pin == 10 ? 0 : NOT_AN_INTERRUPT); } // D2 -> 0 && D3 -> 1
  #endif
  // if we have no EnableInterrupt Library - and also no PCINT - use polling
  #ifndef EnableInterrupt_h
    #define enableInterrupt(pin,handler,mode) pinpolling##pin().enable(pin,handler,mode)
    #define disableInterrupt(pin) pinpolling##pin().disable()
  #endif
#endif


#include <Storage.h>
#include <Pins.h>
#include <Debug.h>
#include <Activity.h>
#include <Led.h>
#include <AlarmClock.h>
#include <Message.h>
#include <Button.h>
#include <Radio.h>
#include <BatterySensor.h>


namespace as {


class AskSinBase {
  Storage storage;
public:

  static void pgm_read(uint8_t* dest,uint16_t adr,uint8_t size) {
    for( int i=0; i<size; ++i, ++dest ) {
      *dest = pgm_read_byte(adr + i);
    }
  }

  static uint16_t crc16 (uint16_t crc,uint8_t d) {
    crc ^= d;
    for( uint8_t i = 8; i != 0; --i ) {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0 );
    }
    return crc;
  }

  // get timer count in ticks
  static uint32_t byteTimeCvt(uint8_t tTime) {
    if( tTime == 0xff ) return 0xffffffff;
    const uint16_t c[8] = {1,10,50,100,600,3000,6000,36000};
    return decis2ticks( (uint32_t)(tTime & 0x1F) * c[tTime >> 5] );
  }

  static uint32_t byteTimeCvtSeconds (uint8_t tTime) {
    const uint8_t c[2] = {1,60};
    return seconds2ticks( (uint32_t)(tTime & 0x7F) * c[tTime >> 7] );
  }

  // get timer count in ticks
  static uint32_t intTimeCvt(uint16_t iTime) {
    if (iTime == 0x00) return 0x00;
    if (iTime == 0xffff) return 0xffffffff;

    uint8_t tByte;
    if ((iTime & 0x1F) != 0) {
      tByte = 2;
      for (uint8_t i = 1; i < (iTime & 0x1F); i++) tByte *= 2;
    } else tByte = 1;

    return decis2ticks( (uint32_t)tByte*(iTime>>5) );
  }

};

template <class StatusLed,class Battery,class Radio>
class AskSin : public AskSinBase {

public:
  typedef StatusLed LedType;
  typedef Battery   BatteryType;
  typedef Radio     RadioType;

  LedType      led;
  BatteryType  battery;
  RadioType    radio;
  Activity     activity;

  void init (const HMID& id) {
    srand((unsigned int&)id);
    led.init();
    radio.init();
    radio.enable();
    // start the system timer
    sysclock.init();
    // signal start to user
    led.set(LedStates::welcome);
    // delay first send by random time
    radio.setSendTimeout((rand() % 3500)+1000);
  }

  bool runready () {
    return sysclock.runready();
  }

  void sendPeer () {}

  // use radio timer to wait given millis
  void waitTimeout(uint16_t millis) {
    radio.waitTimeout(millis);
  }
};

}

#endif
