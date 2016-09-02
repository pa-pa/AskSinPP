#ifndef __ALARMCLOCK_H__
#define __ALARMCLOCK_H__

#include "Alarm.h"

namespace as {


class AlarmClock: protected Link {

  Link ready;

public:

  AlarmClock() {
  }

  void init();

  void cancel(Alarm& item);

  AlarmClock& operator --();

  void runready() {
    Alarm* a;
    while ((a = (Alarm*) ready.unlink()) != 0) {
      a->trigger(*this);
    }
  }

  void add(Alarm& item);

  uint32_t get(const Alarm& item) const;

};

extern AlarmClock aclock;

}

#endif
