
#include "StatusLed.h"
#include "AlarmClock.h"

const StatusLed::BlinkPattern pattern [11] PROGMEM = {
    {0, 0, {0 , 0 } },  // 0; define nothing
    {2, 0, {5, 5,} },   // 1; define pairing string
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

void StatusLed::next (AlarmClock& clock) {
  tick = current.pattern[step++];
  ((step & 0x01) == 0x01) ? ledOn() : ledOff();
  clock.add(*this);
}

void StatusLed::trigger (AlarmClock& clock) {
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
