
#ifndef __SWITCHSTATEMACHINE_H__
#define __SWITCHSTATEMACHINE_H__

#include "cm.h"
#include "SwitchList3.h"
#include "AlarmClock.h"

class SwitchStateMachine {

  class StateAlarm : public Alarm {
    SwitchStateMachine& sm;
    SwitchPeerList      lst;
  public:
    StateAlarm(SwitchStateMachine& m) : Alarm(0), sm(m), lst(0) {}
    void list(SwitchPeerList l) {lst=l;}
    virtual void trigger (AlarmClock& clock) {
      sm.jumpToTarget(lst);
    }
  };


  uint8_t    state : 4;
  StateAlarm alarm;

protected:
  ~SwitchStateMachine () {}

public:
  SwitchStateMachine() : state(AS_CM_JT_OFF), alarm(*this) {}

  virtual void switchState(uint8_t oldstate,uint8_t newstate,uint8_t dly);

  void jumpToTarget(SwitchPeerList lst);

  uint8_t getDelayForState(uint8_t s,SwitchPeerList lst);
  uint8_t getNextState(uint8_t s,SwitchPeerList lst);

  // get timer count in 1/10s
  uint32_t byteTimeCvt(uint8_t tTime) {
    const uint16_t c[8] = {1,10,50,100,600,3000,6000,36000};
    return (uint16_t)(tTime & 0x1F) * c[tTime >> 5];
  }


};

#endif
