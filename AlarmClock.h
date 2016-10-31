//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ALARMCLOCK_H__
#define __ALARMCLOCK_H__

#include "Alarm.h"

namespace as {

#define TICKS_PER_SECOND 10

#define seconds2ticks(tm) ( tm * TICKS_PER_SECOND )
#define ticks2seconds(tm) ( tm / TICKS_PER_SECOND )

//#define decis2ticks(tm) ( tm * TICKS_PER_SECOND / 10 )
#define decis2ticks(tm) ( tm )
#define ticks2decis(tm) ( tm )

//#define centis2ticks(tm)  ( tm * TICKS_PER_SECOND / 100 )
#define centis2ticks(tm)  ( tm / 10 )
#define ticks2centis(tm)  ( tm * 10 )

//#define millis2ticks(tm) ( tm * TICKS_PER_SECOND / 1000 )
#define millis2ticks(tm) ( tm / 100 )
#define ticks2millis(tm) ( tm * 100 )

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
