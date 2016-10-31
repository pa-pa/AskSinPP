//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __STATUSLED_H__
#define __STATUSLED_H__

#include "Alarm.h"

namespace as {

class StatusLed : public Alarm {

public:

  enum Mode { nothing=0, pairing=1, pair_suc=2, pair_err=3, send=4, ack=5,
    noack=6, bat_low=7, defect=8, welcome=9, key_long=10 };

  struct BlinkPattern {
    uint8_t  length;
    uint8_t  duration;
    uint8_t  pattern[6];
  };

private:

  uint8_t pin;
  BlinkPattern current;
  uint8_t step;   // current step in pattern
  uint8_t repeat; // current repeat of the pattern

  void copyPattern (Mode stat);

  void next (AlarmClock& clock);

public:
  StatusLed () : Alarm(0), pin(0), step(0), repeat(0) {
    async(true);
  }
  virtual ~StatusLed () {}

  void init (uint8_t p) {
    pin = p;
    pinMode(pin,OUTPUT);
    ledOff();
  }

  void set(Mode stat);

  void ledOff () {
    digitalWrite(pin,LOW);
  }

  void ledOn () {
    digitalWrite(pin,HIGH);
  }

  void ledOn (uint8_t ticks);

  bool active () const { return current.length != 0; }

  virtual void trigger (AlarmClock& clock);
};

extern StatusLed sled;

}

#endif
