//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-12-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __BLIND_H__
#define __BLIND_H__

#include "MultiChannelDevice.h"
#include "Register.h"
#include "StateMachine.h"

namespace as {

DEFREGISTER(BlindReg1,CREG_AES_ACTIVE,
    CREG_REFERENCE_RUNNING_TIME_TOP_BOTTOM,CREG_REFERENCE_RUNNING_TIME_BOTTOM_TOP,
    CREG_CHANGE_OVER_DELAY,CREG_REFERENCE_RUN_COUNTER,CREG_TRANSMITTRYMAX,
    CREG_STATUSINFO)

class BlindList1 : public RegList1<BlindReg1> {
public:
  BlindList1 (uint16_t addr) : RegList1<BlindReg1>(addr) {}
  void defaults () {
    clear();
    // aesActive(false);
    transmitTryMax(6);
    // powerUpAction(false);
    statusInfoMinDly(4);
    statusInfoRandom(1);

    refRunningTimeTopButton(500);
    refRunningTimeButtonTop(500);
    changeOverDelay(5);
    //refRunCounter(0);
  }
};

DEFREGISTER(BlindReg3,PREG_CTRAMPONOFF,PREG_CTDELAYONOFF,PREG_CTONOFF,
  PREG_CTREPONOFF,PREG_CONDVALUELOW,PREG_CONDVALUEHIGH,PREG_ONDELAYTIME,
  PREG_ONTIME,PREG_OFFDELAYTIME,PREG_OFFTIME,PREG_ACTIONTYPE,PREG_JTONOFF,
  PREG_JTDELAYONOFF,PREG_JTRAMPONOFF,PREG_JTREFONOFF,PREG_OFFLEVEL,
  PREG_ONLEVEL,PREG_MAXTIMEFIRSTDIR,PREG_DRIVINGMODE)

typedef RegList3<BlindReg3> BlindPeerList;

class BlindList3 : public ShortLongList<BlindPeerList> {
public:
  BlindList3 (uint16_t addr) : ShortLongList<BlindPeerList>(addr) {}
  void defaults() {
    BlindPeerList ssl = sh();
    ssl.clear();
//    ssl.ctRampOn(0);
//    ssl.ctRampOff(0);
//    ssl.ctDlyOn(0);
//    ssl.ctDlyOff(0);
//    ssl.ctOn(0);
//    ssl.ctOff(0);
//    ssl.ctRefOn(0);
//    ssl.ctRefOff(0);
    ssl.ctValLo(0x32);
    ssl.ctValHi(0x64);
//    ssl.onDly(0);
    ssl.onTime(0xff);
//    ssl.offDly(0);
    ssl.offTime(0xff);
    ssl.actionType(AS_CM_ACTIONTYPE_JUMP_TO_TARGET);
//    ssl.offLevel(0);
    ssl.onLevel(200); // 201 ???
    ssl.maxTimeFirstDir(255);
    //ssl.drivingMode(0);

    ssl = lg();
    ssl.clear();
//    ssl.ctRampOn(0);
//    ssl.ctRampOff(0);
//    ssl.ctDlyOn(0);
//    ssl.ctDlyOff(0);
//    ssl.ctOn(0);
//    ssl.ctOff(0);
//    ssl.ctRefOn(0);
//    ssl.ctRefOff(0);
    ssl.ctValLo(0x32);
    ssl.ctValHi(0x64);
//    ssl.onDly(0);
    ssl.onTime(0xff);
//    ssl.offDly(0);
    ssl.offTime(0xff);
    ssl.actionType(AS_CM_ACTIONTYPE_JUMP_TO_TARGET);
    ssl.multiExec(true);
//    ssl.offLevel(0);
    ssl.onLevel(200); // 201 ???
    ssl.maxTimeFirstDir(50);
    //ssl.drivingMode(0);
  }

  void odd() { // B
    defaults();
    BlindPeerList ssl = sh();
    ssl.jtOff(AS_CM_JT_OFFDELAY);
    ssl.jtDlyOff(AS_CM_JT_REFOFF);
    ssl.jtOn(AS_CM_JT_OFFDELAY);
    ssl.jtDlyOn(AS_CM_JT_OFFDELAY);
    ssl.jtRampOff(AS_CM_JT_RAMPOFF);
    ssl.jtRampOn(AS_CM_JT_ON);
    ssl.jtRefOff(AS_CM_JT_RAMPOFF);
    ssl.jtRefOn(AS_CM_JT_ON);
    ssl = lg();
    ssl.jtOff(AS_CM_JT_OFFDELAY);
    ssl.jtDlyOff(AS_CM_JT_REFOFF);
    ssl.jtOn(AS_CM_JT_OFFDELAY);
    ssl.jtDlyOn(AS_CM_JT_OFFDELAY);
    ssl.jtRampOff(AS_CM_JT_RAMPOFF);
    ssl.jtRampOn(AS_CM_JT_ON);
    ssl.jtRefOff(AS_CM_JT_RAMPOFF);
    ssl.jtRefOn(AS_CM_JT_ON);
  }

  void even() { // A
    defaults();
    BlindPeerList ssl = sh();
    ssl.jtOff(AS_CM_JT_ONDELAY);
    ssl.jtDlyOff(AS_CM_JT_ONDELAY);
    ssl.jtOn(AS_CM_JT_ONDELAY);
    ssl.jtDlyOn(AS_CM_JT_REFON);
    ssl.jtRampOff(AS_CM_JT_OFF);
    ssl.jtRampOn(AS_CM_JT_RAMPON);
    ssl.jtRefOff(AS_CM_JT_OFF);
    ssl.jtRefOn(AS_CM_JT_RAMPON);
    ssl = lg();
    ssl.jtOff(AS_CM_JT_ONDELAY);
    ssl.jtDlyOff(AS_CM_JT_ONDELAY);
    ssl.jtOn(AS_CM_JT_ONDELAY);
    ssl.jtDlyOn(AS_CM_JT_REFON);
    ssl.jtRampOff(AS_CM_JT_OFF);
    ssl.jtRampOn(AS_CM_JT_RAMPON);
    ssl.jtRefOff(AS_CM_JT_OFF);
    ssl.jtRefOn(AS_CM_JT_RAMPON);
  }

  void single() { //AB
    defaults();
    BlindPeerList ssl = sh();
    ssl.jtOff(AS_CM_JT_ONDELAY);
    ssl.jtDlyOff(AS_CM_JT_REFOFF);
    ssl.jtOn(AS_CM_JT_OFFDELAY);
    ssl.jtDlyOn(AS_CM_JT_REFON);
    ssl.jtRampOn(AS_CM_JT_ON);
    ssl.jtRefOn(AS_CM_JT_ON);
    ssl.jtRampOff(AS_CM_JT_OFF);
    ssl.jtRefOff(AS_CM_JT_ON);
    ssl = lg();
    ssl.jtOff(AS_CM_JT_ONDELAY);
    ssl.jtDlyOff(AS_CM_JT_REFOFF);
    ssl.jtOn(AS_CM_JT_OFFDELAY);
    ssl.jtDlyOn(AS_CM_JT_REFON);
    ssl.jtRampOn(AS_CM_JT_ON);
    ssl.jtRefOn(AS_CM_JT_ON);
    ssl.jtRampOff(AS_CM_JT_OFF);
    ssl.jtRefOff(AS_CM_JT_OFF);
  }

};


class BlindStateMachine : public StateMachine<BlindPeerList> {

  class LevelUpdate : public Alarm {
    BlindStateMachine&  sm;
    uint16_t  done;
    uint16_t  fulltime;
    uint8_t   startlevel;
  public:
    LevelUpdate (BlindStateMachine& m) : Alarm(0), sm(m), done(0), fulltime(0), startlevel(0) {}
    ~LevelUpdate () {}
    void start (AlarmClock& clock,uint16_t ft) {
      startlevel = sm.status();
      fulltime = ft;
      done = 0;
      set(seconds2ticks(1));
      clock.add(*this);
      sm.changed = true;
    }
    virtual void trigger (AlarmClock& clock) {
      // fulltime is 0.1s - so we add 10 for a second
      done += 10;
      if( done > fulltime ) done = fulltime;
      uint8_t dx = (done * 200UL) / fulltime;
      if( sm.state == AS_CM_JT_RAMPON ) {
        if( dx > (200-startlevel) ) dx = 200-startlevel;
        sm.updateLevel(startlevel + dx);
      }
      else if( sm.state == AS_CM_JT_RAMPOFF ) {
        if( dx > startlevel ) dx = startlevel;
        sm.updateLevel(startlevel - dx);
      }
      set(seconds2ticks(1));
      clock.add(*this);
    }
  };

public:
  void updateLevel (uint8_t l) {
    level = l;
    DDECLN(level);
    triggerChanged(decis2ticks(list1.statusInfoMinDly()*5));
  }

protected:
  uint8_t      level, destlevel;
  LevelUpdate  update;
  BlindList1   list1;

  virtual void trigger (AlarmClock& clock) {
    if( state == AS_CM_JT_RAMPON || state == AS_CM_JT_RAMPOFF ) {
      if( destlevel != 0xff ) {
        // update level to destlevel
        updateLevel(destlevel);
      }
    }
    StateMachine<BlindPeerList>::trigger(clock);
  }

public:
  BlindStateMachine () : StateMachine<BlindPeerList>(), level(0), destlevel(0), /*alarm(*this),*/ update(*this), list1(0) {}
  virtual ~BlindStateMachine () {}

  virtual void switchState(uint8_t oldstate,uint8_t newstate) {
    DPRINT("Switch from ");DHEX(oldstate);DPRINT(" to ");DHEXLN(newstate);
    switch( newstate ) {
    case AS_CM_JT_RAMPON:
      update.start(sysclock, list1.refRunningTimeButtonTop());
      break;
    case AS_CM_JT_RAMPOFF:
      update.start(sysclock, list1.refRunningTimeTopButton());
      break;
    }

    switch (oldstate) {
    case AS_CM_JT_RAMPON:
    case AS_CM_JT_RAMPOFF:
      sysclock.cancel(update);
      break;
    }
  }

  void setup(BlindList1 l1) {
    list1 = l1;
  }

  virtual uint32_t getDelayForState(uint8_t stat,const BlindPeerList& lst) {
    if( lst.valid () == true ) {
      if( stat == AS_CM_JT_RAMPON || stat == AS_CM_JT_RAMPOFF ) {
        uint8_t first = lst.maxTimeFirstDir();
        if( first != 0xff && first != 0x00 ) {
          destlevel = 0xff;
          return decis2ticks(first);
        }
        destlevel = stat == AS_CM_JT_RAMPON ? 200 : 0;
      }
    }
    return StateMachine<BlindPeerList>::getDelayForState(stat,lst);
  }

  virtual uint32_t getDefaultDelay(uint8_t stat) const {
    switch( stat ) {
      case AS_CM_JT_ON:
      case AS_CM_JT_OFF:
        return DELAY_INFINITE;

      case AS_CM_JT_REFON:
      case AS_CM_JT_REFOFF:
        return decis2ticks(list1.changeOverDelay());

      case AS_CM_JT_RAMPON:
        return calcDriveTime(destlevel-level,list1.refRunningTimeButtonTop(),destlevel==200);

      case AS_CM_JT_RAMPOFF:
        return calcDriveTime(level-destlevel,list1.refRunningTimeTopButton(),destlevel==0);

    }
    return DELAY_NO;
  }

  uint32_t calcDriveTime(uint8_t dx,uint32_t fulltime,bool extratime) const {
    uint32_t dt = (fulltime * dx) / 200;
    if( extratime == true ) dt += 20; // we add 2 additional seconds
    DPRINT("calcDriveTime: ");DDEC(fulltime);DPRINT(" - ");DDEC(dx);DPRINT(" - ");DDECLN(dt);
    return decis2ticks(dt);
  }

  void remote (const BlindPeerList& lst,uint8_t counter) {
    // perform action as defined in the list
    switch (lst.actionType()) {
    case AS_CM_ACTIONTYPE_JUMP_TO_TARGET:
      jumpToTarget(lst);
      break;
    case AS_CM_ACTIONTYPE_TOGGLE_TO_COUNTER:
      setDestLevel((counter & 0x01) == 0x01 ? 200 : 0);
      break;
    case AS_CM_ACTIONTYPE_TOGGLE_INVERSE_TO_COUNTER:
      setDestLevel((counter & 0x01) == 0x00 ? 200 : 0);
      break;
    }
  }

  void sensor (const BlindPeerList& lst,uint8_t counter,uint8_t value) {
    if( checkCondition(state,lst,value) == true ) {
      remote(lst,counter);
    }
  }

  void toggleState () {
    if( state == AS_CM_JT_OFF ) {
      setDestLevel(200);
    }
    else {
      setDestLevel(0);
    }
  }

  void setDestLevel (uint8_t value) {
    destlevel = value;
    if( destlevel > level || destlevel == 200 ) {
      setState(AS_CM_JT_ONDELAY, 0);
    }
    else if ( destlevel < level || destlevel == 0 ) {
      setState(AS_CM_JT_OFFDELAY, 0);
    }
  }

  void stop () {
    if( state == AS_CM_JT_RAMPON || state == AS_CM_JT_RAMPOFF ) {
      setState(getNextState(state),DELAY_INFINITE);
    }
  }

  uint8_t status () const {
    return level;
  }

  uint8_t flags () const {
    uint8_t f = (state == AS_CM_JT_ON || state == AS_CM_JT_OFF) ? 0x00 : AS_CM_EXTSTATE_RUNNING;
    if( state == AS_CM_JT_RAMPON ) {
      f |= AS_CM_EXTSTATE_UP; // UP flag
    }
    else if( state == AS_CM_JT_RAMPOFF ) {
      f |= AS_CM_EXTSTATE_DOWN; // DOWN flag
    }
    return f;
  }
};


template <class HalType,int PeerCount,class List0Type>
class BlindChannel : public Channel<HalType,BlindList1,BlindList3,EmptyList,PeerCount,List0Type>, public BlindStateMachine {

protected:
  typedef Channel<HalType,BlindList1,BlindList3,EmptyList,PeerCount,List0Type> BaseChannel;
  uint8_t lastmsgcnt;

public:
  BlindChannel () : BaseChannel(), lastmsgcnt(0xff) {}
  virtual ~BlindChannel() {}

  bool changed () const { return BlindStateMachine::changed; }

  void changed (bool c) { BlindStateMachine::changed = c; }

  void init () {
//    typename BaseChannel::List1 l1 = BaseChannel::getList1();
    setState(AS_CM_JT_OFF, DELAY_INFINITE);
//    changed(true);
    updateLevel(0);
  }

  void setup(Device<HalType,List0Type>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev,number,addr);
    BlindStateMachine::setup(this->getList1());
  }

  uint8_t flags () const {
    return BlindStateMachine::flags();
  }

  void stop () {
    BlindStateMachine::stop();
  }

  bool process (const ActionSetMsg& msg) {
    setDestLevel( msg.value() );
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
