//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "AlarmClock.h"
#include "Debug.h"

#if ARDUINO_ARCH_AVR or ARDUINO_ARCH_ATMEGA32
  typedef uint8_t WiringPinMode;
#endif

namespace as {

template <uint8_t OFFSTATE=HIGH,uint8_t ONSTATE=LOW,WiringPinMode MODE=INPUT_PULLUP>
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

public:
  StateButton() :
      Alarm(0), stat(none), pinstate(OFFSTATE), pin(0), longpresstime(millis2ticks(400)), ca(*this)  {
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

  void irq () {
    sysclock.cancel(ca);
    // use alarm to run code outside of interrupt
    sysclock.add(ca);
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
        device.reset(); // long pressed again - reset
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
      HMID self;
      device.getDeviceID(self);
      uint8_t cnt = device.nextcount();
      msg.init(cnt,1,cnt,false,false);
      msg.to(self);
      msg.from(self);
      if( device.process(msg) == false ) {
        DPRINTLN(F("No self peer. Create internal peering to toggle state!"));
        // no self peer - use old toggle code
        // device.channel(1).toggleState();
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
      RemoteEventMsg& msg = fillMsg(false);
      device.process(msg);
      counter++;
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
    HMID self;
    device.getDeviceID(self);
    uint8_t cnt = device.nextcount();
    msg.init(cnt,num,counter,lg,false);
    msg.to(self);
    msg.from(self);
    return msg;
  }
  Peer peer () const {
    HMID self;
    device.getDeviceID(self);
    return Peer(self,num);
  }
};

template<class DeviceType>
class Encoder : public InternalButton<DeviceType> {

  class EncAlarm : public Alarm {
    Encoder& enc;
  public:
    EncAlarm (Encoder& e) : Alarm(0), enc(e) {
      async(true);
    }
    virtual void trigger (AlarmClock& clock) {
      enc.checkPins();
      start(clock);
    }
    void start (AlarmClock& clock) {
      set(millis2ticks(5));
      clock.add(*this);
    }
    void stop (AlarmClock& clock) {
      clock.cancel(*this);
    }
  };

  static int8_t table(int index) {
    // https://www.mikrocontroller.net/articles/Drehgeber
    const int8_t encoder_table[16] PROGMEM = {0,0,-1,0,0,0,0,1,1,0,0,0,0,-1,0,0};
    return pgm_read_byte(&encoder_table[index]);
  }

  int8_t last;
  volatile int8_t delta;
  uint8_t clkpin, dtpin;
  EncAlarm alarm;

public:
  Encoder (DeviceType& dev,uint8_t num) : InternalButton<DeviceType>(dev,num), last(0), delta(0), clkpin(0), dtpin(0), alarm(*this) {};
  virtual ~Encoder () {};

  bool checkPins () {
    int8_t ll=0;
    if (digitalRead(clkpin)) ll |=2;
    if (digitalRead(dtpin))  ll |=1;
    last = ((last << 2)  & 0x0F) | ll;
    delta += table(last);
    return delta != 0;
  }

  void init (uint8_t sw) {
    InternalButton<DeviceType>::init(sw);
  }

  void init (uint8_t clk,uint8_t dt) {
    clkpin = clk;
    dtpin = dt;
    pinMode(clkpin, INPUT);
    pinMode(dtpin, INPUT);
    alarm.start(sysclock);
  }

  int8_t read () {
    int8_t val=0;
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
    {
      val = delta;
      delta = val & 1;
    }
    return val >> 1;
  }

  template<class ChannelType>
  void process (ChannelType& channel) {
    int8_t dx = read();
    if( dx != 0 && channel.status() != 0 ) {
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

}

#endif
