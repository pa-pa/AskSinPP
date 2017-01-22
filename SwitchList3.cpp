//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#include "SwitchList3.h"

namespace as {

void SwitchList3::defaults() {
  SwitchPeerList ssl = sh();
  ssl.ctDlyOn(0);
  ssl.ctDlyOff(0);
  ssl.ctOn(0);
  ssl.ctOff(0);
  ssl.ctValLo(0x32);
  ssl.ctValHi(0x64);
  ssl.onDly(0);
  ssl.onTime(0xff);
  ssl.offDly(0);
  ssl.offTime(0xff);
  ssl.actionType(AS_CM_ACTIONTYPE_JUMP_TO_TARGET);
  ssl.offTimeMode(false);
  ssl.onTimeMode(false);

  ssl = lg();
  ssl.ctDlyOn(0);
  ssl.ctDlyOff(0);
  ssl.ctOn(0);
  ssl.ctOff(0);
  ssl.ctValLo(0x32);
  ssl.ctValHi(0x64);
  ssl.onDly(0);
  ssl.onTime(0xff);
  ssl.offDly(0);
  ssl.offTime(0xff);
  ssl.actionType(AS_CM_ACTIONTYPE_JUMP_TO_TARGET);
  ssl.multiExec(true);
  ssl.offTimeMode(false);
  ssl.onTimeMode(false);
}

void SwitchList3::odd() {
  defaults();
  SwitchPeerList ssl = sh();
  ssl.jtOn(AS_CM_JT_OFFDELAY);
  ssl.jtOff(AS_CM_JT_OFF);
  ssl.jtDlyOn(AS_CM_JT_OFF);
  ssl.jtDlyOff(AS_CM_JT_OFF);
  ssl = lg();
  ssl.jtOn(AS_CM_JT_OFFDELAY);
  ssl.jtOff(AS_CM_JT_OFF);
  ssl.jtDlyOn(AS_CM_JT_OFF);
  ssl.jtDlyOff(AS_CM_JT_OFF);
}

void SwitchList3::even() {
  defaults();
  SwitchPeerList ssl = sh();
  ssl.jtOn(AS_CM_JT_ON);
  ssl.jtOff(AS_CM_JT_ONDELAY);
  ssl.jtDlyOn(AS_CM_JT_ON);
  ssl.jtDlyOff(AS_CM_JT_ON);
  ssl = lg();
  ssl.jtOn(AS_CM_JT_ON);
  ssl.jtOff(AS_CM_JT_ONDELAY);
  ssl.jtDlyOn(AS_CM_JT_ON);
  ssl.jtDlyOff(AS_CM_JT_ON);
}

void SwitchList3::single() {
  defaults();
  SwitchPeerList ssl = sh();
  ssl.jtOn(AS_CM_JT_OFFDELAY);
  ssl.jtOff(AS_CM_JT_ONDELAY);
  ssl.jtDlyOn(AS_CM_JT_ON);
  ssl.jtDlyOff(AS_CM_JT_OFF);
  ssl = lg();
  ssl.jtOn(AS_CM_JT_OFFDELAY);
  ssl.jtOff(AS_CM_JT_ONDELAY);
  ssl.jtDlyOn(AS_CM_JT_ON);
  ssl.jtDlyOff(AS_CM_JT_OFF);
}

}

