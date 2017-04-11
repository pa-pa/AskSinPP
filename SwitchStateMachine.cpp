//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#include "SwitchStateMachine.h"

namespace as {

void SwitchStateMachine::switchState(__attribute__((unused)) uint8_t oldstate,__attribute__((unused)) uint8_t newstate) {}

void SwitchStateMachine::setState (uint8_t next,uint32_t delay,const SwitchPeerList& lst,uint8_t deep) {
  // check deep to prevent infinite recursion
  if( next != AS_CM_JT_NONE && deep < 4) {
    // first cancel possible running alarm
    aclock.cancel(alarm);
    // if state is different
    if (state != next) {
      switchState(state, next);
      state = next;
    }
    if (delay == DELAY_NO) {
      // go immediately to the next state
      next = getNextState();
      delay = getDelayForState(next,lst);
      setState(next, delay, lst, ++deep);
    }
    else if (delay != DELAY_INFINITE) {
      alarm.list(lst);
      alarm.set(delay);
      aclock.add(alarm);
    }
  }
}

}

