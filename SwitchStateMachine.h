
#ifndef __SWITCHSTATEMACHINE_H__
#define __SWITCHSTATEMACHINE_H__

#include "cm.h"
#include "SwitchList3.h"
#include "AlarmClock.h"

namespace as {

class SwitchStateMachine {

  class StateAlarm : public Alarm {
    SwitchStateMachine& sm;
    SwitchPeerList      lst;
    uint8_t             act;
  public:
    StateAlarm(SwitchStateMachine& m) : Alarm(0), sm(m), lst(0), act(AS_CM_JT_NONE) {}
    void list(SwitchPeerList l) {lst=l;}
    void action (uint8_t a) {act=a;}
    virtual void trigger (AlarmClock& clock) {
      if( act != AS_CM_JT_NONE ) {
        sm.setState(act,0xffff);
      }
      else {
        sm.jumpToTarget(lst);
      }
    }
  };


  uint8_t    state : 4;
  StateAlarm alarm;

  void setState (uint8_t state,uint16_t duration);

protected:
  ~SwitchStateMachine () {}

public:
  SwitchStateMachine() : state(AS_CM_JT_OFF), alarm(*this) {}

  virtual void switchState(uint8_t oldstate,uint8_t newstate);

  void jumpToTarget(SwitchPeerList lst);

  void toggleState () {
    setState( state == AS_CM_JT_ON ? AS_CM_JT_OFF : AS_CM_JT_ON, 0xffff);
  }

  uint8_t getNextState(uint8_t state,const SwitchPeerList& lst) const {
    switch( state ) {
      case AS_CM_JT_ONDELAY:  return lst.jtDlyOn();
      case AS_CM_JT_ON:       return lst.jtOn();
      case AS_CM_JT_OFFDELAY: return lst.jtDlyOff();
      case AS_CM_JT_OFF:      return lst.jtOff();
    }
    return AS_CM_JT_NONE;
  }

  uint8_t getDelayForState(uint8_t state,const SwitchPeerList& lst) const {
    switch( state ) {
      case AS_CM_JT_ONDELAY:  return lst.onDly();
      case AS_CM_JT_ON:       return lst.onTime();
      case AS_CM_JT_OFFDELAY: return lst.offDly();
      case AS_CM_JT_OFF:      return lst.offTime();
      default: break;
    }
    return 0;
  }

  bool delayActive () const { return aclock.get(alarm) > 0; }

  // get timer count in 1/10s
  uint32_t byteTimeCvt(uint8_t tTime) {
    const uint16_t c[8] = {1,10,50,100,600,3000,6000,36000};
    return (uint16_t)(tTime & 0x1F) * c[tTime >> 5];
  }

  // get timer count in 1/10s
  uint32_t intTimeCvt(uint16_t iTime) {
    if (iTime == 0) return 0;

    uint8_t tByte;
    if ((iTime & 0x1F) != 0) {
      tByte = 2;
      for (uint8_t i = 1; i < (iTime & 0x1F); i++) tByte *= 2;
    } else tByte = 1;

    return (uint32_t)tByte*(iTime>>5);
  }

  void remote (const SwitchPeerList& lst,uint8_t counter) {
    // perform action as defined in the list
    switch (lst.actionType()) {
    case AS_CM_ACTIONTYPE_JUMP_TO_TARGET:
      jumpToTarget(lst);
      break;
    case AS_CM_ACTIONTYPE_TOGGLE_TO_COUNTER:
      setState((counter & 0x01) == 0x01 ? AS_CM_JT_ON : AS_CM_JT_OFF, 0xffff);
      break;
    case AS_CM_ACTIONTYPE_TOGGLE_INVERSE_TO_COUNTER:
      setState((counter & 0x01) == 0x00 ? AS_CM_JT_ON : AS_CM_JT_OFF, 0xffff);
      break;
    }
  }

  void status (uint8_t stat, uint16_t delay) {
    setState( stat == 0 ? AS_CM_JT_OFF : AS_CM_JT_ON, delay );
  }

  uint8_t status () const {
    return state == AS_CM_JT_OFF ? 0 : 200;
  }

  uint8_t flags () const {
    return delayActive() ? 0x40 : 0x00;
  }
};

}

#endif
