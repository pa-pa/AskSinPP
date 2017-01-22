//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#include "TimerOne.h"
#include "AlarmClock.h"

namespace as {

AlarmClock aclock;

void callback(void) {
  --aclock;
}

void AlarmClock::init() {
  Timer1.initialize(1000000 / TICKS_PER_SECOND); // initialize timer1, and set a 1/10 second period
  //Timer1.attachInterrupt(callback); // attaches callback() as a timer overflow interrupt
  enable();
}

void AlarmClock::disable () {
  Timer1.detachInterrupt();
}

void AlarmClock::enable () {
  Timer1.attachInterrupt(callback);
}

void AlarmClock::cancel(Alarm& item) {
  ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
  {
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
    // cancel also in ready queue
    ready.remove(item);
  }
}

AlarmClock& AlarmClock::operator --() {
  ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
  {
    Alarm* alarm = (Alarm*) select();
    if (alarm != 0) {
      --alarm->tick;
      while ((alarm != 0) && (alarm->tick == 0)) {
        unlink(); // remove expired alarm
        // run in interrupt
        if (alarm->async() == true) {
          alarm->trigger(*this);
        }
        // run in application
        else {
          ready.append(*alarm);
        }
        alarm = (Alarm*) select();
      }
    }
  }
  return *this;
}

void AlarmClock::add(Alarm& item) {
  if (item.tick > 0) {
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
    {
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

uint32_t AlarmClock::get(const Alarm& item) const {
  uint32_t aux = 0;
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

}
