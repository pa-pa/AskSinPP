//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-03-20 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __PUSHBUTTON_H__
#define __PUSHBUTTON_H__

#include <AlarmClock.h>

namespace as {

template <uint8_t PRESSEDSTATE>
class PushButton : public Alarm {
private:
  uint8_t pin;
public:
  PushButton () : Alarm(0), pin(0) { async(true); }
  virtual ~PushButton () {}

  virtual void trigger (AlarmClock& clock) {
    digitalWrite(pin,PRESSEDSTATE==HIGH ? LOW : HIGH);
  }

  void init (uint8_t p) {
    pin = p;
    pinMode(p,OUTPUT);
    digitalWrite(pin,PRESSEDSTATE==HIGH ? LOW : HIGH);
  }

  void press (uint16_t millis) {
    digitalWrite(pin,PRESSEDSTATE==HIGH ? HIGH : LOW);
    sysclock.cancel(*this);
    set(millis2ticks(millis));
    sysclock.add(*this);
  }
};

}

#endif
