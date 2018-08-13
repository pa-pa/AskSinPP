//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-08-10 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __PWM_H__
#define __PWM_H__

#include <Arduino.h>

namespace as {

#if ARDUINO_ARCH_AVR or ARDUINO_ARCH_ATMEGA32
// we use this table for the dimmer levels
static const uint8_t pwmtable[32] PROGMEM = {
    1, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23,
    27, 32, 38, 45, 54, 64, 76, 91, 108, 128, 152, 181, 215, 255
};
template<uint8_t STEPS=200>
class PWM8 {
  uint8_t  pin;
public:
  PWM8 () : pin(0) {}
  ~PWM8 () {}

  void init(uint8_t p) {
    pin = p;
    pinMode(pin,OUTPUT);
  }
  void set(uint8_t value) {
    uint8_t offset = value*31/STEPS;
    uint8_t pwm = pgm_read_word (& pwmtable[offset]);
    analogWrite(pin,pwm);
  }
};
#endif

#ifdef ARDUINO_ARCH_STM32F1

template<uint8_t STEPS=200,uint16_t FREQU=65535>
class PWM16 {
  float R;
  uint8_t pin;
public:
  PWM16 () : pin(0) {
    R = (STEPS * log10(2))/(log10(FREQU));
  }
  ~PWM16 () {}

  void init(uint8_t p) {
    pin = p;
    pinMode(pin,PWM);
    set(0);
  }

  void set(uint8_t value) {
    uint16_t duty = 0;
    if ( value == STEPS) {
      duty = FREQU;
    }
    else {
      duty = pow(2,(value/R)) + 4;
      //duty = exp(value/18.0) + 4;
    }
    pwmWrite(pin,duty);
  }
};

#endif

};

#endif
