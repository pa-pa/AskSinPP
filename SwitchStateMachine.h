
#ifndef __SWITCHSTATEMACHINE_H__
#define __SWITCHSTATEMACHINE_H__

#include "cm.h"
#include "SwitchList3.h"
#include "AlarmClock.h"

namespace as {

class SwitchStateMachine {

#define DELAY_NO 0x00
#define DELAY_INFINITE 0xffffffff

  class StateAlarm : public Alarm {
    SwitchStateMachine& sm;
    SwitchPeerList      lst;
  public:
    StateAlarm(SwitchStateMachine& m) : Alarm(0), sm(m), lst(0) {}
    void list(SwitchPeerList l) {lst=l;}
    virtual void trigger (AlarmClock& clock) {
      uint8_t next = sm.getNextState();
      uint32_t dly = sm.getDelayForState(next,lst);
      sm.setState(next,dly,lst);
    }
  };


  uint8_t    state;
  StateAlarm alarm;

  void setState (uint8_t state,uint32_t duration,const SwitchPeerList& lst=SwitchPeerList(0),uint8_t deep=0);

protected:
  ~SwitchStateMachine () {}

public:
  SwitchStateMachine() : state(AS_CM_JT_OFF), alarm(*this) {}

  virtual void switchState(uint8_t oldstate,uint8_t newstate);

  void jumpToTarget(const SwitchPeerList& lst) {
    uint8_t next = getJumpTarget(state,lst);
    if( next != AS_CM_JT_NONE ) {
      // get delay
      uint32_t dly = getDelayForState(next,lst);
      // switch to next
      setState(next,dly,lst);
    }
  }

  void toggleState () {
    setState( state == AS_CM_JT_ON ? AS_CM_JT_OFF : AS_CM_JT_ON, DELAY_INFINITE);
  }

  uint8_t getNextState () {
    switch( state ) {
      case AS_CM_JT_ONDELAY:  return AS_CM_JT_ON;
      case AS_CM_JT_ON:       return AS_CM_JT_OFFDELAY;
      case AS_CM_JT_OFFDELAY: return AS_CM_JT_OFF;
      case AS_CM_JT_OFF:      return AS_CM_JT_ONDELAY;
    }
    return AS_CM_JT_NONE;
  }

  uint8_t getJumpTarget(uint8_t stat,const SwitchPeerList& lst) const {
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  return lst.jtDlyOn();
      case AS_CM_JT_ON:       return lst.jtOn();
      case AS_CM_JT_OFFDELAY: return lst.jtDlyOff();
      case AS_CM_JT_OFF:      return lst.jtOff();
    }
    return AS_CM_JT_NONE;
  }

  uint32_t getDelayForState(uint8_t stat,const SwitchPeerList& lst) const {
    if( lst.valid() == false ) {
      return getDefaultDelay(stat);
    }
    uint8_t value = 0;
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  value = lst.onDly(); break;
      case AS_CM_JT_ON:       value = lst.onTime(); break;
      case AS_CM_JT_OFFDELAY: value = lst.offDly(); break;
      case AS_CM_JT_OFF:      value = lst.offTime(); break;
    }
    return byteTimeCvt(value);
  }

  uint32_t getDefaultDelay(uint8_t stat) const {
    switch( stat ) {
      case AS_CM_JT_ON:
      case AS_CM_JT_OFF:
        return DELAY_INFINITE;
    }
    return DELAY_NO;
  }

  bool delayActive () const { return aclock.get(alarm) > 0; }

  // get timer count in 1/10s
  static uint32_t byteTimeCvt(uint8_t tTime) {
    if( tTime == 0xff ) return 0xffffffff;
    const uint16_t c[8] = {1,10,50,100,600,3000,6000,36000};
    return (uint16_t)(tTime & 0x1F) * c[tTime >> 5];
  }

  // get timer count in 1/10s
  static uint32_t intTimeCvt(uint16_t iTime) {
    if (iTime == 0x00) return 0x00;
    if (iTime == 0xffff) return 0xffffffff;

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
      setState((counter & 0x01) == 0x01 ? AS_CM_JT_ON : AS_CM_JT_OFF, DELAY_INFINITE);
      break;
    case AS_CM_ACTIONTYPE_TOGGLE_INVERSE_TO_COUNTER:
      setState((counter & 0x01) == 0x00 ? AS_CM_JT_ON : AS_CM_JT_OFF, DELAY_INFINITE);
      break;
    }
  }

  void status (uint8_t stat, uint16_t delay) {
    setState( stat == 0 ? AS_CM_JT_OFF : AS_CM_JT_ON, intTimeCvt(delay) );
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
