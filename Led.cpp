//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#include "AlarmClock.h"
#include "Led.h"

namespace as {


StatusLed sled;


const StatusLed::BlinkPattern pattern [8] PROGMEM = {
    {0, 0, {0 , 0 } },  // 0; define nothing
    {2, 20, {5, 5,} },  // 1; define pairing string
    {2, 1, {10, 1,} },   // 2; define send indicator
    {0, 0, {0, 0,} },   // 3; define ack indicator
    {0, 0, {0, 0,} },   // 4; define no ack indicator
    {6, 3, {5, 1, 1, 1, 1, 10} }, // 5; define battery low indicator
    {6, 1, {1, 1, 5, 1, 5, 10} }, // 6; define welcome indicator
    {2, 6, {2, 2, } },  // 7; key long indicator
};

void StatusLed::copyPattern (Mode stat) {
  memcpy_P(&current,&pattern[stat],sizeof(BlinkPattern));
}

void StatusLed::set(Mode stat) {
  aclock.cancel(*this);
  ledOff();
  copyPattern(stat);
  if( current.length > 0 ) {
    step = 0;
    repeat = 0;
    next(aclock);
  }
}

void StatusLed::ledOn (uint8_t ticks) {
  if( active() == false && ticks > 0 ) {
    current.length = 2;
    current.duration = 1;
    current.pattern[0] = ticks2decis(ticks);
    current.pattern[1] = 0;
    // start the pattern
    step = repeat = 0;
    next(aclock);
  }
}

void StatusLed::next (AlarmClock& clock) {
  tick = decis2ticks(current.pattern[step++]);
  ((step & 0x01) == 0x01) ? ledOn() : ledOff();
  clock.add(*this);
}

void StatusLed::trigger (AlarmClock& clock) {
  ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
    if( step < current.length ) {
      next(clock);
    }
    else {
      step = 0;
      if( current.duration == 0 || ++repeat < current.duration ) {
        next(clock);
      }
      else {
        ledOff();
        copyPattern(nothing);
      }
    }
  }
}

}
