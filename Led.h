//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __STATUSLED_H__
#define __STATUSLED_H__

#include "Alarm.h"
#include "Debug.h"

namespace as {

class LedStates {
public:
  enum Mode { nothing=0, pairing=1,send=2, ack=3,
    nack=4, bat_low=5, welcome=6, key_long=7 };

  struct BlinkPattern {
    uint8_t  length;
    uint8_t  duration;
    uint8_t  pattern[6];
  };

  static const BlinkPattern single[8] PROGMEM;
  static const BlinkPattern dual1[8] PROGMEM;
  static const BlinkPattern dual2[8] PROGMEM;
};

template <uint8_t LEDPIN>
class Led : public Alarm, public LedStates {
private:
  BlinkPattern current;
  uint8_t step;   // current step in pattern
  uint8_t repeat; // current repeat of the pattern

  void copyPattern (Mode stat,const BlinkPattern* patt) {
    memcpy_P(&current,patt+stat,sizeof(BlinkPattern));
  }

  void next (AlarmClock& clock) {
    tick = decis2ticks(current.pattern[step++]);
    ((step & 0x01) == 0x01) ? ledOn() : ledOff();
    clock.add(*this);
  }

public:
  Led () : Alarm(0), step(0), repeat(0) {
    async(true);
  }
  virtual ~Led() {}

  void init () {
    pinMode(LEDPIN,OUTPUT);
    ledOff();
  }

  void set(Mode stat,const BlinkPattern* patt) {
    aclock.cancel(*this);
    ledOff();
    copyPattern(stat,patt);
    if( current.length > 0 ) {
      step = 0;
      repeat = 0;
      next(aclock);
    }
  }

  void ledOff () {
    digitalWrite(LEDPIN,LOW);
  }

  void ledOn () {
    digitalWrite(LEDPIN,HIGH);
  }

  void ledOn (uint8_t ticks) {
    if( active() == false && ticks > 0 ) {
      current.length = 2;
      current.duration = 1;
      current.pattern[0] = ticks2decis(ticks);
      current.pattern[1] = 0;
      // start the pattern
      step = repeat = 0;
      next(aclock);
    }
  }

  bool active () const { return current.length != 0; }

  virtual void trigger (AlarmClock& clock) {
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
      if( step < current.length ) {
        next(clock);
      }
      else {
        step = 0;
        if( current.duration == 0 || ++repeat < current.duration ) {
          next(clock);
        }
        else {
          ledOff();
          copyPattern(nothing,single);
        }
      }
    }
  }
};

template<uint8_t LEDPIN1>
class StatusLed : public LedStates {

public:

  Led<LEDPIN1> led1;

public:
  StatusLed () {}

  void init () {  led1.init(); }
  bool active () const { return led1.active(); }
  void ledOn (uint8_t ticks) { led1.ledOn(ticks); }
  void set(Mode stat) { led1.set(stat,single); }
};

template <uint8_t LEDPIN1,uint8_t LEDPIN2>
class DualStatusLed : public LedStates  {
private:
  Led<LEDPIN1> led1;
  Led<LEDPIN2> led2;
public:
  DualStatusLed () {}
  void init () { led1.init(); led2.init(); }
  bool active () const { return led1.active() || led2.active(); }
  void ledOn (uint8_t ticks) { led1.ledOn(ticks); led2.ledOn(ticks); }
  void set(Mode stat) { led1.set(stat,dual1); led2.set(stat,dual2); }
};

}

#endif
