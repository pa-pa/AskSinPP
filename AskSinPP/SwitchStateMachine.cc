
#include "SwitchStateMachine.h"

  void SwitchStateMachine::switchState(uint8_t oldstate,uint8_t newstate,uint8_t dly) {
    DPRINT(F("Switch State: "));
    DHEX(newstate);
    DPRINT(F(" ["));
    DHEX(dly);
    DPRINTLN(F("]"));
  }

  void SwitchStateMachine::jumpToTarget(SwitchCtrlList lst) {
    uint8_t next = AS_CM_JT_NONE;
    switch( state ) {
      case AS_CM_JT_ONDELAY:  next = lst.jtDlyOn();  break;
      case AS_CM_JT_ON:       next = lst.jtOn();     break;
      case AS_CM_JT_OFFDELAY: next = lst.jtDlyOff(); break;
      case AS_CM_JT_OFF:      next = lst.jtOff();    break;
      default: break;
    }
    if( next != AS_CM_JT_NONE ) {
      // first cancel possible running alarm
      aclock.cancel(alarm);
      // get delay
      uint8_t dly = getDelayForState(next,lst);
      // signal state change
      switchState(state,next,dly);
      state = next;
      if( dly == 0 ) {
        // switch immediately to next state
        jumpToTarget(lst);
      }
      else if( dly != 0xff ) {
        // setup alarm to switch after delay
        alarm.list(lst);
        alarm.set(byteTimeCvt(dly));
        aclock.add(alarm);
      }
    }
  }

  uint8_t SwitchStateMachine::getDelayForState(uint8_t s,SwitchCtrlList lst) {
    uint8_t dly = 0;
    switch( s ) {
      case AS_CM_JT_ONDELAY:  dly = lst.onDly();   break;
      case AS_CM_JT_ON:       dly = lst.onTime();  break;
      case AS_CM_JT_OFFDELAY: dly = lst.offDly();  break;
      case AS_CM_JT_OFF:      dly = lst.offTime(); break;
      default: break;
    }
    return dly;
  }

