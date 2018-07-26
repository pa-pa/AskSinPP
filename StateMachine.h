//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-12-23 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "Alarm.h"

namespace as {

template <class PeerList>
class StateMachine : public Alarm {

  class ChangedAlarm : public Alarm {
    StateMachine<PeerList>&  sm;
  public:
    ChangedAlarm (StateMachine<PeerList>& s) : Alarm(0), sm(s) {}
    virtual ~ChangedAlarm () {}
    void set (uint32_t t,AlarmClock& clock) {
      clock.cancel(*this);
      Alarm::set(t);
      clock.add(*this);
    }
    virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
      sm.changed = true;
    }
  };

protected:
  enum { DELAY_NO=0x00, DELAY_INFINITE=0xffffffff };

  uint8_t       state;
  bool          changed;
  ChangedAlarm  calarm;
  PeerList      actlst;

  StateMachine () : Alarm(0), state(AS_CM_JT_NONE), changed(false), calarm(*this), actlst(0) {}
  virtual ~StateMachine () {}

  virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
    uint8_t next = getNextState(state);
    uint32_t dly = getDelayForState(next,actlst);
    setState(next,dly,actlst);
  }

  void setState (uint8_t next,uint32_t delay,const PeerList& lst=PeerList(0)) {
    if( next != AS_CM_JT_NONE ) {
      // first cancel possible running alarm
      sysclock.cancel(*this);
      // if state is different
      while (state != next) {
        switchState(state, next, delay);
        state = next;

        if (delay == DELAY_NO) {
          // go immediately to the next state
          next = getNextState(state);
          delay = getDelayForState(next,lst);
        }
      }
      if (delay != DELAY_INFINITE) {
        actlst = lst;
        set(delay);
        sysclock.add(*this);
      }
    }
  }

  virtual void switchState(__attribute__((unused)) uint8_t oldstate,__attribute__((unused)) uint8_t newstate, __attribute__((unused)) uint32_t) {}

  void jumpToTarget(const PeerList& lst) {
    uint8_t next = getJumpTarget(state,lst);
    if( next != AS_CM_JT_NONE ) {
      // get delay
      uint32_t dly = getDelayForState(next,lst);
      // switch to next
      setState(next,dly,lst);
    }
  }

  virtual uint8_t getNextState (uint8_t stat) {
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  return AS_CM_JT_REFON;
      case AS_CM_JT_REFON:    return AS_CM_JT_RAMPON;
      case AS_CM_JT_RAMPON:   return AS_CM_JT_ON;
      case AS_CM_JT_ON:       return AS_CM_JT_OFFDELAY;
      case AS_CM_JT_OFFDELAY: return AS_CM_JT_REFOFF;
      case AS_CM_JT_REFOFF:   return AS_CM_JT_RAMPOFF;
      case AS_CM_JT_RAMPOFF:  return AS_CM_JT_OFF;
      case AS_CM_JT_OFF:      return AS_CM_JT_ONDELAY;
    }
    return AS_CM_JT_NONE;
  }

  virtual uint32_t getDelayForState(uint8_t stat,const PeerList& lst) {
    uint32_t delay = getDefaultDelay(stat);
    if( lst.valid() == true ) {
      uint8_t value = 0;
      switch( stat ) {
        case AS_CM_JT_ONDELAY:  value = lst.onDly(); break;
        case AS_CM_JT_ON:       value = lst.onTime(); break;
        case AS_CM_JT_OFFDELAY: value = lst.offDly(); break;
        case AS_CM_JT_OFF:      value = lst.offTime(); break;
        default:                return delay; break;
      }
      delay = AskSinBase::byteTimeCvt(value);
    }
    return delay;
  }

  virtual uint32_t getDefaultDelay(uint8_t stat) const {
    switch( stat ) {
      case AS_CM_JT_ON:
      case AS_CM_JT_OFF:
        return DELAY_INFINITE;
    }
    return DELAY_NO;
  }

  bool delayActive () const { return sysclock.get(*this) > 0; }

  void triggerChanged (uint32_t delay) {
    calarm.set(delay,sysclock);
  }

  uint8_t getJumpTarget(uint8_t stat,const PeerList& lst) const {
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  return lst.jtDlyOn();
      case AS_CM_JT_REFON:    return lst.jtRefOn();
      case AS_CM_JT_RAMPON:   return lst.jtRampOn();
      case AS_CM_JT_ON:       return lst.jtOn();
      case AS_CM_JT_OFFDELAY: return lst.jtDlyOff();
      case AS_CM_JT_REFOFF:   return lst.jtRefOff();
      case AS_CM_JT_RAMPOFF:  return lst.jtRampOff();
      case AS_CM_JT_OFF:      return lst.jtOff();
    }
    return AS_CM_JT_NONE;
  }

  uint8_t getConditionForState(uint8_t stat,const PeerList& lst) const {
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  return lst.ctDlyOn();
      case AS_CM_JT_REFON:    return lst.ctRepOn();
      case AS_CM_JT_RAMPON:   return lst.ctRampOn();
      case AS_CM_JT_ON:       return lst.ctOn();
      case AS_CM_JT_OFFDELAY: return lst.ctDlyOff();
      case AS_CM_JT_REFOFF:   return lst.ctRepOff();
      case AS_CM_JT_RAMPOFF:  return lst.ctRampOff();
      case AS_CM_JT_OFF:      return lst.ctOff();
    }
    return AS_CM_CT_X_GE_COND_VALUE_LO;
  }

  bool checkCondition (uint8_t stat,const PeerList& lst,uint8_t value) {
    uint8_t cond = getConditionForState(stat,lst);
    bool doit = false;
    switch( cond ) {
    case AS_CM_CT_X_GE_COND_VALUE_LO:
      doit = (value >= lst.ctValLo());
      break;
    case AS_CM_CT_X_GE_COND_VALUE_HI:
      doit = (value >= lst.ctValHi());
      break;
    case AS_CM_CT_X_LT_COND_VALUE_LO:
      doit = (value < lst.ctValLo());
      break;
    case AS_CM_CT_X_LT_COND_VALUE_HI:
      doit = (value < lst.ctValHi());
      break;
    case AS_CM_CT_COND_VALUE_LO_LE_X_LT_COND_VALUE_HI:
      doit = ((lst.ctValLo() <= value) && (value < lst.ctValHi()));
      break;
    case AS_CM_CT_X_LT_COND_VALUE_LO_OR_X_GE_COND_VALUE_HI:
      doit = ((value < lst.ctValLo()) || (value >= lst.ctValHi()));
      break;
    }
    return doit;
  }

};

}

#endif
