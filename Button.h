#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "AlarmClock.h"
#include "Debug.h"

namespace as {

class Button: public Alarm {

#define DEBOUNCETIME millis2ticks(200)

public:
  enum States {
    invalid = 0,
    none = 1,
    released = 2,
    pressed = 3,
    debounce = 4,
    longpressed = 5,
    longreleased = 6,
  };

protected:
  uint8_t stat;
  uint8_t longpresstime;
  uint8_t pin;
  uint8_t pinstate;

public:
  Button() :
      Alarm(0), stat(none), longpresstime(millis2ticks(400)), pin(0), pinstate(HIGH) {
  }
  virtual ~Button() {
  }

  void setLongPressTime(uint8_t t) {
    longpresstime = t;
  }

  uint8_t getPin () {
    return pin;
  }

  virtual void trigger(AlarmClock& clock) {
    uint8_t nextstate = invalid;
    uint8_t nexttick = 0;
    switch ( state() ) {
    case released:
    case longreleased:
      nextstate = none;
      break;

    case debounce:
      nextstate = pressed;
      if (pinstate == LOW) {
        // set timer for detect longpressed
        nexttick = longpresstime - DEBOUNCETIME;
      } else {
        nextstate = released;
        nexttick = DEBOUNCETIME;
      }
      break;

    case pressed:
    case longpressed:
      if( pinstate == LOW) {
        nextstate = longpressed;
        nexttick = longpresstime;
      }
      break;
    }
    // reactivate alarm if needed
    if( nexttick != 0 ) {
      tick = nexttick;
      clock.add(*this);
    }
    // trigger the state change
    if( nextstate != invalid ) {
      state(nextstate);
    }
  }

  virtual void state(uint8_t s) {
     switch(s) {
     case released: DPRINTLN(" released"); break;
     case pressed: DPRINTLN(" pressed"); break;
     case debounce: DPRINTLN(" debounce"); break;
     case longpressed: DPRINTLN(" longpressed"); break;
     case longreleased: DPRINTLN(" longreleased"); break;
     default: DPRINTLN(""); break;
     }
    stat = s;
  }

  uint8_t state() const {
    return stat;
  }

  void check() {
    uint8_t ps = digitalRead(pin);
    if( pinstate != ps ) {
      pinstate = ps;
      uint8_t nexttick = 0;
      uint8_t nextstate = state();
      switch ( state() ) {
      case none:
        nextstate = debounce;
        nexttick = DEBOUNCETIME;
        break;

      case pressed:
      case longpressed:
        if (pinstate == HIGH) {
          nextstate = state() == pressed ? released : longreleased;
          nexttick = DEBOUNCETIME;
        }
        break;
      }
      if( nexttick != 0 ) {
        aclock.cancel(*this);
        tick = nexttick;
        aclock.add(*this);
      }
      if( nextstate != state () ) {
        state(nextstate);
      }
    }
  }

  void init(uint8_t pin) {
    this->pin = pin;
    pinMode(pin, INPUT_PULLUP);
  }
};

}

#endif
