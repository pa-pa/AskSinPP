
#include "SwitchStateData.h"

  void SwitchStateList::defaults () {
    SwitchCtrlList ssl = sh();
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
    ssl.actionType(AS_CM_JT_ONDELAY);
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
    ssl.actionType(AS_CM_JT_ONDELAY);
    ssl.multiExec(true);
    ssl.offTimeMode(false);
    ssl.onTimeMode(false);
  }

  void SwitchStateList::odd () {
    defaults();
    SwitchCtrlList ssl = sh();
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

  void SwitchStateList::even () {
    defaults();
    SwitchCtrlList ssl = sh();
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

  void SwitchStateList::single () {
    defaults();
    SwitchCtrlList ssl = sh();
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



