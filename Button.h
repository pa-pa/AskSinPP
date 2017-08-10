//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "AlarmClock.h"
#include "Debug.h"

#ifdef ARDUINO_ARCH_AVR
  typedef uint8_t WiringPinMode;
#endif

namespace as {

template <uint8_t OFFSTATE=HIGH,uint8_t ONSTATE=LOW,WiringPinMode MODE=INPUT_PULLUP>
class StateButton: public Alarm {

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
  uint8_t  stat     : 3;
  uint8_t  pinstate : 1;
  uint8_t  pin;
  uint16_t longpresstime;

public:
  StateButton() :
      Alarm(0), stat(none), pinstate(OFFSTATE), pin(0), longpresstime(millis2ticks(400))  {
  }
  virtual ~StateButton() {
  }

  void setLongPressTime(uint16_t t) {
    longpresstime = t;
  }

  uint8_t getPin () {
    return pin;
  }

  virtual void trigger(AlarmClock& clock) {
    uint8_t  nextstate = invalid;
    uint16_t nexttick = 0;
    switch ( state() ) {
    case released:
    case longreleased:
      nextstate = none;
      break;

    case debounce:
      nextstate = pressed;
      if (pinstate == ONSTATE) {
        // set timer for detect longpressed
        nexttick = longpresstime - DEBOUNCETIME;
      } else {
        nextstate = released;
        nexttick = DEBOUNCETIME;
      }
      break;

    case pressed:
    case longpressed:
      if( pinstate == ONSTATE) {
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
     case released: DPRINTLN(F(" released")); break;
     case pressed: DPRINTLN(F(" pressed")); break;
     case debounce: DPRINTLN(F(" debounce")); break;
     case longpressed: DPRINTLN(F(" longpressed")); break;
     case longreleased: DPRINTLN(F(" longreleased")); break;
     default: DPRINTLN(F("")); break;
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
      uint16_t nexttick = 0;
      uint8_t  nextstate = state();
      switch ( state() ) {
      case none:
        nextstate = debounce;
        nexttick = DEBOUNCETIME;
        break;

      case pressed:
      case longpressed:
        if (pinstate == OFFSTATE) {
          nextstate = state() == pressed ? released : longreleased;
          nexttick = DEBOUNCETIME;
        }
        break;
      }
      if( nexttick != 0 ) {
        sysclock.cancel(*this);
        tick = nexttick;
        sysclock.add(*this);
      }
      if( nextstate != state () ) {
        state(nextstate);
      }
    }
  }

  void init(uint8_t pin) {
    this->pin = pin;
    pinMode(pin, MODE);
  }
};

// define standard button switches to GND
typedef StateButton<HIGH,LOW,INPUT_PULLUP> Button;

template <class DEVTYPE,uint8_t OFFSTATE=HIGH,uint8_t ONSTATE=LOW,WiringPinMode MODE=INPUT_PULLUP>
class ConfigButton : public StateButton<HIGH,LOW,INPUT_PULLUP> {
  DEVTYPE& device;
public:
  typedef StateButton<HIGH,LOW,INPUT_PULLUP> ButtonType;

  ConfigButton (DEVTYPE& dev,uint8_t longpresstime=3) : device(dev) {
    setLongPressTime(seconds2ticks(longpresstime));
  }
  virtual void state (uint8_t s) {
    uint8_t old = ButtonType::state();
    ButtonType::state(s);
    if( s == ButtonType::released ) {
      device.startPairing();
    }
    else if( s == ButtonType::longpressed ) {
      if( old == ButtonType::longpressed ) {
        device.reset(); // long pressed again - reset
      }
      else {
        device.led().set(LedStates::key_long);
      }
    }
  }
};

template <class DEVTYPE,uint8_t OFFSTATE=HIGH,uint8_t ONSTATE=LOW,WiringPinMode MODE=INPUT_PULLUP>
class ConfigToggleButton : public StateButton<HIGH,LOW,INPUT_PULLUP> {
  DEVTYPE& device;
public:
  typedef StateButton<HIGH,LOW,INPUT_PULLUP> ButtonType;

  ConfigToggleButton (DEVTYPE& dev,uint8_t longpresstime=3) : device(dev) {
    setLongPressTime(seconds2ticks(longpresstime));
  }
  virtual void state (uint8_t s) {
    uint8_t old = ButtonType::state();
    ButtonType::state(s);
    if( s == ButtonType::released ) {
      RemoteEventMsg& msg = (RemoteEventMsg&)device.message();
      HMID self;
      device.getDeviceID(self);
      uint8_t cnt = device.nextcount();
      msg.init(cnt,1,cnt,false,false);
      msg.to(self);
      msg.from(self);
      if( device.channel(1).process(msg) == false ) {
        DPRINTLN("No self peer - use toggleState");
        // no self peer - use old toggle code
        device.channel(1).toggleState();
      }
    }
    else if( s == ButtonType::longreleased ) {
      device.startPairing();
    }
    else if( s == ButtonType::longpressed ) {
      if( old == ButtonType::longpressed ) {
        device.reset(); // long pressed again - reset
      }
      else {
        device.led().set(LedStates::key_long);
      }
    }
  }
};

#define buttonISR(btn,pin) class btn##ISRHandler { \
  public: \
  static void isr () { btn.check(); } \
}; \
btn.init(pin); \
enableInterrupt(pin,btn##ISRHandler::isr,CHANGE);

}

#endif
