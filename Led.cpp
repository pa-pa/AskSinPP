
#include "AlarmClock.h"
#include "Led.h"

namespace as {


StatusLed sled;

const StatusLed::BlinkPattern pattern [11] PROGMEM = {
    {0, 0, {0 , 0 } },  // 0; define nothing
    {2, 20, {5, 5,} },  // 1; define pairing string
    {2, 1, {20, 0,} },  // 2; define pairing success
    {2, 3, {1, 1,} },   // 3; define pairing error
    {2, 1, {1, 1,} },   // 4; define send indicator
    {0, 0, {0, 0,} },   // 5; define ack indicator
    {0, 0, {0, 0,} },   // 6; define no ack indicator
    {6, 3, {5, 1, 1, 1, 1, 10} }, // 7; define battery low indicator
    {6, 3, {1, 1, 1, 1, 1, 10} }, // 8; define defect indicator
    {6, 1, {1, 1, 5, 1, 5, 10} }, // 9; define welcome indicator
    {2, 6, {2, 2, } },  // 10; key long indicator
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
