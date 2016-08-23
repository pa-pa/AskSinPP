
#ifdef ARDUINO
  #ifndef __INC_TIMERONE_H__
  #define __INC_TIMERONE_H__
    #include <TimerOne.h>
  #endif
#endif

#include "AlarmClock.h"

AlarmClock aclock;

void AlarmClock::init() {
#ifdef ARDUINO
    Timer1.initialize(100000);         // initialize timer1, and set a 1/10 second period
    Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt
#endif
}

void AlarmClock::cancel(Alarm& item) {
  ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
    Alarm *tmp = (Alarm*) select();
    Link *vor = this;
    // search for the alarm to cancel
    while (tmp != 0) {
      if (tmp == &item) {
        vor->unlink();
        // raise next alarm about item ticks
        tmp = (Alarm*) item.select();
        if (tmp != 0) {
          tmp->tick += item.tick;
        }
        return;
      }
      vor = tmp;
      tmp = (Alarm*) tmp->select();
    }
  }
}

AlarmClock& AlarmClock::operator --() {
  Alarm* alarm = (Alarm*) select();
  if (alarm != 0) {
    --alarm->tick;
    while ((alarm != 0) && (alarm->tick == 0)) {
      unlink(); // remove expired alarm
      ready.append(*alarm);
      alarm = (Alarm*) select();
    }
  }
  return *this;
}

void AlarmClock::add(Alarm& item) {
  if (item.tick > 0) {
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
      Link* prev = this;
      Alarm* temp = (Alarm*) select();
      while ((temp != 0) && (temp->tick < item.tick)) {
        item.tick -= temp->tick;
        prev = temp;
        temp = (Alarm*) temp->select();
      }
      item.select(temp);
      prev->select(&item);
      if (temp != 0) {
        temp->tick -= item.tick;
      }
    }
  } else {
    ready.append(item);
  }
}

uint16_t AlarmClock::get(Alarm& item) const {
  uint16_t aux = 0;
  Alarm* tmp = (Alarm*) select();
  while (tmp != 0) {
    aux += tmp->tick;
    if (tmp == &item) {
      return aux;
    }
    tmp = (Alarm*) tmp->select();
  }
  return 0;
}

inline void callback(void) {
  --aclock;
}
