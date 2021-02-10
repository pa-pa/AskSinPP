
#ifndef __PINS_H__
#define __PINS_H__

#include <Arduino.h>
#include <Alarm.h>
#include <AlarmClock.h>

#ifndef PINPOLL_COUNT_LOW
#define PINPOLL_COUNT_LOW 1
#endif
#ifndef PINPOLL_COUNT_HIGH
#define PINPOLL_COUNT_HIGH 1
#endif

namespace as {

class ArduinoPins {
public:
  inline static void setOutput   (uint8_t pin) { pinMode(pin,OUTPUT); }
  inline static void setInput    (uint8_t pin) { pinMode(pin,INPUT);  }
  inline static void setHigh     (uint8_t pin) { digitalWrite(pin,HIGH); }
  inline static void setLow      (uint8_t pin) { digitalWrite(pin,LOW); }
  inline static uint8_t getState (uint8_t pin) { return digitalRead(pin); }
#if defined (ARDUINO_ARCH_STM32F1)
  inline static void setPWM      (uint8_t pin) { pinMode(pin,PWM); }
  inline static void setPWM      (uint8_t pin,uint16_t value) { pwmWrite(pin,value); }
#elif defined (ARDUINO_ARCH_STM32) && defined (STM32L1xx)
  inline static void setPWM      (uint8_t pin) { pinMode(pin, OUTPUT); }
  inline static void setPWM      (uint8_t pin, uint8_t value) { analogWrite(pin, value); }
  inline static void setPWMFreq  (uint16_t freq) { analogWriteFrequency(freq); }
  inline static void setPWMRes   (uint8_t res)   { analogWriteResolution(res); }
#else
  inline static void setPWM      (uint8_t pin) { pinMode(pin,OUTPUT); }
  inline static void setPWM      (uint8_t pin,uint8_t value) { analogWrite(pin,value); }
#endif
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


  class PinPollingAlarm : public Alarm {
    private:
    uint8_t laststate;
    uint8_t pin;
    uint8_t mode;
    uint8_t count;
    void (*isr)(void);

    public:
    PinPollingAlarm () : Alarm(1), laststate(LOW), pin(0), mode(0), count(1), isr(0) {
      async(true);
    }
    virtual ~PinPollingAlarm () {}
    uint8_t readPin() {
      uint8_t p = digitalRead(pin);
      // if state has changed - lower count - if count 0 return changed state
      if( laststate != p ) {
        --count;
        // DDEC(count);
        if( count > 0 ) {
          p = laststate;
        }
        else {
          count = (p == LOW ? PINPOLL_COUNT_LOW : PINPOLL_COUNT_HIGH);
          // DPRINTLN("*");
        }
      }
      else {
        // we need 3 HIGH before we switch back to high
        count = (p == LOW ? PINPOLL_COUNT_LOW : PINPOLL_COUNT_HIGH);
      }
      return p;
    }
    virtual void trigger (AlarmClock& clock) {
      uint8_t state = readPin();
      if( state != laststate) {
        if( state == HIGH ) {
          if( mode == CHANGE || mode == RISING ) {
            isr();
          }
        }
        else {
          if( mode == CHANGE || mode == FALLING ) {
            isr();
          }
        }
        laststate = state;
      }
      set(1);
      clock.add(*this);
    }

    void enable (uint8_t p,void (*func)(void),uint8_t m) {
      sysclock.cancel(*this);
      pin = p;
      isr = func;
      mode = m;
      laststate = digitalRead(pin);
      DPRINT(F("Enable PinPolling: ")); DDEC(pin); DPRINT(F(" - ")); DDECLN(mode);
      set(1);
      sysclock.add(*this);
    }
    void disable () {
      DPRINT(F("Disable PinPolling: ")); DDECLN(pin);
      sysclock.cancel(*this);
    }
  };

  inline PinPollingAlarm& pinpolling4()  { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling5()  { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling6()  { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling7()  { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling8()  { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling9()  { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling10() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling11() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling12() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling13() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling14() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling15() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling16() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling17() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling18() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling19() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling20() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling21() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling22() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling23() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling24() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling25() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling26() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling27() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling28() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling29() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling30() { static PinPollingAlarm p; return p; }
  inline PinPollingAlarm& pinpolling31() { static PinPollingAlarm p; return p; }

}

#endif
