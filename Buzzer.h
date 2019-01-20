//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-01-20 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "Alarm.h"
#include "Debug.h"

namespace as {

template<uint8_t PIN, class PINTYPE=ArduinoPins>
class Buzzer : public Alarm {
  bool enable;
  int8_t repeat;
  uint16_t ontime, offtime;
public:
  Buzzer () : Alarm(0), enable(false), repeat(0), ontime(0), offtime(0) {
    async(true);
  }
  virtual ~Buzzer () {}

  void init () {
    PINTYPE::setOutput(PIN);
  }

  void enabled(bool value) {
    enable = value;
  }

  bool on (uint16_t onticks,uint16_t offticks,int8_t repeat) {
    if( on() == true ) {
      ontime=onticks;
      offtime=offticks;
      this->repeat=repeat;
      if( ontime > 0 ) {
        set(ontime);
        sysclock.add(*this);
      }
      return true;
    }
    return false;
  }

  bool on (uint16_t ticks) {
    return on(ticks,0,1) ;
  }

  bool on () {
    if( enable == true ) {
      sysclock.cancel(*this);
      PINTYPE::setHigh(PIN);
      return true;
    }
    return false;
  }

  bool off (bool force) {
    if ( force == true ) {
      repeat = 0;
      ontime = 0;
    }
    PINTYPE::setLow(PIN);
    return true;
  }

  bool off () {
    return off(false);
  }

  bool active () {
    return PINTYPE::getState(PIN) == HIGH;
  }

  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    if( active() ) {
      off();
      if (repeat != -1) repeat--;
      if( (repeat != 0) && ontime > 0 ) {
        set(offtime);
        clock.add(*this);
      }
    }
    else if( (repeat != 0) && ontime > 0 ) {
      on();
      set(ontime);
      clock.add(*this);
    }
  }
};

class NoBuzzer {
public:
  NoBuzzer () {}
  ~NoBuzzer () {}
  void init () {}
  void enabled(__attribute__ ((unused)) bool value) {}
  bool on (__attribute__ ((unused))uint16_t onticks,__attribute__ ((unused))uint16_t offticks,__attribute__ ((unused))uint8_t repeat) { return false; }
  bool on (__attribute__ ((unused)) uint16_t ticks) { return false; }
  bool on () { return false; }
  void off () {}
  bool active () { return false; }
};

}

#endif
