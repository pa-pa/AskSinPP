//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SWITCH_H__
#define __SWITCH_H__

#include "MultiChannelDevice.h"
#include "Register.h"


namespace as {

DEFREGISTER(SwitchReg1,CREG_AES_ACTIVE,CREG_TRANSMITTRYMAX,CREG_POWERUPACTION,CREG_STATUSINFO)

class SwitchList1 : public RegList1<SwitchReg1> {
public:
  SwitchList1(uint16_t addr) : RegList1<SwitchReg1>(addr) {}
  void defaults () {
    clear ();
    //aesActive(false);
    transmitTryMax(6);
    //powerUpAction(false);
    statusInfoMinDly(4);
    statusInfoRandom(1);
  }
};

DEFREGISTER(SwitchReg3,PREG_CTDELAYONOFF,PREG_CTONOFF,
    PREG_CONDVALUELOW,PREG_CONDVALUEHIGH,PREG_ONDELAYTIME,PREG_ONTIME,
    PREG_OFFDELAYTIME,PREG_OFFTIME,PREG_ACTIONTYPE,PREG_JTONOFF,
    PREG_JTDELAYONOFF)

typedef RegList3<SwitchReg3> SwitchPeerList;

class SwitchList3 : public ShortLongList<SwitchPeerList> {
public:
  SwitchList3 (uint16_t addr) : ShortLongList<SwitchPeerList>(addr) {}
  void defaults() {
    SwitchPeerList ssl = sh();
    ssl.clear();
//    ssl.ctDlyOn(0);
//    ssl.ctDlyOff(0);
//    ssl.ctOn(0);
//    ssl.ctOff(0);
    ssl.ctValLo(0x32);
    ssl.ctValHi(0x64);
//    ssl.onDly(0);
    ssl.onTime(0xff);
//    ssl.offDly(0);
    ssl.offTime(0xff);
    ssl.actionType(AS_CM_ACTIONTYPE_JUMP_TO_TARGET);
//    ssl.offTimeMode(false);
//    ssl.onTimeMode(false);

    ssl = lg();
    ssl.clear();
//    ssl.ctDlyOn(0);
//    ssl.ctDlyOff(0);
//    ssl.ctOn(0);
//    ssl.ctOff(0);
    ssl.ctValLo(0x32);
    ssl.ctValHi(0x64);
//    ssl.onDly(0);
    ssl.onTime(0xff);
//    ssl.offDly(0);
    ssl.offTime(0xff);
    ssl.actionType(AS_CM_ACTIONTYPE_JUMP_TO_TARGET);
    ssl.multiExec(true);
//    ssl.offTimeMode(false);
//    ssl.onTimeMode(false);
  }

  void odd() {
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

  void even() {
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

  void single() {
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
    ssl.multiExec(false);
  }
};

class SwitchStateMachine {

#define DELAY_NO 0x00
#define DELAY_INFINITE 0xffffffff

  class StateAlarm : public Alarm {
    SwitchStateMachine& sm;
    SwitchPeerList      lst;
  public:
    StateAlarm(SwitchStateMachine& m) : Alarm(0), sm(m), lst(0) {}
    void list(SwitchPeerList l) {lst=l;}
    virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
      uint8_t next = sm.getNextState();
      uint32_t dly = sm.getDelayForState(next,lst);
      sm.setState(next,dly,lst);
    }
  };

  void setState (uint8_t next,uint32_t delay,const SwitchPeerList& lst=SwitchPeerList(0),uint8_t deep=0) {
    // check deep to prevent infinite recursion
    if( next != AS_CM_JT_NONE && deep < 4) {
      // first cancel possible running alarm
      sysclock.cancel(alarm);
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
        sysclock.add(alarm);
      }
    }
  }

protected:
  uint8_t    state;
  StateAlarm alarm;

public:
  SwitchStateMachine() : state(AS_CM_JT_NONE), alarm(*this) {}
  virtual ~SwitchStateMachine () {}

  virtual void switchState(__attribute__((unused)) uint8_t oldstate,__attribute__((unused)) uint8_t newstate) {}

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

  uint8_t getConditionForState(uint8_t stat,const SwitchPeerList& lst) const {
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  return lst.ctDlyOn();
      case AS_CM_JT_ON:       return lst.ctOn();
      case AS_CM_JT_OFFDELAY: return lst.ctDlyOff();
      case AS_CM_JT_OFF:      return lst.ctOff();
    }
    return AS_CM_CT_X_GE_COND_VALUE_LO;
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
    return AskSinBase::byteTimeCvt(value);
  }

  uint32_t getDefaultDelay(uint8_t stat) const {
    switch( stat ) {
      case AS_CM_JT_ON:
      case AS_CM_JT_OFF:
        return DELAY_INFINITE;
    }
    return DELAY_NO;
  }

  bool delayActive () const { return sysclock.get(alarm) > 0; }

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

  void sensor (const SwitchPeerList& lst,uint8_t counter,uint8_t value) {
    uint8_t cond = getConditionForState(state,lst);
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
      doit =((value < lst.ctValLo()) || (value >= lst.ctValHi()));
      break;
    }
    if( doit == true ) {
      remote(lst,counter);
    }
  }

  void status (uint8_t stat, uint16_t delay) {
    setState( stat == 0 ? AS_CM_JT_OFF : AS_CM_JT_ON, AskSinBase::intTimeCvt(delay) );
  }

  uint8_t status () const {
    return state == AS_CM_JT_OFF ? 0 : 200;
  }

  uint8_t flags () const {
    return delayActive() ? 0x40 : 0x00;
  }
};


template <class HalType,int PeerCount,class List0Type>
class SwitchChannel : public Channel<HalType,SwitchList1,SwitchList3,EmptyList,PeerCount,List0Type>, public SwitchStateMachine {

protected:
  typedef Channel<HalType,SwitchList1,SwitchList3,EmptyList,PeerCount,List0Type> BaseChannel;
  uint8_t lowact;
  uint8_t pin;
  uint8_t lastmsgcnt;

public:
  SwitchChannel () : BaseChannel(), lowact(false), pin(0), lastmsgcnt(0xff) {}
  virtual ~SwitchChannel() {}

  void init (uint8_t p,bool value=false) {
    pin=p;
    pinMode(pin,OUTPUT);
    lowact = value;
    typename BaseChannel::List1 l1 = BaseChannel::getList1();
    status(l1.powerUpAction() == true ? 200 : 0, 0xffff );
    BaseChannel::changed(true);
  }

  void setup(Device<HalType,List0Type>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev,number,addr);
  }

  uint8_t flags () const {
    uint8_t flags = SwitchStateMachine::flags();
    if( this->device().battery().low() == true ) {
      flags |= 0x80;
    }
    return flags;
  }


  virtual void switchState(__attribute__((unused)) uint8_t oldstate,uint8_t newstate) {
    if( newstate == AS_CM_JT_ON ) {
      digitalWrite(pin,lowact ? LOW : HIGH);
    }
    else if ( newstate == AS_CM_JT_OFF ) {
      digitalWrite(pin,lowact ? HIGH : LOW);
    }
    BaseChannel::changed(true);
  }
    
  bool process (__attribute__((unused)) const ActionCommandMsg& msg) {
    return true;
  }
    
  bool process (const ActionSetMsg& msg) {
    status( msg.value(), msg.delay() );
    return true;
  }

  bool process (const RemoteEventMsg& msg) {
    bool lg = msg.isLong();
    Peer p(msg.peer());
    uint8_t cnt = msg.counter();
    typename BaseChannel::List3 l3 = BaseChannel::getList3(p);
    if( l3.valid() == true ) {
      // l3.dump();
      typename BaseChannel::List3::PeerList pl = lg ? l3.lg() : l3.sh();
      // pl.dump();
      if( cnt != lastmsgcnt || (lg == true && pl.multiExec() == true) ) {
        lastmsgcnt = cnt;
        remote(pl,cnt);
      }
      return true;
    }
    return false;
  }

  bool process (const SensorEventMsg& msg) {
    bool lg = msg.isLong();
    Peer p(msg.peer());
    uint8_t cnt = msg.counter();
    uint8_t value = msg.value();
    typename BaseChannel::List3 l3 = BaseChannel::getList3(p);
    if( l3.valid() == true ) {
      // l3.dump();
      typename BaseChannel::List3::PeerList pl = lg ? l3.lg() : l3.sh();
      // pl.dump();
      sensor(pl,cnt,value);
      return true;
    }
    return false;
  }
};

}

#endif
