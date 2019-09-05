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
  ~Alarm() {}

  bool m_Async : 4;
  bool m_Active : 4;
public:
  uint32_t tick : 24;

  virtual void trigger(AlarmClock&) = 0;

  Alarm () :
    m_Async(false), m_Active(0), tick(0) {
}
  Alarm(uint32_t t) :
      m_Async(false), m_Active(0), tick(t) {
  }
  Alarm(uint32_t t,bool asynch) :
      m_Async(asynch), m_Active(0), tick(t) {
  }
  void set(uint32_t t) {
    tick = t;
  }
  void async(bool value) {
    m_Async = value;
  }
  bool async() const {
    return m_Async;
  }
  void active(bool value) {
    m_Active = value;
  }
  bool active () const {
    return m_Active;
  }
};

class RTCAlarm : public Alarm {
public:
  uint16_t millis;
protected:
  ~RTCAlarm() {}

  bool delayMillis ();

public:
  RTCAlarm() : Alarm(0), millis(0) {}
  RTCAlarm(uint32_t t,uint16_t m) : Alarm(t), millis(m) {}
  RTCAlarm(uint32_t t,uint16_t m,bool asynch) : Alarm(t,asynch), millis(m) {}
};

}

#endif
