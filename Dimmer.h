//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-03-29 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __DIMMER_H__
#define __DIMMER_H__

#include "MultiChannelDevice.h"
#include "Register.h"
#include "actors/PWM.h"

#include <stdarg.h>

#define LOGIC_INACTIVE 0
#define LOGIC_OR 1
#define LOGIC_AND 2
#define LOGIC_XOR 3
#define LOGIC_NOR 4
#define LOGIC_NAND 5
#define LOGIC_ORINVERS 6
#define LOGIC_ANDINVERS 7
#define LOGIC_PLUS 8
#define LOGIC_MINUS 9
#define LOGIC_MUL 10
#define LOGIC_PLUSINVERS 11
#define LOGIC_MINUSINVERS 12
#define LOGIC_MULINVERS 13
#define LOGIC_INVERSPLUS 14
#define LOGIC_INVERSMINUS 15
#define LOGIC_INVERSMUL 16

// rampos & rampoff for dimmer is refon & refoff
#undef AS_CM_JT_RAMPON
#define AS_CM_JT_RAMPON AS_CM_JT_REFON
#undef AS_CM_JT_RAMPOFF
#define AS_CM_JT_RAMPOFF AS_CM_JT_REFOFF

namespace as {

DEFREGISTER(DimmerReg1,CREG_AES_ACTIVE,CREG_TRANSMITTRYMAX,CREG_OVERTEMPLEVEL,
    CREG_REDUCETEMPLEVEL,CREG_REDUCELEVEL,CREG_POWERUPACTION,CREG_STATUSINFO,
    CREG_CHARACTERISTIC,CREG_LOGICCOMBINATION)

class DimmerList1 : public RegList1<DimmerReg1> {
public:
  DimmerList1 (uint16_t addr) : RegList1<DimmerReg1>(addr) {}
  void defaults () {
    clear();
    // aesActive(false);
    transmitTryMax(6);
    // powerUpAction(false);
    statusInfoMinDly(4);
    statusInfoRandom(1);

    overTempLevel(80);
    reduceTempLevel(75);
    reduceLevel(80);
    characteristic(true);
    logicCombination(LOGIC_OR);
  }
};

DEFREGISTER(DimmerReg3,PREG_CTRAMPONOFF,PREG_CTDELAYONOFF,PREG_CTONOFF,
  PREG_CONDVALUELOW,PREG_CONDVALUEHIGH,PREG_ONDELAYTIME,PREG_ONTIME,
  PREG_OFFDELAYTIME,PREG_OFFTIME,PREG_ACTIONTYPE,PREG_JTONOFF,
  PREG_JTDELAYONOFF,PREG_JTRAMPONOFF,PREG_DELAYMODE,PREG_OFFLEVEL,
  PREG_ONMINLEVEL,PREG_ONLEVEL,PREG_RAMPSTARTSTEP,PREG_RAMPONTIME,
  PREG_RAMPOFFTIME,PREG_DIMMINLEVEL,PREG_DIMMAXLEVEL,PREG_DIMSTEP,
  PREG_OFFDELAYSTEP,PREG_OFFDELAYNEWTIME,PREG_OFFDELAYOLDTIME,
  PREG_ELSEACTIONTYPE,PREG_ELSEJTONOFF,PREG_ELSEJTDELAYONOFF,
  PREG_ELSEJTRAMPONOFF)

typedef RegList3<DimmerReg3> DimmerPeerList;

class DimmerList3 : public ShortLongList<DimmerPeerList> {
public:
  DimmerList3 (uint16_t addr) : ShortLongList<DimmerPeerList>(addr) {}
  void defaults() {
    DimmerPeerList ssl = sh();
    ssl.clear();
//    ssl.ctRampOn(0);
//    ssl.ctRampOff(0);
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
    ssl.offDelayBlink(true);
//    ssl.offLevel(0);
    ssl.onMinLevel(2);
    ssl.onLevel(200); // 201 ???
    ssl.rampStartStep(10);
    ssl.rampOnTime(10);
    ssl.rampOffTime(10);
    ssl.dimMinLevel(10);
    ssl.dimMaxLevel(200);
    ssl.dimStep(5);
    ssl.offDelayStep(10);
    ssl.offDelayNewTime(5);
    ssl.offDelayOldTime(5);
    ssl.elseActionType(AS_CM_ACTIONTYPE_INACTIVE);
//    ssl.elseOffTimeMode(false);
//    ssl.elseOnTimeMode(false);
    ssl.elseJtOn(AS_CM_JT_ONDELAY);
    ssl.elseJtOff(AS_CM_JT_ONDELAY);
    ssl.elseJtDlyOn(AS_CM_JT_ONDELAY);
    ssl.elseJtDlyOff(AS_CM_JT_ONDELAY);
    ssl.elseJtRampOn(AS_CM_JT_ONDELAY);
    ssl.elseJtRampOff(AS_CM_JT_ONDELAY);

    ssl = lg();
    ssl.clear();
//    ssl.ctRampOn(0);
//    ssl.ctRampOff(0);
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
    ssl.offDelayBlink(true);
//    ssl.offLevel(0);
    ssl.onMinLevel(2);
    ssl.onLevel(200); // 201 ???
    ssl.rampStartStep(10);
    ssl.rampOnTime(10);
    ssl.rampOffTime(10);
    ssl.dimMinLevel(10);
    ssl.dimMaxLevel(200);
    ssl.dimStep(5);
    ssl.offDelayStep(10);
    ssl.offDelayNewTime(5);
    ssl.offDelayOldTime(5);
    ssl.elseActionType(AS_CM_ACTIONTYPE_INACTIVE);
//    ssl.elseOffTimeMode(false);
//    ssl.elseOnTimeMode(false);
    ssl.elseJtOn(AS_CM_JT_ONDELAY);
    ssl.elseJtOff(AS_CM_JT_ONDELAY);
    ssl.elseJtDlyOn(AS_CM_JT_ONDELAY);
    ssl.elseJtDlyOff(AS_CM_JT_ONDELAY);
    ssl.elseJtRampOn(AS_CM_JT_ONDELAY);
    ssl.elseJtRampOff(AS_CM_JT_ONDELAY);
  }

  void odd() {
    defaults();
    DimmerPeerList ssl = sh();
    ssl.jtOn(AS_CM_JT_OFFDELAY);
    ssl.jtOff(AS_CM_JT_OFF);
    ssl.jtDlyOn(AS_CM_JT_RAMPOFF);
    ssl.jtDlyOff(AS_CM_JT_RAMPOFF);
    ssl.jtRampOn(AS_CM_JT_RAMPOFF);
    ssl.jtRampOff(AS_CM_JT_OFF);
    ssl = lg();
    ssl.actionType(AS_CM_ACTIONTYPE_DOWNDIM);
  }

  void even() {
    defaults();
    DimmerPeerList ssl = sh();
    ssl.jtOn(AS_CM_JT_ON);
    ssl.jtOff(AS_CM_JT_ONDELAY);
    ssl.jtDlyOn(AS_CM_JT_RAMPON);
    ssl.jtDlyOff(AS_CM_JT_RAMPON);
    ssl.jtRampOn(AS_CM_JT_ON);
    ssl.jtRampOff(AS_CM_JT_RAMPON);
    ssl = lg();
    ssl.actionType(AS_CM_ACTIONTYPE_UPDIM);
  }

  void single() {
    defaults();
    DimmerPeerList ssl = sh();
    ssl.jtOn(AS_CM_JT_OFFDELAY);
    ssl.jtOff(AS_CM_JT_ONDELAY);
    ssl.jtDlyOn(AS_CM_JT_RAMPON);
    ssl.jtDlyOff(AS_CM_JT_RAMPOFF);
    ssl.jtRampOn(AS_CM_JT_ON);
    ssl.jtRampOff(AS_CM_JT_OFF);
    ssl = lg();
    ssl.actionType(AS_CM_ACTIONTYPE_TOGGLEDIM_TO_COUNTER);
  }

};


class DimmerStateMachine {

#define DELAY_NO 0x00
#define DELAY_INFINITE 0xffffffff

  class ChangedAlarm : public Alarm {
    DimmerStateMachine&  sm;
  public:
    ChangedAlarm (DimmerStateMachine& s) : Alarm(0), sm(s) {}
    virtual ~ChangedAlarm () {}
    void set (uint32_t t,AlarmClock& clock) {
      clock.cancel(*this);
      Alarm::set(t);
      clock.add(*this);
    }
    virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
      sm.change = true;
    }
  };


  class RampAlarm : public Alarm {
  public:
    DimmerStateMachine& sm;
    DimmerPeerList      lst;
    uint32_t            delay, tack;
    uint8_t             destlevel;
    uint8_t             dx;

    RampAlarm (DimmerStateMachine& m) : Alarm(0), sm(m), lst(0), delay(0), tack(0), destlevel(0), dx(5) {}
    void list (DimmerPeerList l) { lst=l; delay=tack=0; destlevel=sm.status(); }
    void init (uint8_t state,DimmerPeerList l) {
      uint8_t destlevel = state == AS_CM_JT_RAMPOFF ? 0 : 200;
      if( l.valid() == true ) {
        destlevel = state == AS_CM_JT_RAMPOFF ? l.offLevel() : l.onLevel();
      }
      init(sm.getDelayForState(state,l),destlevel,l.valid() ? 0 : DELAY_INFINITE,l);
    }
    void init (uint32_t ramptime,uint8_t level,uint32_t dly,DimmerPeerList l=DimmerPeerList(0)) {
      DPRINT("Ramp/Level: ");DDEC(ramptime);DPRINT("/");DDECLN(level);
      // check that we start with the defined minimum
      if( lst.valid() && sm.status() < lst.onMinLevel() ) {
        sm.updateLevel(lst.onMinLevel());
      }
      lst=l;
      destlevel = level==201 ? sm.lastonlevel : level;
      delay = dly;
      sm.updateState(destlevel==0 ? AS_CM_JT_RAMPOFF : AS_CM_JT_RAMPON, delay);
      uint8_t curlevel = sm.status();
      uint32_t diff;
      if( curlevel > destlevel ) { // dim down
        diff = curlevel - destlevel;
      }
      else { // dim up
        diff = destlevel - curlevel;
      }
      if( ramptime > diff ) {
        dx = 1;
        tack = ramptime / diff;
      }
      else {
        tack = 1;
        dx = uint8_t(diff / (ramptime > 0 ? ramptime : 1));
      }
      set(tack);
      //DPRINT("Level/Dx/Tack: ");DDEC(curlevel);DPRINT("/");DDEC(dx);DPRINT("/");DDECLN(tack);
    }
    virtual void trigger (AlarmClock& clock) {
      uint8_t curlevel = sm.status();
      // DHEX(curlevel);DPRINT("  ");DHEXLN(destlevel);
      if( sm.status() != destlevel ) {
        if( curlevel > destlevel ) { // dim down
          uint8_t rest = curlevel - destlevel;
          sm.updateLevel( curlevel - (rest < dx ? rest : dx));
        }
        else { // dim up
          uint8_t rest = destlevel - curlevel;
          sm.updateLevel( curlevel + (rest < dx ? rest : dx));
        }
      }
      // we catch our destination level -> go to next state
      if( sm.status() == destlevel ) {
        uint8_t next = sm.getNextState();
        if( delay == 0 && lst.valid() == true ) {
          delay = sm.getDelayForState(next,lst);
        }
        sm.setState(next,delay,lst);
      }
      else { // enable again for next ramp step
        set(tack);
        clock.add(*this);
      }
    }
  };

  void updateLevel (uint8_t newlevel) {
    // DPRINT("UpdLevel: ");DDECLN(newlevel);
    level = newlevel;
  }

  void updateState (uint8_t next,uint32_t delay) {
    if( state != next ) {
      switchState(state, next,delay);
      state = next;
      if ( state == AS_CM_JT_ON || state == AS_CM_JT_OFF ) {
        triggerChanged();
      }
    }
  }

  void triggerChanged () {
    calarm.set(decis2ticks(list1.statusInfoMinDly()*5),sysclock);
  }

  void setState (uint8_t next,uint32_t delay,const DimmerPeerList& lst=DimmerPeerList(0),uint8_t deep=0) {
    // check deep to prevent infinite recursion
    if( next != AS_CM_JT_NONE && deep < 4) {
      // first cancel possible running alarm
      sysclock.cancel(alarm);
      // if state is different
      if (state != next) {
        updateState(next,delay);
      }
      if ( state == AS_CM_JT_RAMPON || state == AS_CM_JT_RAMPOFF ) {
        alarm.init(state,lst);
        sysclock.add(alarm);
      }
      else {
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
  }

protected:
  uint8_t      state : 4;
  bool         change : 1;
  bool         toggledimup : 1;
  bool         erroverheat : 1;
  bool 		   erroroverload : 1;
  bool         errreduced : 1;
  uint8_t      level, lastonlevel;
  RampAlarm    alarm;
  ChangedAlarm calarm;
  DimmerList1  list1;

public:
  DimmerStateMachine() : state(AS_CM_JT_NONE), change(false), toggledimup(true), erroverheat(false), erroroverload(false), errreduced(false),
    level(0), lastonlevel(200), alarm(*this), calarm(*this), list1(0) {}
  virtual ~DimmerStateMachine () {}

  bool changed () const { return change; }
  void changed (bool c) { change=c; }

  void overheat(bool value) {
    erroverheat = value;
  }

  void overload(bool value){
	 erroroverload = value;
  }
  bool getoverload(){
	  return erroroverload;
  }
  void reduced (bool value) {
    errreduced = value;
  }

  void setup(DimmerList1 l1) {
    list1 = l1;
  }

  virtual void switchState(__attribute__ ((unused)) uint8_t oldstate,uint8_t newstate,__attribute__ ((unused)) uint32_t stateDelay) {
    // DPRINT("Dimmer State: ");DHEX(oldstate);DPRINT(" -> ");DHEX(newstate);DPRINT("  Level: ");DHEXLN(level);
    if( newstate == AS_CM_JT_ON ) {
      lastonlevel = level;
    }
  }

  void jumpToTarget(const DimmerPeerList& lst) {
    uint8_t next = getJumpTarget(state,lst);
    // DPRINT("Jmp: ");DHEX(state);DPRINT(" - ");DHEXLN(next);
    if( next != AS_CM_JT_NONE ) {
      // get delay
      uint32_t dly = getDelayForState(next,lst);
      // switch to next
      setState(next,dly,lst);
    }
  }

  void toggleState () {
    if( state == AS_CM_JT_OFF ) {
      setLevel(lastonlevel,5,0xffff);
    }
    else {
      setLevel(0,5,0xffff);
    }
  }

  uint8_t getNextState () {
    switch( state ) {
      case AS_CM_JT_ONDELAY:  return AS_CM_JT_RAMPON;
      case AS_CM_JT_RAMPON:   return AS_CM_JT_ON;
      case AS_CM_JT_ON:       return AS_CM_JT_OFFDELAY;
      case AS_CM_JT_OFFDELAY: return AS_CM_JT_RAMPOFF;
      case AS_CM_JT_RAMPOFF:  return AS_CM_JT_OFF;
      case AS_CM_JT_OFF:      return AS_CM_JT_ONDELAY;
    }
    return AS_CM_JT_NONE;
  }

  uint8_t getJumpTarget(uint8_t stat,const DimmerPeerList& lst) const {
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  return lst.jtDlyOn();
      case AS_CM_JT_RAMPON:   return lst.jtRampOn();
      case AS_CM_JT_ON:       return lst.jtOn();
      case AS_CM_JT_OFFDELAY: return lst.jtDlyOff();
      case AS_CM_JT_RAMPOFF:  return lst.jtRampOff();
      case AS_CM_JT_OFF:      return lst.jtOff();
    }
    return AS_CM_JT_NONE;
  }

  uint8_t getConditionForState(uint8_t stat,const DimmerPeerList& lst) const {
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  return lst.ctDlyOn();
      case AS_CM_JT_RAMPON:   return lst.ctRampOn();
      case AS_CM_JT_ON:       return lst.ctOn();
      case AS_CM_JT_OFFDELAY: return lst.ctDlyOff();
      case AS_CM_JT_RAMPOFF:  return lst.ctRampOff();
      case AS_CM_JT_OFF:      return lst.ctOff();
    }
    return AS_CM_CT_X_GE_COND_VALUE_LO;
  }

  uint32_t getDelayForState(uint8_t stat,const DimmerPeerList& lst) const {
    if( lst.valid() == false ) {
      return getDefaultDelay(stat);
    }
    uint8_t value = 0;
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  value = lst.onDly(); break;
      case AS_CM_JT_RAMPON:   value = lst.rampOnTime(); break;
      case AS_CM_JT_ON:       value = lst.onTime(); break;
      case AS_CM_JT_OFFDELAY: value = lst.offDly(); break;
      case AS_CM_JT_RAMPOFF:  value = lst.rampOffTime(); break;
      case AS_CM_JT_OFF:      value = lst.offTime(); break;
    }
    return AskSinBase::byteTimeCvt(value);
  }

  uint32_t getDefaultDelay(uint8_t stat) const {
    switch( stat ) {
      case AS_CM_JT_ON:
      case AS_CM_JT_OFF:
        return DELAY_INFINITE;
      case AS_CM_JT_RAMPON:
      case AS_CM_JT_RAMPOFF:
        return decis2ticks(5);
    }
    return DELAY_NO;
  }

  bool delayActive () const { return sysclock.get(alarm) > 0; }

  void dimUp (const DimmerPeerList& lst) {
    uint8_t dx = lst.dimStep();
    uint8_t newlevel = level+dx;
    if( newlevel > lst.dimMaxLevel() ) {
      newlevel = lst.dimMaxLevel();
    }
    updateState(AS_CM_JT_RAMPON,getDelayForState(AS_CM_JT_RAMPON, lst));
    updateLevel(newlevel);
    updateState(AS_CM_JT_ON,getDelayForState(AS_CM_JT_ON, lst));
  }

  void dimDown (const DimmerPeerList& lst) {
    uint8_t dx = lst.dimStep();
    uint8_t newlevel = level - (dx < level ? dx : level);
    if( newlevel < lst.dimMinLevel() ) {
      newlevel = lst.dimMinLevel();
    }
    uint8_t newstate = newlevel > lst.onMinLevel() ? AS_CM_JT_RAMPON : AS_CM_JT_RAMPOFF;
    updateState(newstate,getDelayForState(newstate, lst));
    updateLevel(newlevel);
    newstate = newlevel > lst.onMinLevel() ? AS_CM_JT_ON : AS_CM_JT_OFF;
    updateState(newstate,getDelayForState(newstate, lst));
  }

  bool set (uint8_t value,uint16_t ramp,uint16_t delay) {
    setLevel(value,ramp,delay);
    return true;
  }

  void remote (const DimmerPeerList& lst,uint8_t counter) {
    // perform action as defined in the list
    switch (lst.actionType()) {
    case AS_CM_ACTIONTYPE_JUMP_TO_TARGET:
      jumpToTarget(lst);
      break;
    case AS_CM_ACTIONTYPE_TOGGLE_TO_COUNTER:
      setState((counter & 0x01) == 0x01 ? AS_CM_JT_RAMPON : AS_CM_JT_RAMPOFF, DELAY_INFINITE, lst);
      break;
    case AS_CM_ACTIONTYPE_TOGGLE_INVERSE_TO_COUNTER:
      setState((counter & 0x01) == 0x01 ? AS_CM_JT_RAMPON : AS_CM_JT_RAMPOFF, DELAY_INFINITE, lst);
      break;
    case AS_CM_ACTIONTYPE_UPDIM:
      dimUp(lst);
      break;
    case AS_CM_ACTIONTYPE_DOWNDIM:
      dimDown(lst);
      break;
    case AS_CM_ACTIONTYPE_TOGGLEDIM:
      if( toggledimup == true ) dimUp(lst);
      else dimDown(lst);
      toggledimup = ! toggledimup;
      break;
    case AS_CM_ACTIONTYPE_TOGGLEDIM_TO_COUNTER:
      (counter & 0x01) == 0x01 ? dimUp(lst) : dimDown(lst);
      break;
    case AS_CM_ACTIONTYPE_TOGGLEDIM_TO_COUNTER_INVERSE:
      (counter & 0x01) == 0x00 ? dimUp(lst) : dimDown(lst);
      break;
    }

  }

  void sensor (const DimmerPeerList& lst,uint8_t counter,uint8_t value) {
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
    else {
      // TODO use else jump table
    }
  }

  void setLevel (uint8_t level, uint16_t ramp, uint16_t delay) {
    // DPRINT("SetLevel: ");DHEX(level);DPRINT(" ");DHEX(ramp);DPRINT(" ");DHEXLN(delay);
    sysclock.cancel(alarm);
    if( ramp==0 ) {
      alarm.destlevel=level;
      updateLevel(level);
      setState(level==0 ? AS_CM_JT_OFF : AS_CM_JT_ON, AskSinBase::intTimeCvt(delay));
    }
    else {
      alarm.init(AskSinBase::intTimeCvt(ramp), level, AskSinBase::intTimeCvt(delay));
      sysclock.add(alarm);
    }
  }

  void stop () {}

  uint8_t status () const {
    return level;
  }

  uint8_t flags () const {
    uint8_t f = delayActive() ? 0x40 : 0x00;
    if( erroverheat == true ) {
      f |= AS_CM_EXTSTATE_OVERHEAT;
    }
	if( erroroverload == true) {
	  f |= AS_CM_EXTSTATE_OVERLOAD;
	}
    if( errreduced == true ) {
      f |= AS_CM_EXTSTATE_REDUCED;
    }
    if( alarm.destlevel < level) {
      f |= AS_CM_EXTSTATE_DOWN;
    }
    else if( alarm.destlevel > level) {
      f |= AS_CM_EXTSTATE_UP;
    }
    return f;
  }
};

template <class HalType,int PeerCount,class List0Type=List0>
class DimmerChannel : public ActorChannel<HalType,DimmerList1,DimmerList3,PeerCount,List0Type,DimmerStateMachine> {
  uint8_t* phys;
protected:
  typedef ActorChannel<HalType,DimmerList1,DimmerList3,PeerCount,List0Type,DimmerStateMachine> BaseChannel;
public:
  DimmerChannel () : BaseChannel(), phys(0) {}

  void setPhysical(uint8_t& p) {
    phys = &p;
  }

  void patchStatus (Message& msg) {
    if( msg.length() == 0x0e ) {
      msg.length(0x0f);
      if( phys != 0 ) {
        msg.data()[3] = *phys;
      }
    }
  }
};

template<class HalType,class ChannelType,int ChannelCount,int VirtualCount,class List0Type=List0>
class DimmerDevice : public MultiChannelDevice<HalType,ChannelType,ChannelCount,List0Type> {
public:
  typedef MultiChannelDevice<HalType,ChannelType,ChannelCount,List0Type> DeviceType;

  DimmerDevice (const DeviceInfo& info,uint16_t addr) : DeviceType(info,addr) {}
  virtual ~DimmerDevice () {}

  /* the following definitions are needed for the DimmerControler */
  static int const channelCount = ChannelCount;
  static int const virtualCount = VirtualCount;
  typedef ChannelType DimmerChannelType;
  DimmerChannelType& dimmerChannel(uint8_t ch) {
    return this->channel(ch);
  }
};


template<class HalType,class DimChannelType,class RmtChannelType,int DimChannelCount,int DimVirtualCount,int RmtChannelCount, class List0Type=List0>
class DimmerAndRemoteDevice : public ChannelDevice<HalType, VirtBaseChannel<HalType, List0Type>, DimChannelCount + RmtChannelCount, List0Type> {

public:
	VirtChannel<HalType, DimChannelType, List0Type> dmc[DimChannelCount];
	VirtChannel<HalType, RmtChannelType, List0Type> rmc[RmtChannelCount];	
  public:
    typedef ChannelDevice<HalType, VirtBaseChannel<HalType, List0Type>, DimChannelCount + RmtChannelCount, List0Type> DeviceType;
	 DimmerAndRemoteDevice (const DeviceInfo& info, uint16_t addr) : DeviceType(info, addr) {
		for( uint8_t i=0; i<RmtChannelCount; ++i ) {
			DeviceType::registerChannel(rmc[i], i+1);
		}
		for( uint8_t j=0; j<DimChannelCount; ++j ) {
			DeviceType::registerChannel(dmc[j], j+RmtChannelCount+1);
		}
    }
    virtual ~DimmerAndRemoteDevice () {}
	
	/* the following definitions are needed for the DimmerControler */
	static int const channelCount = DimChannelCount;
	static int const virtualCount = DimVirtualCount;
	typedef DimChannelType DimmerChannelType;
    DimmerChannelType& dimmerChannel(uint8_t ch) {
		return this->dmc[ch-1];
    }
	typedef RmtChannelType RemoteChannelType;
	RemoteChannelType& remoteChannel(uint8_t re){
		return this->rmc[re-1];
	}
};

template<class HalType,class DimmerType,class PWM>
class DimmerControl {
protected:
  DimmerType& dimmer;
  PWM pwms[DimmerType::channelCount/DimmerType::virtualCount];
  uint8_t physical[DimmerType::channelCount/DimmerType::virtualCount];
  uint8_t factor[DimmerType::channelCount/DimmerType::virtualCount];
private:
  uint8_t counter;
  uint8_t overloadcounter;

  class ChannelCombiner : public Alarm {
    DimmerControl<HalType,DimmerType,PWM>& control;
  public:
    ChannelCombiner (DimmerControl<HalType,DimmerType,PWM>& d) : Alarm(0), control(d) {}
    virtual ~ChannelCombiner () {}
    virtual void trigger (AlarmClock& clock) {
      control.updatePhysical();
      set(millis2ticks(10));
      clock.add(*this);
    }
  } cb;

public:
  DimmerControl (DimmerType& dim) : dimmer(dim), counter(0), overloadcounter(0), cb(*this) {}
  virtual ~DimmerControl () {}

  uint8_t channelCount  () { return DimmerType::channelCount; }
  uint8_t virtualCount  () { return DimmerType::virtualCount; }
  uint8_t physicalCount () { return DimmerType::channelCount/DimmerType::virtualCount; }

  void firstinit () {
    for( uint8_t i=1; i<=channelCount(); ++i ) {
      if( i <= physicalCount() ){
        dimmer.dimmerChannel(i).getList1().logicCombination(LOGIC_OR);
      }
      else {
        dimmer.dimmerChannel(i).getList1().logicCombination(LOGIC_INACTIVE);
      }
    }
  }

  bool init (HalType& hal,...) {
    bool first = dimmer.init(hal);
    if( first == true ) {
      firstinit();
    }
    va_list argp;
    va_start(argp, hal);
    for( uint8_t i=0; i<physicalCount(); ++i ) {
      uint8_t p =  va_arg(argp, int);
      pwms[i].init(p);
      physical[i] = 0;
      factor[i] = 200; // 100%
    }
    va_end(argp);
    initChannels();
    cb.trigger(sysclock);
    return first;
  }

  void initChannels () {
    for( uint8_t i=1; i<=physicalCount(); ++i ) {
      for( uint8_t j=i; j<=channelCount(); j+=physicalCount() ) {
        dimmer.dimmerChannel(j).setPhysical(physical[i-1]);
        bool powerup = dimmer.dimmerChannel(j).getList1().powerUpAction();
        dimmer.dimmerChannel(j).setLevel(powerup == true ? 200 : 0,0,0xffff);
      }
    }
  }

  virtual void updatePhysical () {
    // DPRINT("Pin ");DHEX(pin);DPRINT("  Val ");DHEXLN(calcPwm());
    for( uint8_t i=0; i<physicalCount(); ++i ) {
      uint8_t value = (uint8_t)combineChannels(i+1);
      value = (((uint16_t)factor[i] * (uint16_t)value) / 200);
      if( physical[i] != value ) {
        // DPRINT("Ch: ");DDEC(i+1);DPRINT(" Phy: ");DDECLN(value);
        physical[i]  = value;
        pwms[i].set(physical[i]);
      }
    }
  }

  uint16_t combineChannels (uint8_t start) {
    uint16_t value = 0;
    for( uint8_t i=start; i<=channelCount(); i+=physicalCount() ) {
      uint8_t level = dimmer.dimmerChannel(i).status();
      switch( dimmer.dimmerChannel(i).getList1().logicCombination() ) {
      default:
      case LOGIC_INACTIVE:
        break;
      case LOGIC_OR:
        value = value > level ? value : level;
        break;
      case LOGIC_AND:
        value = value < level ? value : level;
        break;
      case LOGIC_XOR:
        value = value==0 ? level : (level==0 ? value : 0);
        break;
      case LOGIC_NOR:
        value = 200 - (value > level ? value : level);
        break;
      case LOGIC_NAND:
        value = 200 - (value < level ? value : level);
        break;
      case LOGIC_ORINVERS:
        level = 200 - level;
        value = value > level ? value : level;
        break;
      case LOGIC_ANDINVERS:
        level = 200 - level;
        value = value < level ? value : level;
        break;
      case LOGIC_PLUS:
        value += level;
        if( value > 200 ) value = 200;
        break;
      case LOGIC_MINUS:
        if( level > value ) value = 0;
        else value -= level;
        break;
      case LOGIC_MUL:
        value = value * level / 200;
        break;
      case LOGIC_PLUSINVERS:
        level = 200 - level;
        value += level;
        if( value > 200 ) value = 200;
        break;
        break;
      case LOGIC_MINUSINVERS:
        level = 200 - level;
        if( level > value ) value = 0;
        else value -= level;
        break;
      case LOGIC_MULINVERS:
        level = 200 - level;
        value = value * level / 200;
        break;
      case LOGIC_INVERSPLUS:
        value += level;
        if( value > 200 ) value = 200;
        value = 200 - value;
        break;
      case LOGIC_INVERSMINUS:
        if( level > value ) value = 0;
        else value -= level;
        value = 200 - value;
        break;
      case LOGIC_INVERSMUL:
        value = value * level / 200;
        value = 200 - value;
        break;
      }
    }
    // DHEXLN(value);
    return value;
  }
  
  
  void setOverload (bool overload=false) {
      counter++;
      if ( overload ){
          overloadcounter++;
          
      }
      for( uint8_t i=1; i<=physicalCount(); ++i ) {
        typename DimmerType::DimmerChannelType& c = dimmer.dimmerChannel(i);
        if ( counter > 5 ){
            if((counter - overloadcounter) <= 2 ){
              factor[i-1] = 0;
              c.overload(true);
          }
          else{
             counter = 0;
             overloadcounter = 0;
          }
          
        }
        else if ( c.getoverload()) {
              c.overload(false);
              factor[i-1] = 200;
          }
        }
    }

  void setTemperature (uint16_t temp) {
    uint8_t t = temp/10;
    for( uint8_t i=1; i<=physicalCount(); ++i ) {
      typename DimmerType::DimmerChannelType& c = dimmer.dimmerChannel(i);
      if( c.getList1().overTempLevel() <= t ) {
        factor[i-1] = 0; // overtemp -> switch off
        c.overheat(true);
        c.reduced(false);
      }
      else if( c.getList1().reduceTempLevel() <= t ) {
        factor[i-1] = c.getList1().reduceLevel();
        c.overheat(false);
        c.reduced(true);
      }
      else {
        factor[i-1] = 200; // 100%
        c.overheat(false);
        c.reduced(false);
      }
    }
  }
};


template<class HalType,class DimmerType,class PWM>
class DualWhiteControl : public DimmerControl<HalType,DimmerType,PWM> {
public:
  typedef DimmerControl<HalType,DimmerType,PWM> BaseControl;
  DualWhiteControl (DimmerType& dim) : BaseControl(dim) {
#ifndef NDEBUG
    if( this->physicalCount() != 2 ) {
      DPRINTLN("DualWhiteControl needs physical count == 2");
    }
#endif
  }
  virtual ~DualWhiteControl () {}

  virtual void updatePhysical () {
    uint16_t bright = this->combineChannels(1);
    uint16_t adjust = this->combineChannels(2);
    // set the values
    if( this->physical[0] != bright || this->physical[1] != adjust) {
      this->physical[0]  = bright;
      this->physical[1]  = adjust;
      // adjust the color temp
      uint8_t pwm0 = (bright * (200-adjust)) / 200;
      uint8_t pwm1 = (bright * adjust) / 200;
      this->pwms[0].set(pwm0);
      this->pwms[1].set(pwm1);
    }
  }
};

}

#endif
