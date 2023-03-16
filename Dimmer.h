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


DEFREGISTER(DimmerReg0, MASTERID_REGS, DREG_CONFBUTTONTIME, DREG_LOCALRESETDISABLE,
  DREG_SPEEDMULTIPLIER)

class DimmerList0 : public RegList0<DimmerReg0> {
public:
  DimmerList0(uint16_t addr) : RegList0<DimmerReg0>(addr) {}

  void defaults() {
    clear();
    confButtonTime(255);
    localResetDisable(0);
    speedMultiplier(5);
  }
}; 
  
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
    ssl.actionType(AS_CM_ACTIONTYPE_INACTIVE);
    //ssl.jtOn(AS_CM_JT_OFFDELAY);
    //ssl.jtOff(AS_CM_JT_ONDELAY);
    //ssl.jtDlyOn(AS_CM_JT_RAMPON);
    //ssl.jtDlyOff(AS_CM_JT_RAMPOFF);
    //ssl.jtRampOn(AS_CM_JT_ON);
    //ssl.jtRampOff(AS_CM_JT_OFF);
    ssl = lg();
    ssl.actionType(AS_CM_ACTIONTYPE_INACTIVE);
  }

};


class DimmerStateMachine {

#define DELAY_NO 0x00
#define DELAY_INFINITE 0xffffffff

  class ChangedAlarm : public Alarm {
    enum { CHANGED=0x04 };
  public:
    ChangedAlarm () : Alarm(0) {}
    virtual ~ChangedAlarm () {}
    void set (uint32_t t,AlarmClock& clock) {
      clock.cancel(*this);
      Alarm::set(t);
      clock.add(*this);
    }
    virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
      setflag(CHANGED);
    }
    bool changed () const { return hasflag(CHANGED); }
    void changed (bool c) { setflag(c,CHANGED); }
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
      DPRINT(F("Ramp/Level: "));DDEC(ramptime);DPRINT(F("/"));DDECLN(level);
      // check that we start with the defined minimum
      if( lst.valid() && sm.status() !=0 && sm.status() < lst.onMinLevel() ) {
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
      if (diff == 0) { 
        dx = 0; 
        tack = 1; 
      } 
      else if( ramptime > diff ) {
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

  class BlinkAlarm : public Alarm {
  public:
    DimmerStateMachine& sm;
    volatile uint8_t level;
    uint8_t tack;

    BlinkAlarm(DimmerStateMachine& m) : Alarm(0), sm(m), tack(millis2ticks(500)) {} 
    virtual ~BlinkAlarm() {}

    void init(DimmerPeerList l) {
      if (!l.offDelayBlink()) return;
      updateLevel();
      set(tack);
      //DPRINT("init: "); DPRINT(l.offDelayBlink()); DPRINT(", level: "); DDEC(level);  DPRINT(" - "); DDECLN(millis());
    }
    void updateLevel() {
      level = sm.status();
    }
    virtual void trigger(AlarmClock& clock) {
      uint8_t temp = (sm.status() == level) ? level - (level/4) : level;
      sm.updateLevel(temp);
      //DPRINT("trigger: "); DPRINTLN(temp);
      set(tack);
      clock.add(*this);
    }
  };

  void updateLevel (uint8_t newlevel) {
    //DPRINT("UpdLevel: ");DDECLN(newlevel);
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
    const char* dbgJT[] = {
      PSTR("NONE"), 
      PSTR("ONDELAY"), 
      PSTR("RAMPON")/*REFON*/, 
      PSTR("ON"), 
      PSTR("OFFDELAY"),
      PSTR("RAMPOFF")/*REFOFF*/, 
      PSTR("OFF"), 
      PSTR("na"), 
      PSTR("RAMPON"), 
      PSTR("RAMPOFF"),
    };
    DPRINT(F("setState: ")); DPRINT(dbgJT[state]); DPRINT("->"); DPRINT(dbgJT[next]); DPRINT('\n');


    // check deep to prevent infinite recursion
    if( next != AS_CM_JT_NONE && deep < 4) {
      // first cancel possible running alarm
      sysclock.cancel(blink);
      sysclock.cancel(alarm);
      // if state is different
      if (state != next) {
        updateState(next, delay);
      }
      if (state == AS_CM_JT_OFFDELAY) {
        blink.init(lst);
        sysclock.add(blink);
      }
      //if (state == AS_CM_JT_OFFDELAY) blink.end();
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
  enum { TOGGLEDIMUP=0x08,ERROROVERHEAT=0x10,ERROROVERLOAD=0x20,ERRORREDUCED=0x40 };

  uint8_t      state;
  uint8_t      level, lastonlevel;
  RampAlarm    alarm;
  BlinkAlarm   blink;
  ChangedAlarm calarm;
  DimmerList1  list1;

public:
  DimmerStateMachine() : state(AS_CM_JT_NONE),
    level(0), lastonlevel(200), alarm(*this), blink(*this), calarm(), list1(0) { calarm.setflag(TOGGLEDIMUP); }
  virtual ~DimmerStateMachine () {}

  bool changed () const { return calarm.changed(); }
  void changed (bool c) { calarm.changed(c); }

  void overheat(bool value) {
    calarm.setflag(value,ERROROVERHEAT);
  }
  void overload(bool value){
    calarm.setflag(value,ERROROVERLOAD);
  }
  void reduced (bool value) {
    calarm.setflag(value,ERRORREDUCED);
  }
  bool getoverload(){
    return calarm.hasflag(ERROROVERLOAD);
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
      // on/off time mode / absolute / minimal
      if( next == state && (next == AS_CM_JT_ON || next == AS_CM_JT_OFF) && dly < DELAY_INFINITE) {
        bool minimal = next == AS_CM_JT_ON ? lst.onTimeMode() : lst.offTimeMode();
        // if minimal is set - we jump out if the new delay is shorter
        if( minimal == true ) {
          // DPRINT("Minimal");DDECLN(dly);
          uint32_t curdly = sysclock.get(alarm); // 0 means DELAY_INFINITE
          if( curdly == 0 || curdly > dly ) {
            // DPRINTLN(F("Skip short Delay"));
            return;
          }
        }
      }
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
      default: break;
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
      default: break;
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
      default: break;
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
      default: break;
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
      default: break;
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
    blink.updateLevel();
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
    blink.updateLevel();
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
      if( calarm.hasflag(TOGGLEDIMUP) ) {
        dimUp(lst);
        calarm.remflag(TOGGLEDIMUP);
      }
      else {
        dimDown(lst);
        calarm.setflag(TOGGLEDIMUP);
      }
      break;
    case AS_CM_ACTIONTYPE_TOGGLEDIM_TO_COUNTER:
      (counter & 0x01) == 0x01 ? dimUp(lst) : dimDown(lst);
      break;
    case AS_CM_ACTIONTYPE_TOGGLEDIM_TO_COUNTER_INVERSE:
      (counter & 0x01) == 0x00 ? dimUp(lst) : dimDown(lst);
      break;
    default: break;
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
    default: break;
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
    if( calarm.hasflag(ERROROVERHEAT) == true ) {
      f |= AS_CM_EXTSTATE_OVERHEAT;
    }
    if( calarm.hasflag(ERROROVERLOAD) == true ) {
      f |= AS_CM_EXTSTATE_OVERLOAD;
    }
    if( calarm.hasflag(ERRORREDUCED) == true ) {
      f |= AS_CM_EXTSTATE_REDUCED;
    }
    if( alarm.active()) {
      if( alarm.destlevel < level) {
        f |= AS_CM_EXTSTATE_DOWN;
      }
      else if( alarm.destlevel > level) {
        f |= AS_CM_EXTSTATE_UP;
      }
    }
    return f;
  }
};

template <class HalType,int PeerCount,class List0Type= DimmerList0>
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

template<class HalType,class ChannelType,int ChannelCount,int VirtualCount,class List0Type= DimmerList0>
class DimmerDevice : public MultiChannelDevice<HalType,ChannelType,ChannelCount,List0Type> {
public:
//  bool configHasChanged = false;
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


template<class HalType,class DimChannelType,class RmtChannelType,int DimChannelCount,int DimVirtualCount,int RmtChannelCount, class List0Type= DimmerList0>
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

  bool init (HalType& hal,const uint8_t pins[]) {
    bool first = dimmer.init(hal);
    if( first == true ) {
      firstinit();
    }
    for( uint8_t i=0; i<physicalCount(); ++i ) {
      pwms[i].init(pins[i]);
      physical[i] = 0;
      factor[i] = 200; // 100%
    }
    initChannels();
    cb.trigger(sysclock);
    return first;
  }

  PWM& pwm (uint8_t num) { return pwms[num]; }

  void initChannels() {
    for (uint8_t i = 1; i <= physicalCount(); ++i) {
      for (uint8_t j = i; j <= channelCount(); j += physicalCount()) {
        dimmer.dimmerChannel(j).setPhysical(physical[i - 1]);
        dimmer.dimmerChannel(j).setLevel(0, 0, 0xffff);

        uint8_t powerup = dimmer.dimmerChannel(j).getList1().powerUpAction();
        //DPRINT(F("init_cnl:")); DPRINT(j); DPRINT(F(", pwrup:")); DPRINTLN(powerup);
        Peer ownID(1);
        dimmer.getDeviceID(ownID);
        DimmerList3 l3 = dimmer.dimmerChannel(j).getList3(ownID);
        //ownID.dump(); DPRINT('\n');

        if (powerup == true && l3.valid() == true) {
          typename DimmerList3::PeerList pl = l3.sh();
          dimmer.dimmerChannel(j).remote(pl, 1);
        } 
      }
    }
  }

  virtual void updatePhysical () {
    checkParam();
    // DPRINT("Pin ");DHEX(pin);DPRINT("  Val ");DHEXLN(calcPwm());
    for( uint8_t i=0; i<physicalCount(); ++i ) {
      uint8_t value = (uint8_t)combineChannels(i+1);
      value = (((uint16_t)factor[i] * (uint16_t)value) / 200);
      if (physical[i] != value) {
        //DPRINT("Ch: "); DDEC(i + 1); DPRINT(" sm: "); DPRINT(dimmer.sm); DPRINT(" Phy: "); DDECLN(value);
        physical[i]  = value;
        pwms[i].set(physical[i]);
      }
    }
  }

  uint16_t combineChannels (uint8_t start) {
    if( virtualCount() == 1 ) {
      return dimmer.dimmerChannel(start).status();
    }
    else {
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

  void setTemperature (int16_t temp) {
    temp = temp/10;
    for( uint8_t i=1; i<=physicalCount(); ++i ) {
      typename DimmerType::DimmerChannelType& c = dimmer.dimmerChannel(i);
      if( (int16_t)c.getList1().overTempLevel() <= temp ) {
        factor[i-1] = 0; // overtemp -> switch off
        c.overheat(true);
        c.reduced(false);
      }
      else if( (int16_t)c.getList1().reduceTempLevel() <= temp ) {
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

  void checkParam() {
    bool cc = this->dimmer.hasConfigChanged();
    if (cc == false) return;
    for (uint8_t i = 0; i < physicalCount(); ++i) {
      uint8_t speedMultiplier = dimmer.getList0().speedMultiplier();
      uint8_t characteristic = dimmer.dimmerChannel(i + 1).getList1().characteristic();
      pwms[i].param(speedMultiplier, characteristic);
    }
  }

};


template<class HalType,class DimmerType,class PWM>
class DualWhiteControl : public DimmerControl<HalType,DimmerType,PWM> {
public:
  uint8_t cBaseType;         // "CHARACTERISTIC_CROSSFADE" default="true", "CHARACTERISTIC_DIM2WARM", "CHARACTERISTIC_DIM2HOT"
  uint8_t cLevelLimit;       // 1="CHARACTERISTIC_HALF_CONSTANT" default="true", 2="CHARACTERISTIC_MAXIMUM"
  uint8_t cCurve;            // linear or quadratic for dimmerchannel 2
  uint8_t cColourAssignment; // "CHARACTERISTIC_LOW_IS_WARM" default="true", "CHARACTERISTIC_LOW_IS_COLD"

  typedef DimmerControl<HalType,DimmerType,PWM> BaseControl;
  DualWhiteControl (DimmerType& dim) : BaseControl(dim) {
  #ifndef NDEBUG
    if( this->physicalCount() != 2 ) {
      DPRINTLN(F("DualWhiteControl needs physical count == 2"));
    }
  #endif
  }
  virtual ~DualWhiteControl () {}

  virtual void updatePhysical() {
    updatePhysical(this->checkParam(), this->combineChannels(1), this->combineChannels(2));
  }

  virtual void updatePhysical(uint8_t param, uint8_t bright, uint8_t adjust) {

    if (param == 0 && this->physical[0] == bright && this->physical[1] == adjust) return;
    this->physical[0] = bright;
    this->physical[1] = adjust;

    uint8_t pwmCold, pwmWarm;
    uint8_t _adjust = 200 - adjust;


    if (cBaseType == 0) {      // CHARACTERISTIC_CROSSFADE 

      if ((cLevelLimit == 0) && (cCurve == 0)) {
        pwmCold = adjust;
        pwmWarm = _adjust;
      }
      else if ((cLevelLimit == 1) && (cCurve == 0)) {
        pwmCold = (adjust > 100) ? 200 : adjust * 2;
        pwmWarm = (_adjust > 100) ? 200 : _adjust * 2;
      }
      else if ((cLevelLimit == 0) && (cCurve == 1)) {
        pwmCold = (adjust > 100) ? ((int32_t)_adjust * (int32_t)_adjust / -100) + 200 : (uint16_t)adjust * (uint16_t)adjust / 100;
        pwmWarm = (adjust > 100) ? (uint16_t)_adjust * (uint16_t)_adjust / 100 : ((int32_t)adjust * (int32_t)adjust / -100) + 200;
      }
      else if ((cLevelLimit == 1) && (cCurve == 1)) {
        pwmCold = (adjust > 100) ? 200 : (uint16_t)adjust * (uint16_t)adjust / 50;
        pwmWarm = (_adjust > 100) ? 200 : (uint16_t)_adjust * (uint16_t)_adjust / 50;
      }

    }
    else if (cBaseType == 1) { // CHARACTERISTIC_DIM2WARM

      pwmCold = (uint16_t)adjust * (uint16_t)adjust / 200;
      if (cCurve == 1) {
        pwmCold = (uint16_t)pwmCold * (uint16_t)pwmCold / 200;
      }

      if (cLevelLimit == 0) {
        uint8_t _xw = (adjust > 100) ? adjust - 100 : 100 - adjust;
        pwmWarm = ((int32_t)_xw * (int32_t)_xw / -200) + 50;
      }
      else if (cLevelLimit == 1) {
        pwmWarm = ((int32_t)_adjust * (int32_t)_adjust / -200) + 200;
      }

      if ((cLevelLimit == 0) && (cCurve == 1)) {
        pwmWarm = (uint16_t)pwmWarm * (uint16_t)adjust / 120;
      }
      else if ((cLevelLimit == 1) && (cCurve == 1)) {
        pwmWarm = (uint16_t)pwmWarm * (uint16_t)pwmWarm / 200;
      }

    }
    else if (cBaseType == 2) { // CHARACTERISTIC_DIM2HOT

      if (cCurve == 0) {
        pwmCold = (adjust < 100) ? 0 : (adjust - 100) * 2;
      }
      else if (cCurve == 1) {
        pwmCold = (adjust < 140) ? 0 : (adjust - 140) * 10 / 3;
      }

      if ((cLevelLimit == 0) && (cCurve == 0)) {
        pwmWarm = (adjust > 100) ? _adjust : adjust;
      }
      else if ((cLevelLimit == 1) && (cCurve == 0)) {
        pwmWarm = (adjust > 100) ? 200 : adjust * 2;
      }
      else if ((cLevelLimit == 0) && (cCurve == 1)) {
        uint8_t _xw = (adjust > 140) ? (uint16_t)_adjust * 5 / 3 : (uint16_t)adjust * 5 / 7;
        pwmWarm = (adjust > 140) ? ((int32_t)adjust * (int32_t)(100 - _xw) / -200) + 100 : (uint16_t)_xw * (uint16_t)_xw / 100;
      }
      else if ((cLevelLimit == 1) && (cCurve == 1)) {
        pwmWarm = (adjust > 140) ? 200 : (uint16_t)adjust * (uint16_t)adjust / 98;
      }
    }

    pwmCold = (uint16_t)pwmCold * (uint16_t)bright / (uint16_t)200;
    pwmWarm = (uint16_t)pwmWarm * (uint16_t)bright / (uint16_t)200;

    //printParam();
    //DPRINT(F("adjust ")); DPRINT(adjust); DPRINT(F(", bright ")); DPRINT(bright); DPRINT(F(", pwmCold ")); DPRINT(pwmCold); DPRINT(F(", pwmWarm ")); DPRINTLN(pwmWarm);

    // 0 xor 0 = 0; 0 xor 1 = 1; 1 xor 0 = 1; 1 xor 1 = 0
    this->pwms[0 ^ cColourAssignment].set(pwmCold);
    this->pwms[1 ^ cColourAssignment].set(pwmWarm);
  }
  
  uint8_t checkParam() {
    bool cc = this->dimmer.hasConfigChanged();
    if (cc == false) return 0;

    // speed multiplier and characteristic from channel 1 is needed in the pwm channels
    uint8_t charCurve = this->dimmer.dimmerChannel(1).getList1().characteristic();
    uint8_t speedMultiplier = this->dimmer.getList0().speedMultiplier();
    this->pwms[0].param(speedMultiplier, charCurve);
    this->pwms[1].param(speedMultiplier, charCurve);

    // charcteristic of channel 2 is needed within the dualwhite instance 
    cBaseType = this->dimmer.dimmerChannel(2).getList1().characteristicBasetype();
    cLevelLimit = this->dimmer.dimmerChannel(2).getList1().characteristicLevelLimit();
    cCurve = this->dimmer.dimmerChannel(2).getList1().characteristic();
    cColourAssignment = this->dimmer.dimmerChannel(2).getList1().characteristicColourAssignment();
    //printParam();
    return 1;
  }

  void printParam() {
    char* str_charBaseType[3] = { "CROSSFADE", "DIM2WARM ", "DIM2HOT  " };
    DPRINT(str_charBaseType[cBaseType]); DPRINT(F(": cLevelLimit ")); DPRINT(cLevelLimit); DPRINT(F(", cCurve ")); DPRINT(cCurve); DPRINT(F(", cColour ")); DPRINTLN(cColourAssignment);
  }

};

}

#endif
