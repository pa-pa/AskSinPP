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

  bool isready () const {
    return ready.select() != 0;
  }

  bool runready() {
    bool worked = false;
    Alarm* a;
    while ((a = (Alarm*) ready.unlink()) != 0) {
      a->trigger(*this);
      worked = true;
    }
    return worked;
  }

  void add(Alarm& item);

  uint32_t get(const Alarm& item) const;

  uint32_t next () const {
    Alarm* n = (Alarm*)select();
    return n != 0 ? n->tick : 0;
  }

  // correct the alarms after sleep
  void correct (uint32_t ticks) {
    ticks--;
    Alarm* n = (Alarm*)select();
    if( n != 0 ) {
      uint32_t nextticks = n->tick-1;
      n->tick -= min(nextticks,ticks);
    }
    --(*this);
  }

  void disable ();

  void enable ();

};

extern AlarmClock aclock;

}

#endif
