//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-10-19 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __THREESTATE_H__
#define __THREESTATE_H__

#include "MultiChannelDevice.h"
#include "Sensors.h"

namespace as {

template <class Sensor,class HALTYPE,class List0Type,class List1Type,class List4Type,int PEERCOUNT>
class ThreeStateGenericChannel : public Channel<HALTYPE,List1Type,EmptyList,List4Type,PEERCOUNT,List0Type>, public Alarm {

  class EventSender : public Alarm {
  public:
    ThreeStateGenericChannel& channel;
    uint8_t count, state;

    EventSender (ThreeStateGenericChannel& c) : Alarm(0), channel(c), count(0), state(255) {}
    virtual ~EventSender () {}
    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      SensorEventMsg& msg = (SensorEventMsg&)channel.device().message();
      msg.init(channel.device().nextcount(),channel.number(),count++,state,channel.device().battery().low());
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

  ThreeStateGenericChannel () : BaseChannel(), Alarm(0), sender(*this), sabpin(0), sabotage(false) {}
  virtual ~ThreeStateGenericChannel () {}

  void setup(Device<HALTYPE,List0Type>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev,number,addr);
  }

  void init (uint8_t sab) {
    sabpin = sab;
    init();
  }

  void init () {
    // start polling
    set(possens.interval());
    sysclock.add(*this);
  }

  uint8_t status () const {
    return sender.state;
  }

  uint8_t flags () const {
    uint8_t flags = sabotage ? 0x07 << 1 : 0x00;
    flags |= this->device().battery().low() ? 0x80 : 0x00;
    return flags;
  }

  uint8_t readPin(uint8_t pinnr) {
    uint8_t value=0;
    pinMode(pinnr,INPUT_PULLUP);
    value = digitalRead(pinnr);
    pinMode(pinnr,OUTPUT);
    digitalWrite(pinnr,LOW);
    return value;
  }

  void trigger (__attribute__ ((unused)) AlarmClock& clock)  {
    set(possens.interval());
    clock.add(*this);
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
      bool sabstate = readPin(sabpin) == LOW;
      if( sabotage != sabstate && this->device().getList0().sabotageMsg() == true ) {
        sabotage = sabstate;
        this->changed(true); // trigger StatusInfoMessage to central
      }
    }
  }
};

class TwoPinPosition : public Position {
  // pin mapping can be changed by bootloader config data
  // map pins to pos     00   01   10   11
  uint8_t posmap[4] = {State::PosC,State::PosC,State::PosB,State::PosA};
  uint8_t sens1, sens2;
public:
  TwoPinPosition () : sens1(0), sens2(0) { _present = true; }

  void init (uint8_t pin1,uint8_t pin2, const uint8_t* pmap) {
    memcpy(posmap,pmap,4);
    init(pin1,pin2);
  }

  void init (uint8_t pin1,uint8_t pin2) {
    sens1=pin1;
    sens2=pin2;
  }

  void measure (__attribute__((unused)) bool async=false) {
    uint8_t pinstate = readPin(sens2) << 1 | readPin(sens1);
    _position = posmap[pinstate & 0x03];
  }

  uint8_t readPin(uint8_t pinnr) {
    uint8_t value=0;
    pinMode(pinnr,INPUT_PULLUP);
    value = digitalRead(pinnr);
    pinMode(pinnr,OUTPUT);
    digitalWrite(pinnr,LOW);
    return value;
  }
};


template <class HALTYPE,class List0Type,class List1Type,class List4Type,int PEERCOUNT>
class ThreeStateChannel : public ThreeStateGenericChannel<TwoPinPosition,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> {
public:
  typedef ThreeStateGenericChannel<TwoPinPosition,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> BaseChannel;

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



#define DEFCYCLETIME seconds2ticks(60UL*60*20)
template<class HalType,class ChannelType,int ChannelCount,class List0Type,uint32_t CycleTime=DEFCYCLETIME> // at least one message per day
class ThreeStateDevice : public MultiChannelDevice<HalType,ChannelType,ChannelCount,List0Type> {
  class CycleInfoAlarm : public Alarm {
    ThreeStateDevice& dev;
  public:
    CycleInfoAlarm (ThreeStateDevice& d) : Alarm (CycleTime), dev(d) {}
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
  ThreeStateDevice(const DeviceInfo& info,uint16_t addr) : DevType(info,addr), cycle(*this) {}
  virtual ~ThreeStateDevice () {}

  virtual void configChanged () {
    // activate cycle info message
    if( this->getList0().cycleInfoMsg() == true ) {
      DPRINTLN("Activate Cycle Msg");
      sysclock.cancel(cycle);
      cycle.set(CycleTime);
      sysclock.add(cycle);
    }
    else {
      DPRINTLN("Deactivate Cycle Msg");
      sysclock.cancel(cycle);
    }
  }
};

}

#endif
