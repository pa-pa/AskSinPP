
#include "SwitchStateMachine.h"

namespace as {

void SwitchStateMachine::switchState(uint8_t oldstate,uint8_t newstate) {}

void SwitchStateMachine::setState (uint8_t next,uint16_t duration) {
  if( next != AS_CM_JT_NONE ) {
    // first cancel possible running alarm
    aclock.cancel(alarm);
    if( state != next ) {
      switchState(state,next);
      state = next;
    }
    if( duration != 0xffff ) {
      alarm.action(AS_CM_JT_ON ? AS_CM_JT_OFF : AS_CM_JT_ON);
      alarm.set(intTimeCvt(duration));
      aclock.add(alarm);
    }
  }
}

void SwitchStateMachine::jumpToTarget(SwitchPeerList lst) {
  uint8_t next = getNextState(state,lst);
  if( next != AS_CM_JT_NONE ) {
    // get rest of delay value
    uint32_t olddelay = aclock.get(alarm);
    // first cancel possible running alarm
    aclock.cancel(alarm);
    // get delay
    uint8_t dly = getDelayForState(next,lst);
    // signal state change, if any
    if( state != next ) {
      switchState(state,next);
      state = next;
    }
    if( dly == 0 ) {
      // switch immediately to next state
      jumpToTarget(lst);
    }
    else if( dly != 0xff ) {
      uint32_t delayvalue = byteTimeCvt(dly);
      // setup alarm to process after delay
      alarm.action(AS_CM_JT_NONE); // NONE means JumpToTarget
      alarm.list(lst);
      alarm.set(max(delayvalue,olddelay));
      aclock.add(alarm);
    }
  }
}


}

