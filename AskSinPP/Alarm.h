
#ifndef __ALARM_H__
#define __ALARM_H__

#include "Link.h"

class AlarmClock;

class Alarm : public Link {
protected:
  ~Alarm () {}
public:
  uint32_t tick;

  virtual void trigger (AlarmClock&) = 0;

  Alarm (uint32_t t) : tick(t) {}
  void set(uint32_t t) { tick=t; }
};

#endif
