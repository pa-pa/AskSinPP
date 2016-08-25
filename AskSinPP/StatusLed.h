
#ifndef __STATUSLED_H__
#define __STATUSLED_H__

#include "Alarm.h"

class StatusLed : public Alarm {

public:

  enum Mode { nothing=0, pairing=1, pair_suc=2, pair_err=3, send=4, ack=5,
    noack=6, bat_low=7, defect=8, welcome=9, key_long=10 };

private:

  struct BlinkPattern {
    uint8_t  length : 3;
    uint8_t  duration : 3;
    uint8_t  pattern[6];
  };

  uint8_t pin;
  const BlinkPattern* current;
  uint8_t step : 3; // current step in pattern
  uint8_t repeat : 3; // current repeat of the pattern

  const struct BlinkPattern pattern [11] = {
      {0, 0, {0 , 0 } },  // 0; define nothing
      {2, 0, {5, 5,} }, // 1; define pairing string
      {2, 1, {20, 0,} }, // 2; define pairing success
      {2, 3, {1, 1,} },  // 3; define pairing error
      {2, 1, {1, 1,} },   // 4; define send indicator
      {0, 0, {0, 0,} },   // 5; define ack indicator
      {0, 0, {0, 0,} },   // 6; define no ack indicator
      {6, 3, {5, 1, 1, 1, 1, 10} }, // 7; define battery low indicator
      {6, 3, {1, 1, 1, 1, 1, 10} }, // 8; define defect indicator
      {6, 1, {1, 1, 5, 1, 5, 10} }, // 9; define welcome indicator
      {2, 6, {2, 2, } }, // 10; key long indicator
  };

public:
  StatusLed (uint8_t p) : Alarm(0), pin(p), current(0), step(0), repeat(0) {
    pinMode(pin,OUTPUT);
    ledOff();
  }
  virtual ~StatusLed () {}

  void set(Mode stat);

  void update (AlarmClock& clock);

  void ledOff () {
    digitalWrite(pin,LOW);
  }

  void ledOn () {
    digitalWrite(pin,HIGH);
  }

  virtual void trigger (AlarmClock& clock);
};



#endif
