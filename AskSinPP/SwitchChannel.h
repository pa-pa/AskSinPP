
#ifndef __SWITCHCHANNEL_H__
#define __SWITCHCHANNEL_H__

#include "Debug.h"

#include "Channel.h"
#include "SwitchChannelData.h"
#include "SwitchStateData.h"
#include "SwitchStateMachine.h"


class SwitchChannel : public Channel, public SwitchStateMachine {

public:
  SwitchChannel () : Channel(SwitchChannelList::size(),SwitchStateList::size()) {}
  virtual ~SwitchChannel() {}

  virtual void switchState(uint8_t oldstate,uint8_t newstate,uint8_t dly) {
    DPRINT(F("Switch State: "));
    DHEX(newstate);
    DPRINT(F(" ["));
    DHEX(dly);
    DPRINT(F("] - "));
    if( newstate == AS_CM_JT_ON ) {
      DPRINTLN(F("ON"));
    }
    else if( newstate == AS_CM_JT_OFF ) {
      DPRINTLN(F("OFF"));
    }
    else {
      DPRINTLN(F("..."));
    }
  }

  void firstinit () {
    SwitchChannelList cl(listAddress(1));
    cl.defaults();
    for( uint8_t i=0; i<peers(); ++i ) {
      eeprom.clearData(peerAddress(i),Peer::size());
      eeprom.clearData(listAddress(3,i),SwitchStateList::size());
    }
  }

};

#endif
