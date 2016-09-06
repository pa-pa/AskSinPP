#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "AlarmClock.h"
#include "Debug.h"

namespace as {

class Button: public Alarm {

#define LONGTIME 30
#define DEBOUNCETIME 2
#define LONGREPEAT 3
#define LONGLONGTIME 30

public:
  enum States {
    none = 0,
    released = 1,
    pressed = 2,
    debounce = 3,
    longpressed = 4,
    longlongpressed = 5
  };

protected:
  uint8_t stat;
  uint8_t repeat;
  uint8_t pin;

public:
  Button() :
      Alarm(0), stat(none), repeat(0), pin(0) {
  }
  virtual ~Button() {
  }

  virtual void trigger(AlarmClock& clock) {
    bool low = digitalRead(pin) == LOW;
    switch (state()) {
    case released:
      state(none);
      break;

    case debounce:
      state(pressed);
      if (low == true) {
        // set timer for detect longpressed
        tick = LONGTIME - DEBOUNCETIME;
        clock.add(*this);
      } else {
        state(released);
        tick = DEBOUNCETIME;
        aclock.add(*this);
      }
      break;

    case pressed:
      state(longpressed);
      repeat = 0;
      tick = LONGREPEAT;
      clock.add(*this);
      break;

    case longpressed:
      DHEXLN(repeat);
      if (++repeat < 10) {
        state(longpressed);
        tick = LONGREPEAT;
        clock.add(*this);
      } else {
        state(longlongpressed);
      }
      break;
    }
  }

  virtual void state(uint8_t s) {
    /*
     switch(s) {
     case released: DPRINTLN("released"); break;
     case pressed: DPRINTLN("pressed"); break;
     case debounce: DPRINTLN("debounce"); break;
     case longpressed: DPRINTLN("longpressed"); break;
     case longlongpressed: DPRINTLN("longlongpressed"); break;
     }
     */
    stat = s;
  }

  uint8_t state() const {
    return stat;
  }

  void pinChange() {
    bool low = digitalRead(pin) == LOW;
    switch (state()) {
    case none:
      state(debounce);
      tick = DEBOUNCETIME;
      aclock.add(*this);
      break;

    case pressed:
    case longpressed:
    case longlongpressed:
      if (low == false) {
        aclock.cancel(*this);
        state(released);
        tick = DEBOUNCETIME;
        aclock.add(*this);
      }
      break;
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
