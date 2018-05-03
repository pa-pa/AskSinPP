//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-05-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __IBUTTON_H__
#define __IBUTTON_H__

#include "MultiChannelDevice.h"
#include "Register.h"

namespace as {

DEFREGISTER(IButtonReg1,CREG_AES_ACTIVE,0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7)
class IButtonList1 : public RegList1<IButtonReg1> {
public:
  IButtonList1 (uint16_t addr) : RegList1<IButtonReg1>(addr) {}
  void defaults () {
    clear();
  }
};

template<class HALTYPE,int PEERCOUNT,class List0Type=List0>
class IButtonChannel : public Channel<HALTYPE,IButtonList1,EmptyList,DefList4,PEERCOUNT,List0Type>, Alarm {

  enum { none=0, released, longpressed, longreleased };

  uint8_t state, matches, repeatcnt;

public:
  typedef Channel<HALTYPE,IButtonList1,EmptyList,DefList4,PEERCOUNT,List0Type> BaseChannel;

  IButtonChannel () : BaseChannel(), Alarm(0), state(0), matches(0),repeatcnt(0) {}
  virtual ~IButtonChannel () {}

  virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
    state = 0;
    this->changed(true);
  }

  uint8_t status () const {
    return state;
  }

  uint8_t flags () const {
    return 0;
  }

  bool match (uint8_t* addr) {
    matches <<= 1;
    uint8_t s = none;
    if( free() == false && isID(addr) == true ) {
      matches |= 0b00000001;
      // 3 or 6 matches are longpress and longlongpress
      if( (matches & 0b00111111) == 0b00000111 || (matches & 0b00111111) == 0b00111111 ) {
        s = longpressed;
        DPRINTLN("longpressed");
        // clear longlong
        matches &= 0b11000111;
      }
    }
    // check for long release
    if( (matches & 0b00001111) == 0b00001110 ) {
      s = longreleased;
      DPRINTLN("longreleased");
    }
    // check for release
    else if( (matches & 0b00000011) == 0b00000010 ) {
      s = released;
      DPRINTLN("released");
    }
    if( s != none ) {
      RemoteEventMsg& msg = (RemoteEventMsg&)this->device().message();
      msg.init(this->device().nextcount(),this->number(),repeatcnt,(s==longreleased || s==longpressed),this->device().battery().low());
      if( s == released || s == longreleased) {
        // send the message to every peer
        this->device().sendPeerEvent(msg,*this);
        repeatcnt++;
      }
      else if (s == longpressed) {
        // broadcast the message
        this->device().broadcastPeerEvent(msg,*this);
      }
    }
    return (matches & 0b00000001) == 0b00000001;
  }

  bool isID (uint8_t* buf) {
    IButtonList1 l = this->getList1();
    for( uint8_t n=0; n<8; ++n ) {
      if( l.readRegister(0xe0+n) != buf[n] ) {
        return false;
      }
    }
    return true;
  }

  void storeID (uint8_t* buf) {
    if( learn() == true ) {
      for( uint8_t n=0; n<8; ++n ) {
        this->getList1().writeRegister(0xe0+n,buf[n]);
      }
      state = 0;
      this->changed(true);
      sysclock.cancel(*this);
    }
  }

  bool free () {
    return this->getList1().readRegister(0xe0) == 0;
  }

  bool learn () const {
    return state == 200;
  }

  bool process (const ActionSetMsg& msg) {
    state = msg.value();
    this->changed(true);
    if( state != 0 ) {
      set(seconds2ticks(60));
      sysclock.add(*this);
    }
    return true;
  }

  bool process (__attribute__((unused)) const RemoteEventMsg& msg) { return false; }
  bool process (__attribute__((unused)) const SensorEventMsg& msg) { return false; }

};


template <class IButtonDev,class IButtonChannel,int READER_PIN,int LED_GREEN,int LED_RED>
class IButtonScanner : public Alarm {
  OneWire   ow;
  IButtonDev& dev;
  DualStatusLed<LED_GREEN,LED_RED> led;
  uint8_t cnt;
public:
  IButtonScanner (IButtonDev& d) : Alarm(millis2ticks(500)), ow(READER_PIN), dev(d), cnt(0) {
    led.init();
  }
  virtual ~IButtonScanner () {}

  IButtonChannel* learning () {
    for( uint8_t i=0; i<dev.ibuttonCount(); ++i ) {
      IButtonChannel& bc = dev.ibuttonChannel(i);
      if( bc.learn() == true ) {
        return &bc;
      }
    }
    return 0;
  }

  IButtonChannel* matches (uint8_t* addr) {
    for( uint8_t i=0; i<dev.ibuttonCount(); ++i ) {
      IButtonChannel& bc = dev.ibuttonChannel(i);
      if( bc.match(addr) == true ) {
        return &bc;
      }
    }
    return 0;
  }

  bool scan (uint8_t* addr) {
    ow.reset_search();
    if ( ow.search(addr) == false || OneWire::crc8(addr, 7) != addr[7] ) {
      memset(addr,0,8);
      return false;
    }
    return true;
  }

  void trigger (AlarmClock& clock) {
    // reactivate
    set(millis2ticks(250));
    clock.add(*this);
    ++cnt;
    // check if we have a learning channel
    IButtonChannel* lc = learning();
    if( lc != 0 ) {
      uint8_t cycle = cnt & 0x01;
      led.ledOn(cycle == 0 ? tick : 0, cycle == 0 ? 0 : tick);
    }
    // scan the bus now
    uint8_t addr[8];
    bool found = scan(addr);
    // search matching channel
    IButtonChannel* match = matches(addr);
    if( found == true ) {
      if( lc != 0 ) {
        clock.cancel(*this);
        set(seconds2ticks(5));
        led.ledOff();
        led.ledOn(tick);
        clock.add(*this);
        lc->storeID(addr);
      }
      else {
        if( match != 0 ) {
          led.ledOn(millis2ticks(500),0);
        }
        else {
          led.ledOn(0,millis2ticks(500));
        }
      }
    }
  }
};

}

#endif
