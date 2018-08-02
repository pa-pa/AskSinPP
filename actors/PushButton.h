
#ifndef __PUSHBUTTON_H__
#define __PUSHBUTTON_H__

#include <AlarmClock.h>

namespace as {

class PushButton : public Alarm {
private:
  uint8_t pin;
public:
  PushButton () : Alarm(0) { async(true); }
  virtual ~PushButton () {}

  virtual void trigger (AlarmClock& clock) {
    digitalWrite(pin,LOW);
  }

  void init (uint8_t p) { pin = p }

  void press (uint16_t millis) {
    digitalWrite(pin,HIGH);
    sysclock.cancel(*this);
    set(millis2ticks(millis));
    sysclock.add(*this);
  }
};

}

#endif
