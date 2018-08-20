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
#include "Register.h"

namespace as {

DEFREGISTER(RemoteReg1,CREG_LONGPRESSTIME,CREG_AES_ACTIVE,CREG_DOUBLEPRESSTIME)
class RemoteList1 : public RegList1<RemoteReg1> {
public:
  RemoteList1 (uint16_t addr) : RegList1<RemoteReg1>(addr) {}
  void defaults () {
    clear();
    longPressTime(1);
    // aesActive(false);
    // doublePressTime(0);
  }
};

template<class HALTYPE,int PEERCOUNT,class List0Type=List0>
class RemoteChannel : public Channel<HALTYPE,RemoteList1,EmptyList,DefList4,PEERCOUNT,List0Type>, public Button {

private:
  uint8_t       repeatcnt;
  volatile bool isr;

public:

  typedef Channel<HALTYPE,RemoteList1,EmptyList,DefList4,PEERCOUNT,List0Type> BaseChannel;

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

  uint8_t state() const {
    return Button::state();
  }

  bool pressed () const {
    uint8_t s = state();
    return s == Button::pressed || s == Button::debounce || s == Button::longpressed;
  }
  
  bool configChanged() {
    //we have to add 300ms to the value set in CCU!
    uint16_t _longpressTime = 300 + (this->getList1().longPressTime() * 100);
    //DPRINT("longpressTime = ");DDECLN(_longpressTime);
    setLongPressTime(millis2ticks(_longpressTime));
  } 
};

#define remoteISR(device,chan,pin) class device##chan##ISRHandler { \
  public: \
  static void isr () { device.channel(chan).irq(); } \
}; \
device.channel(chan).button().init(pin); \
if( digitalPinToInterrupt(pin) == NOT_AN_INTERRUPT ) \
  enableInterrupt(pin,device##chan##ISRHandler::isr,CHANGE); \
else \
  attachInterrupt(digitalPinToInterrupt(pin),device##chan##ISRHandler::isr,CHANGE);

#define remoteChannelISR(chan,pin) class __##pin##ISRHandler { \
  public: \
  static void isr () { chan.irq(); } \
}; \
chan.button().init(pin); \
if( digitalPinToInterrupt(pin) == NOT_AN_INTERRUPT ) \
  enableInterrupt(pin,__##pin##ISRHandler::isr,CHANGE); \
else \
  attachInterrupt(digitalPinToInterrupt(pin),__##pin##ISRHandler::isr,CHANGE);

}

#endif
