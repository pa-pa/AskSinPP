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

  enum { ASYNC=0x01, ACTIVE=0x02 };

protected:
  ~Alarm() {}

  uint8_t flags;

  void setflag(bool c,uint8_t mask) { c ? flags |= mask : flags &= ~mask; }
  void setflag(uint8_t mask) { flags |= mask; }
  void remflag(uint8_t mask) { flags &= ~mask; }
  bool hasflag(uint8_t mask) const { return (flags & mask) == mask; }

public:
  uint32_t tick : 24;

  virtual void trigger(AlarmClock&) = 0;

  Alarm () : flags(0), tick(0) {}
  Alarm(uint32_t t) : flags(0), tick(t) {}
  Alarm(uint32_t t,bool asynch) : flags(0), tick(t) {
    async(asynch);
  }
  void set(uint32_t t) {
    tick = t;
  }
  void async(bool value) {
    setflag(value, ASYNC);
  }
  bool async() const {
    return hasflag(ASYNC);
  }
  void active(bool value) {
    setflag(value,ACTIVE);
  }
  bool active () const {
    return hasflag(ACTIVE);
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
