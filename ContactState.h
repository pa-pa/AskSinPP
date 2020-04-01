//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-10-19 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __CONTACTSTATE_H__
#define __CONTACTSTATE_H__


#include "MultiChannelDevice.h"
#include "sensors/PinPosition.h"

#ifndef SABOTAGE_ACTIVE_STATE
#define SABOTAGE_ACTIVE_STATE LOW
#endif

namespace as {

template <class Sensor,class HALTYPE,class List0Type,class List1Type,class List4Type,int PEERCOUNT>
class StateGenericChannel : public Channel<HALTYPE,List1Type,EmptyList,List4Type,PEERCOUNT,List0Type>, public Alarm {

  class EventSender : public Alarm {
  public:
    StateGenericChannel& channel;
    uint8_t count, state;

    EventSender (StateGenericChannel& c) : Alarm(0), channel(c), count(0), state(255) {}
    virtual ~EventSender () {}
    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      SensorEventMsg& msg = (SensorEventMsg&)channel.device().message();
      msg.init(channel.device().nextcount(),channel.number(),count++,state,channel.device().battery().low());
#ifdef CONTACT_STATE_WITH_BATTERY
      msg.append(channel.device().battery().current());
      // msg.append(__gb_BatCount);
#endif
      channel.device().sendPeerEvent(msg,channel);
    }
  };

  EventSender sender;
  uint8_t sabpin;
  bool sabotage;

protected:
  Sensor possens;

public:
  typedef Channel<HALTYPE,List1Type,EmptyList,List4Type,PEERCOUNT,List0Type> BaseChannel;

  StateGenericChannel () : BaseChannel(), Alarm(0), sender(*this), sabpin(0), sabotage(false) {}
  virtual ~StateGenericChannel () {}

  void setup(Device<HALTYPE,List0Type>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev,number,addr);
  }

  void init (uint8_t sab) {
    sabpin = sab;
    init();
  }

  void init () {
    // start polling
    if( possens.interval() > 0) {
      set(possens.interval());
      sysclock.add(*this);
    }
  }

  uint8_t status () const {
    return sender.state;
  }

  uint8_t flags () const {
    uint8_t flags = sabotage ? 0x07 << 1 : 0x00;
    flags |= this->device().battery().low() ? 0x80 : 0x00;
    return flags;
  }

  void trigger (__attribute__ ((unused)) AlarmClock& clock)  {
    if( possens.interval() > 0) {
      set(possens.interval());
      clock.add(*this);
    }
    uint8_t newstate = sender.state;
    uint8_t msg = 0;
    possens.measure();
    switch( possens.position() ) {
    case Sensor::State::PosA:
      msg = this->getList1().msgForPosA();
      break;
    case Sensor::State::PosB:
      msg = this->getList1().msgForPosB();
      break;
    case Sensor::State::PosC:
      msg = this->getList1().msgForPosC();
      break;
    default:
      break;
    }

    if( msg == 1) newstate = 0;
    else if( msg == 2) newstate = 200;
    else if( msg == 3) newstate = 100;

    if( newstate != sender.state ) {
      uint8_t delay = this->getList1().eventDelaytime();
      sender.state = newstate;
      sysclock.cancel(sender);
      if( delay == 0 ) {
        sender.trigger(sysclock);
      }
      else {
        sender.set(AskSinBase::byteTimeCvtSeconds(delay));
        sysclock.add(sender);
      }
      uint16_t ledtime = (uint16_t)this->getList1().ledOntime() * 5;
      if( ledtime > 0 ) {
        this->device().led().ledOn(millis2ticks(ledtime),0);
      }
    }
    if( sabpin != 0 ) {
      bool sabstate = (AskSinBase::readPin(sabpin) == SABOTAGE_ACTIVE_STATE);
      if( sabotage != sabstate && this->device().getList0().sabotageMsg() == true ) {
        sabotage = sabstate;
        this->changed(true); // trigger StatusInfoMessage to central
      }
    }
  }
};

// alias for old code
template <class Sensor,class HALTYPE,class List0Type,class List1Type,class List4Type,int PEERCOUNT>
using ThreeStateGenericChannel = StateGenericChannel<Sensor,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT>;

template <class HALTYPE,class List0Type,class List1Type,class List4Type,int PEERCOUNT>
class ThreeStateChannel : public StateGenericChannel<TwoPinPosition,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> {
public:
  typedef StateGenericChannel<TwoPinPosition,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> BaseChannel;

  ThreeStateChannel () : BaseChannel() {};
  ~ThreeStateChannel () {}

  void init (uint8_t pin1,uint8_t pin2, uint8_t sab,const uint8_t* pmap) {
    BaseChannel::init(sab);
    BaseChannel::possens.init(pin1,pin2,pmap);
  }

  void init (uint8_t pin1,uint8_t pin2, const uint8_t* pmap) {
    BaseChannel::init();
    BaseChannel::possens.init(pin1,pin2,pmap);
  }

  void init (uint8_t pin1,uint8_t pin2, uint8_t sab) {
    BaseChannel::init(sab);
    BaseChannel::possens.init(pin1,pin2);
  }

  void init (uint8_t pin1,uint8_t pin2) {
    BaseChannel::init();
    BaseChannel::possens.init(pin1,pin2);
  }
};

template <class HALTYPE,class List0Type,class List1Type,class List4Type,int PEERCOUNT, uint16_t WAITMILLIS_AFTER_ENABLE=0>
class TwoStateChannel : public StateGenericChannel<OnePinPosition<WAITMILLIS_AFTER_ENABLE>,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> {
public:
  typedef StateGenericChannel<OnePinPosition<WAITMILLIS_AFTER_ENABLE>,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> BaseChannel;

  TwoStateChannel () : BaseChannel() {};
  ~TwoStateChannel () {}

  void init (uint8_t pin, uint8_t en, uint8_t sab) {
    BaseChannel::init(sab);
    BaseChannel::possens.init(pin, en);
  }

  void init (uint8_t pin, uint8_t sab) {
    BaseChannel::init(sab);
    BaseChannel::possens.init(pin, 0);
  }

  void init (uint8_t pin) {
    BaseChannel::init();
    BaseChannel::possens.init(pin, 0);
  }
};

#define DEFCYCLETIME seconds2ticks(60UL*60*20)
template<class HalType,class ChannelType,int ChannelCount,class List0Type,uint32_t CycleTime=DEFCYCLETIME> // at least one message per day
class StateDevice : public MultiChannelDevice<HalType,ChannelType,ChannelCount,List0Type> {
  class CycleInfoAlarm : public Alarm {
    StateDevice& dev;
  public:
    CycleInfoAlarm (StateDevice& d) : Alarm (CycleTime), dev(d) {}
    virtual ~CycleInfoAlarm () {}

    void trigger (AlarmClock& clock)  {
      set(CycleTime);
      clock.add(*this);
      for( uint8_t idx=1; idx<=dev.channels(); ++idx) {
        dev.channel(idx).changed(true); // force StatusInfoMessage to central
      }
    }
  } cycle;
public:
  typedef MultiChannelDevice<HalType,ChannelType,ChannelCount,List0Type> DevType;
  StateDevice(const DeviceInfo& info,uint16_t addr) : DevType(info,addr), cycle(*this) {}
  virtual ~StateDevice () {}

  virtual void configChanged () {
    // activate cycle info message
    if( this->getList0().cycleInfoMsg() == true ) {
      DPRINTLN(F("Activate Cycle Msg"));
      sysclock.cancel(cycle);
      cycle.set(CycleTime);
      sysclock.add(cycle);
    }
    else {
      DPRINTLN(F("Deactivate Cycle Msg"));
      sysclock.cancel(cycle);
    }
  }
};

// alias for old code
template<class HalType,class ChannelType,int ChannelCount,class List0Type,uint32_t CycleTime=DEFCYCLETIME>
using ThreeStateDevice = StateDevice<HalType,ChannelType,ChannelCount,List0Type,CycleTime>;


#define contactISR(pin,func) if( digitalPinToInterrupt(pin) == NOT_AN_INTERRUPT ) \
  enableInterrupt(pin,func,CHANGE); \
else \
  attachInterrupt(digitalPinToInterrupt(pin),func,CHANGE);

}

#endif
