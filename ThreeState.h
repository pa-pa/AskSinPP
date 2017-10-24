//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-10-19 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __THREESTATE_H__
#define __THREESTATE_H__

#include "MultiChannelDevice.h"

namespace as {

enum Positions { NoPos=0, PosA, PosB, PosC };

template <class HALTYPE,class List0Type,class List1Type,class List4Type,int PEERCOUNT>
class ThreeStateChannel : public Channel<HALTYPE,List1Type,EmptyList,List4Type,PEERCOUNT,List0Type>, public Alarm {

  class EventSender : public Alarm {
  public:
    ThreeStateChannel& channel;
    uint8_t count, state;

    EventSender (ThreeStateChannel& c) : Alarm(0), channel(c), count(0), state(255) {}
    virtual ~EventSender () {}
    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      SensorEventMsg& msg = (SensorEventMsg&)channel.device().message();
      msg.init(channel.device().nextcount(),channel.number(),count++,state,channel.device().battery().low());
      channel.device().sendPeerEvent(msg,channel);
    }
  };

  // pin mapping can be changed by bootloader config data
  // map pins to pos     00   01   10   11
  uint8_t posmap[4] = {Positions::PosC,Positions::PosC,Positions::PosB,Positions::PosA};
  volatile bool isr;
  EventSender sender;
  uint8_t sens1, sens2, sabpin;
  bool sabotage;

  public:
    typedef Channel<HALTYPE,List1Type,EmptyList,List4Type,PEERCOUNT,List0Type> BaseChannel;

  ThreeStateChannel () : BaseChannel(), Alarm(0), isr(false), sender(*this), sens1(0), sens2(0), sabpin(0), sabotage(false) {}
  virtual ~ThreeStateChannel () {}

  void setup(Device<HALTYPE,List0Type>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev,number,addr);
  }

  void init (uint8_t pin1,uint8_t pin2, uint8_t sab, const uint8_t* pmap) {
    sabpin = sab;
    init(pin1,pin2,pmap);
  }

  void init (uint8_t pin1,uint8_t pin2, const uint8_t* pmap) {
    memcpy(posmap,pmap,4);
    init(pin1,pin2);
  }

  void init (uint8_t pin1,uint8_t pin2, uint8_t sab) {
    sabpin = sab;
    init(pin1,pin2);
  }

  void init (uint8_t pin1,uint8_t pin2) {
    sens1=pin1;
    sens2=pin2;
    // start polling
    set(seconds2ticks(1));
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
    set(seconds2ticks(1));
    clock.add(*this);
    uint8_t newstate = sender.state;
    uint8_t pinstate = readPin(sens2) << 1 | readPin(sens1);
    uint8_t pos = posmap[pinstate & 0x03];
    uint8_t msg = 0;
    switch( pos ) {
    case PosA:
      msg = this->getList1().msgForPosA();
      break;
    case PosB:
      msg = this->getList1().msgForPosB();
      break;
    case PosC:
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
        sender.set(seconds2ticks(delay));
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

template<class HalType,class ChannelType,int ChannelCount,class List0Type>
class ThreeStateDevice : public MultiChannelDevice<HalType,ChannelType,ChannelCount,List0Type> {
  #define CYCLETIME seconds2ticks(60UL*60*24) // at least one message per day
  class CycleInfoAlarm : public Alarm {
    ThreeStateDevice& dev;
  public:
    CycleInfoAlarm (ThreeStateDevice& d) : Alarm (CYCLETIME), dev(d) {}
    virtual ~CycleInfoAlarm () {}

    void trigger (AlarmClock& clock)  {
      set(CYCLETIME);
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
      cycle.set(CYCLETIME);
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
