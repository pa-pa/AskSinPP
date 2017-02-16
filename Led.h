//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __STATUSLED_H__
#define __STATUSLED_H__

#include "Alarm.h"
#include "Debug.h"

namespace as {


class StatusLed {

public:

  enum Mode { nothing=0, pairing=1,send=2, ack=3,
    nack=4, bat_low=5, welcome=6, key_long=7 };

  struct BlinkPattern {
    uint8_t  length;
    uint8_t  duration;
    uint8_t  pattern[6];
  };

protected:

  class Led : public Alarm {
private:
    uint8_t pin;
    BlinkPattern current;
    uint8_t step;   // current step in pattern
    uint8_t repeat; // current repeat of the pattern

    void copyPattern (Mode stat,const BlinkPattern* patt);
    void next (AlarmClock& clock);

public:
    Led () : Alarm(0), pin(0), step(0), repeat(0) {
      async(true);
    }
    virtual ~Led() {}

    void init (uint8_t p) {
      pin = p;
      pinMode(pin,OUTPUT);
      ledOff();
    }

    void set(Mode stat,const BlinkPattern* patt);

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

  Led led1;

public:
  StatusLed () {}

  void init (uint8_t p1) {  led1.init(p1); }
  bool active () const { return led1.active(); }
  void ledOn (uint8_t ticks) { led1.ledOn(ticks); }
  void set(Mode stat);
};

class DualStatusLed : public StatusLed {
private:
  Led led2;
public:
  DualStatusLed () {}
  void init (uint8_t p1, uint8_t p2) { led1.init(p1); led2.init(p2); }
  bool active () const { return led1.active() || led2.active(); }
  void ledOn (uint8_t ticks) { led1.ledOn(ticks); led2.ledOn(ticks); }
  void set(Mode stat);
};

}

#endif
