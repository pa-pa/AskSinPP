//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#include "AlarmClock.h"

namespace as {

SysClock sysclock;

void callback(void) {
  --sysclock;
}

#ifndef NORTC
RTC rtc;
void rtccallback () {
  //  DPRINT(".");
    rtc.overflow();
  //  rtc.debug();
    --rtc;
}
#endif

#if ARDUINO_ARCH_AVR
ISR(TIMER1_OVF_vect) {
  callback();
}
#ifndef NORTC
ISR(TIMER2_OVF_vect) {
  rtccallback();
}
#endif
#endif


void AlarmClock::cancel(Alarm& item) {
  ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
  {
    item.active(false);
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
          alarm->active(false);
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
      item.active(true);
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

bool RTCAlarm::delayMillis () {
  if( millis != 0 ) {
    set(millis2ticks(millis));
    sysclock.add(*this);
    millis = 0;
    return true;
  }
  return false;
}

SysClock& SysClock::instance() { return sysclock; }

#ifndef NORTC
RTC& RTC::instance() { return rtc; }
#endif

}
