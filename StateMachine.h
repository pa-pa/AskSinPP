//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-12-23 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

namespace as {

template <class PeerList>
class StateMachine {

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

  bool          changed;
  ChangedAlarm  calarm;

  StateMachine () : changed(false), calarm(*this) {}

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
