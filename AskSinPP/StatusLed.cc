
#include "StatusLed.h"
#include "AlarmClock.h"

void StatusLed::set(Mode stat) {
  aclock.cancel(*this);
  ledOff();
  if( stat != nothing ) {
    current = &pattern[stat];
    step = 0;
    repeat = 0;
    update(aclock);
  }
}

void StatusLed::update (AlarmClock& clock) {
  tick = current->pattern[step++];
  ((step & 0x01) == 0x01) ? ledOn() : ledOff();
  clock.add(*this);
}

void StatusLed::trigger (AlarmClock& clock) {
  if( step < current->length ) {
    update(clock);
  }
  else {
    step = 0;
    if( current->duration == 0 || ++repeat < current->duration ) {
      update(clock);
    }
    else {
      ledOff();
      current = &pattern[nothing];
    }
  }
};
