#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "AlarmClock.h"
#include "Debug.h"

namespace as {

class Button: public Alarm {

#define DEBOUNCETIME 2

public:
  enum States {
    none = 0,
    released = 1,
    pressed = 2,
    debounce = 3,
    longpressed = 4,
    longreleased = 5
  };

protected:
  uint8_t stat;
  uint8_t longpresstime;
  uint8_t pin;
  uint8_t pinstate;

public:
  Button() :
      Alarm(0), stat(none), longpresstime(4), pin(0), pinstate(HIGH) {
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
    switch (state()) {
    case released:
    case longreleased:
      state(none);
      break;

    case debounce:
      state(pressed);
      if (pinstate == LOW) {
        // set timer for detect longpressed
        tick = longpresstime - DEBOUNCETIME;
        clock.add(*this);
      } else {
        state(released);
        tick = DEBOUNCETIME;
        aclock.add(*this);
      }
      break;

    case pressed:
    case longpressed:
      if( pinstate == LOW) {
        state(longpressed);
        tick = longpresstime;
      }
      else {
        state() == pressed ? state(released) : state(longreleased);
        tick = DEBOUNCETIME;
      }
      clock.add(*this);
      break;
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
      switch (state()) {
      case none:
        state(debounce);
        tick = DEBOUNCETIME;
        aclock.add(*this);
        break;

      case pressed:
      case longpressed:
        if (pinstate == HIGH) {
          aclock.cancel(*this);
          tick = 0;
          aclock.add(*this);
        }
        break;
      }
    }
  }

  void init(uint8_t pin) {
    this->pin = pin;
    pinMode(pin, INPUT_PULLUP);
  }
};

// used to register interrupt

}

#endif
