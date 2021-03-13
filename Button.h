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

class DoublePressAlarm : public Alarm {
private:
  bool isNewPressAllowed;
  uint16_t doublepresstime;
public:
  DoublePressAlarm () : Alarm(0), isNewPressAllowed(true), doublepresstime(0) {}
  virtual ~DoublePressAlarm () {}

  bool newPressAllowed() {
    return isNewPressAllowed;
  }

  void newPressAllowed(bool b) {
    isNewPressAllowed = b;
    if (b == false) {
      sysclock.cancel(*this);
      set(millis2ticks(doublepresstime));
      sysclock.add(*this);
    }
  }

  virtual void trigger(__attribute__((unused)) AlarmClock& clock) {
    isNewPressAllowed = true;
  }

  void setDoublePressTime(uint16_t t) {
    doublepresstime = t;
  }
};

class NoDoublePressAlarm {
public:
  NoDoublePressAlarm () {}
  ~NoDoublePressAlarm () {}
  bool newPressAllowed() { return true;  }
  void newPressAllowed(__attribute__((unused)) bool b) { }
  void setDoublePressTime(__attribute__((unused)) uint16_t t) {
  }
};

template <uint8_t OFFSTATE=HIGH,uint8_t ONSTATE=LOW,WiringPinMode MODE=INPUT_PULLUP, class DBLPRESS=NoDoublePressAlarm>
class StateButton: public Alarm {

#define DEBOUNCETIME millis2ticks(50)

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

  class CheckAlarm : public Alarm {
  public:
    StateButton& sb;
    CheckAlarm (StateButton& _sb) : Alarm(0), sb(_sb) {}
    ~CheckAlarm () {}
    virtual void trigger(__attribute__((unused)) AlarmClock& clock) {
      sb.check();
    }
  };

protected:
  uint8_t  stat     : 3;
  uint8_t  pinstate : 1;
  uint8_t  pin;
  uint16_t longpresstime;
  CheckAlarm ca;
  DBLPRESS dbl;

public:
  StateButton() :
      Alarm(0), stat(none), pinstate(OFFSTATE), pin(0), longpresstime(millis2ticks(400)), ca(*this)  {
  }
  virtual ~StateButton() {
  }

  void setLongPressTime(uint16_t t) {
    longpresstime = t;
  }

  void setDoublePressTime(uint16_t t) {
    dbl.setDoublePressTime(t);
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
      dbl.newPressAllowed(false);
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

  void irq () {
    if (dbl.newPressAllowed() == true) {
      sysclock.cancel(ca);
      // use alarm to run code outside of interrupt
      sysclock.add(ca);
    }
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
      default:
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
class ConfigButton : public StateButton<OFFSTATE,ONSTATE,MODE> {
  DEVTYPE& device;
public:
  typedef StateButton<OFFSTATE,ONSTATE,MODE> ButtonType;

  ConfigButton (DEVTYPE& dev,uint8_t longpresstime=3) : device(dev) {
    this->setLongPressTime(seconds2ticks(longpresstime));
  }
  virtual ~ConfigButton () {}
  virtual void state (uint8_t s) {
    uint8_t old = ButtonType::state();
    ButtonType::state(s);
    if( s == ButtonType::released ) {
      device.startPairing();
    }
    else if( s == ButtonType::longpressed ) {
      if( old == ButtonType::longpressed ) {
        if( device.getList0().localResetDisable() == false ) {
          device.reset(); // long pressed again - reset
        }
      }
      else {
        device.led().set(LedStates::key_long);
      }
    }
  }
};

template <class DEVTYPE,uint8_t OFFSTATE=HIGH,uint8_t ONSTATE=LOW,WiringPinMode MODE=INPUT_PULLUP>
class ConfigToggleButton : public StateButton<OFFSTATE,ONSTATE,MODE> {
  DEVTYPE& device;
public:
  typedef StateButton<OFFSTATE,ONSTATE,MODE> ButtonType;

  ConfigToggleButton (DEVTYPE& dev,uint8_t longpresstime=3) : device(dev) {
    this->setLongPressTime(seconds2ticks(longpresstime));
  }
  virtual ~ConfigToggleButton () {}
  virtual void state (uint8_t s) {
    uint8_t old = ButtonType::state();
    ButtonType::state(s);
    if( s == ButtonType::released ) {
      RemoteEventMsg& msg = (RemoteEventMsg&)device.message();
      uint8_t cnt = device.nextcount();
      msg.init(cnt,1,cnt,false,false);
      device.getDeviceID(msg.to());
      device.getDeviceID(msg.from());
      msg.clearAck();
      if( device.process(msg) == false ) {
        DPRINTLN(F("No self peer. Create internal peering to toggle state!"));
      }
    }
    else if( s == ButtonType::longreleased ) {
      device.startPairing();
    }
    else if( s == ButtonType::longpressed ) {
      if( old == ButtonType::longpressed ) {
        if( device.getList0().localResetDisable() == false ) {
          device.reset(); // long pressed again - reset
        }
      }
      else {
        device.led().set(LedStates::key_long);
      }
    }
  }
  Peer peer () const {
    HMID self;
    device.getDeviceID(self);
    return Peer(self,1);
  }
};

template <class DEVTYPE,uint8_t OFFSTATE=HIGH,uint8_t ONSTATE=LOW,WiringPinMode MODE=INPUT_PULLUP>
class InternalButton : public StateButton<OFFSTATE,ONSTATE,MODE> {
  DEVTYPE& device;
  uint8_t  num, counter;
public:
  typedef StateButton<OFFSTATE,ONSTATE,MODE> ButtonType;

  InternalButton (DEVTYPE& dev,uint8_t n,uint8_t longpresstime=4) : device(dev), num(n), counter(0) {
    this->setLongPressTime(decis2ticks(longpresstime));
  }
  virtual ~InternalButton () {}
  virtual void state (uint8_t s) {
    ButtonType::state(s);
    if( s == ButtonType::released ) {
      shortPress();
    }
    else if( s == ButtonType::longpressed ) {
      RemoteEventMsg& msg = fillMsg(true);
      device.process(msg);
    }
    else if( s == ButtonType::longreleased ) {
      counter++;
    }
  }
  RemoteEventMsg& fillMsg (bool lg) {
    RemoteEventMsg& msg = (RemoteEventMsg&)device.message();
    uint8_t cnt = device.nextcount();
    msg.init(cnt,num,counter,lg,false);
    device.getDeviceID(msg.to());
    device.getDeviceID(msg.from());
    msg.clearAck();
    return msg;
  }
  Peer peer () const {
    HMID self;
    device.getDeviceID(self);
    return Peer(self,num);
  }
  // trigger a short press event - press button by software
  void shortPress () {
    RemoteEventMsg& msg = fillMsg(false);
    device.process(msg);
    counter++;
  }
};

class BaseEncoder {
  int8_t  counter;
  uint8_t datapin;
public:
  BaseEncoder () : counter(0), datapin(0) {}
  void encirq () {
    uint8_t data = digitalRead(datapin);
    counter += data == LOW ? 1 : -1;
  }
  void init (uint8_t cpin,uint8_t dpin) {
    pinMode(cpin, INPUT_PULLUP);
    pinMode(dpin, INPUT_PULLUP);
    datapin = dpin;
  }
  int8_t read () {
    int8_t result=0;
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
    {
      result = counter;
      counter = 0;
    }
    return result;
  }
};

template<class DeviceType>
class InternalEncoder : public InternalButton<DeviceType>, public BaseEncoder {

public:
  InternalEncoder (DeviceType& dev,uint8_t num) : InternalButton<DeviceType>(dev,num), BaseEncoder() {};
  virtual ~InternalEncoder () {};

  void init (uint8_t sw) {
    InternalButton<DeviceType>::init(sw);
  }
  void init (uint8_t cpin,uint8_t dpin) {
    BaseEncoder::init(cpin,dpin);
  }
  template<class ChannelType>
  void process (ChannelType& channel) {
    int8_t dx = read();
    if( dx != 0 ) {
      typename ChannelType::List3 l3 = channel.getList3(this->peer());
      if( l3.valid() ) {
        if( dx > 0 ) channel.dimUp(l3.sh());
        else channel.dimDown(l3.sh());
      }
    }
  }
};

#define buttonISR(btn,pin) class btn##ISRHandler { \
  public: \
  static void isr () { btn.irq(); } \
}; \
btn.init(pin); \
if( digitalPinToInterrupt(pin) == NOT_AN_INTERRUPT ) \
  enableInterrupt(pin,btn##ISRHandler::isr,CHANGE); \
else \
  attachInterrupt(digitalPinToInterrupt(pin),btn##ISRHandler::isr,CHANGE);

#define encoderISR(enc,clkpin,datapin) class enc##ENCISRHandler { \
  public: \
  static void isr () { enc.encirq(); } \
}; \
enc.init(clkpin,datapin); \
if( digitalPinToInterrupt(clkpin) == NOT_AN_INTERRUPT ) \
  enableInterrupt(clkpin,enc##ENCISRHandler::isr,FALLING); \
else \
  attachInterrupt(digitalPinToInterrupt(clkpin),enc##ENCISRHandler::isr,FALLING);

}

#endif
