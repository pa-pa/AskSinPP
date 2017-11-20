//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ASKSINPP_h__
#define __ASKSINPP_h__

#define ASKSIN_PLUS_PLUS_VERSION "2.0.1"

#define ASKSIN_PLUS_PLUS_IDENTIFIER F("AskSin++ V" ASKSIN_PLUS_PLUS_VERSION " (" __DATE__ " " __TIME__ ")")

#ifdef ARDUINO_ARCH_STM32F1
  #define _delay_us(us) delayMicroseconds(us)
  inline void _delay_ms(uint32_t ms) { do { delayMicroseconds(1000); } while( (--ms) > 0); }

  #define enableInterrupt(pin,handler,mode) attachInterrupt(pin,handler,mode)
  #define disableInterrupt(pin) detachInterrupt(pin)
#endif


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

template <class StatusLed,class Battery,class Radio>
class AskSin {
public:
  typedef StatusLed LedType;
  typedef Battery   BatteryType;
  typedef Radio     RadioType;

  LedType     led;
  BatteryType battery;
  RadioType   radio;
  Activity    activity;

  void init (const HMID& id) {
    srand((unsigned int&)id);
    led.init();
    radio.init();
    radio.enable();
    // start the system timer
    sysclock.init();
    // signal start to user
    led.set(LedStates::welcome);
  }

  bool runready () {
    return sysclock.runready();
  }

  void sendPeer () {}

  // use radio timer to wait given millis
  void waitTimeout(uint16_t millis) {
    radio.waitTimeout(millis);
  }

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

};

}

#endif
