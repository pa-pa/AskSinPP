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
  uint8_t       peerself;
  uint8_t       extpeer;
  volatile bool isr;

public:

  typedef Channel<HALTYPE,RemoteList1,EmptyList,DefList4,PEERCOUNT,List0Type> BaseChannel;

  RemoteChannel () : BaseChannel(), repeatcnt(0), peerself(false), extpeer(0xff), isr(false) {}
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
      // send one message to all peers
      msg.clearAck();
      msg.setBroadcast();
      if( peerself == true ) {
        // simply process by yourself
        this->device().process(msg);
      }
      // send out to one peer
      this->device().send(msg,extpeer != 0xff ? this->peer(extpeer) : this->device().getMasterID());
    }
  }

  void configChanged () {
    peerself = false;
    extpeer = 0xff;
    for( int i=0; i<this->peers(); ++i ){
      Peer p = this->peer(i);
      if( p.valid() == true ) {
        if( this->device().isDeviceID(p) == true ) {
          peerself |=  true;
        }
        else {
          extpeer = i; // store offset to an external peer
        }
      }
    }
  }

  void pinchanged () {
    isr = true;
  }

  bool checkpin () {
    bool result = isr;
    if( isr == true ) {
      isr = false;
      this->check();
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
