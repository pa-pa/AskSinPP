
#ifndef __PINS_H__
#define __PINS_H__

#include <Arduino.h>

namespace as {

class ArduinoPins {
public:
  inline static void setOutput   (uint8_t pin) { pinMode(pin,OUTPUT); }
  inline static void setInput    (uint8_t pin) { pinMode(pin,INPUT);  }
  inline static void setHigh     (uint8_t pin) { digitalWrite(pin,HIGH); }
  inline static void setLow      (uint8_t pin) { digitalWrite(pin,LOW); }
  inline static uint8_t getState (uint8_t pin) { return digitalRead(pin); }
};

#ifdef __AVR__
#ifdef DDRA
  class PortA {
  public:
    inline static void setOutput   (uint8_t pin) { DDRA |= (1 << pin); }
    inline static void setInput    (uint8_t pin) { DDRA &= ~(1 << pin);  }
    inline static void setHigh     (uint8_t pin) { PORTA |= (1 << pin); }
    inline static void setLow      (uint8_t pin) { PORTA &= ~(1 << pin); }
    inline static uint8_t getState (uint8_t pin) { return (PINA & (1 << pin)) == 0 ? LOW : HIGH; }
  };
#endif
  class PortB {
  public:
    inline static void setOutput   (uint8_t pin) { DDRB |= (1 << pin); }
    inline static void setInput    (uint8_t pin) { DDRB &= ~(1 << pin);  }
    inline static void setHigh     (uint8_t pin) { PORTB |= (1 << pin); }
    inline static void setLow      (uint8_t pin) { PORTB &= ~(1 << pin); }
    inline static uint8_t getState (uint8_t pin) { return (PINB & (1 << pin)) == 0 ? LOW : HIGH; }
  };
  class PortC {
  public:
    inline static void setOutput   (uint8_t pin) { DDRC |= (1 << pin); }
    inline static void setInput    (uint8_t pin) { DDRC &= ~(1 << pin);  }
    inline static void setHigh     (uint8_t pin) { PORTC |= (1 << pin); }
    inline static void setLow      (uint8_t pin) { PORTC &= ~(1 << pin); }
    inline static uint8_t getState (uint8_t pin) { return (PINC & (1 << pin)) == 0 ? LOW : HIGH; }
  };
  class PortD {
  public:
    inline static void setOutput   (uint8_t pin) { DDRD |= (1 << pin); }
    inline static void setInput    (uint8_t pin) { DDRD &= ~(1 << pin);  }
    inline static void setHigh     (uint8_t pin) { PORTD |= (1 << pin); }
    inline static void setLow      (uint8_t pin) { PORTD &= ~(1 << pin); }
    inline static uint8_t getState (uint8_t pin) { return (PIND & (1 << pin)) == 0 ? LOW : HIGH; }
  };
#endif


}

#endif
