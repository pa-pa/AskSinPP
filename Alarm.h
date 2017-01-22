//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ALARM_H__
#define __ALARM_H__

#include "Link.h"

namespace as {

class AlarmClock;

class Alarm: public Link {
protected:
  ~Alarm() {
  }
public:
  bool asyn;
  uint32_t tick;

  virtual void trigger(AlarmClock&) = 0;

  Alarm(uint32_t t) :
      asyn(false), tick(t) {
  }
  void set(uint32_t t) {
    tick = t;
  }
  void async(bool value) {
    asyn = value;
  }
  bool async() const {
    return asyn;
  }
};

}

#endif
