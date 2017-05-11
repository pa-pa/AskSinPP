//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-04-12 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __REMOTE_H__
#define __REMOTE_H__

#include "Channel.h"
#include "ChannelList.h"
#include "Button.h"
#include "Message.h"

namespace as {

class RemoteList1Data {
public:
  uint8_t Unused1          : 4;     // 0x04
  uint8_t LongPressTime    : 4;     // 0x04
  uint8_t AesActive        : 1;     // 0x08, s:0, e:1
  uint8_t DoublePressTime  : 4;     // 0x09
  uint8_t Unused2          : 4;     // 0x09

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x04: return 0;
      case 0x08: return 1;
      case 0x09: return 2;
      default: break;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x04;
      case 1:  return 0x08;
      case 2:  return 0x09;
      default: break;
    }
    return 0xff;
  }
};

class RemoteList1 : public ChannelList<RemoteList1Data> {
public:
  RemoteList1(uint16_t a) : ChannelList(a) {}

  uint8_t longPressTime () const { return getByte(0,0xf0,4); }
  bool longPressTime (uint8_t value) const { return setByte(0,value,0xf0,4); }

  bool aesActive () const { return isBitSet(1,0x01); }
  bool aesActive (bool s) const { return setBit(1,0x01,s); }

  uint8_t doublePressTime () const { return getByte(2,0x0f,0); }
  bool doublePressTime (uint8_t value) const { return setByte(2,value,0x0f,0); }


  void defaults () {
    longPressTime(1);
    aesActive(false);
    doublePressTime(0);
  }
};

template<class HALTYPE,int PEERCOUNT>
class RemoteChannel : public Channel<HALTYPE,RemoteList1,EmptyList,List4,PEERCOUNT>, public Button {

private:
  uint8_t       repeatcnt;
  volatile bool isr;

public:

  typedef Channel<HALTYPE,RemoteList1,EmptyList,List4,PEERCOUNT> BaseChannel;

  RemoteChannel () : BaseChannel(), repeatcnt(0), isr(false) {}
  virtual ~RemoteChannel () {}

  Button& button () { return *(Button*)this; }

  uint8_t status () const {
    return 0;
  }

  uint8_t flags () const {
    return 0;
  }

  virtual void state(uint8_t s) {
    DHEX(BaseChannel::number());
    Button::state(s);
    if( s == released ) {
      RemoteEventMsg& msg = (RemoteEventMsg&)BaseChannel::device().message();
      msg.init(BaseChannel::device().nextcount(),BaseChannel::number(),repeatcnt++,false,BaseChannel::device().battery().low());
      BaseChannel::device().sendPeerEvent(msg,*this);
    }
    else if( s == longpressed ) {
      RemoteEventMsg& msg = (RemoteEventMsg&)BaseChannel::device().message();
      msg.init(BaseChannel::device().nextcount(),BaseChannel::number(),repeatcnt,true,BaseChannel::device().battery().low());
      BaseChannel::device().sendPeerEvent(msg,*this);
    }
    else if( s == longreleased ) {
      repeatcnt++;
    }
  }

  void pinchanged () {
    isr = true;
  }

  bool checkpin () {
    bool result = isr;
    if( isr == true ) {
      isr = false;
      Button::check();
    }
    return result;
  }
};

#define remoteISR(device,chan,pin) class device##chan##ISRHandler { \
  public: \
  static void isr () { device.channel(chan).pinchanged(); } \
}; \
device.channel(chan).button().init(pin); \
enableInterrupt(pin,device##chan##ISRHandler::isr,CHANGE);

#define remoteChannelISR(chan,pin) class __##pin##ISRHandler { \
    public: \
    static void isr () { chan.pinchanged(); } \
  }; \
  chan.button().init(pin); \
  enableInterrupt(pin,__##pin##ISRHandler::isr,CHANGE);


}

#endif
