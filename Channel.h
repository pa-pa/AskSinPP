//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "Peer.h"
#include "Storage.h"

namespace as {

template <class HalType,class List0Type> class Device;
class ActionSetMsg;
class ActionCommandMsg;
class RemoteEventMsg;
class SensorEventMsg;

template<class HalType,class List1Type,class List3Type,class List4Type,int PeerCount,class List0Type=List0>
class Channel {
  Device<HalType,List0Type>*   dev;
  bool      change : 1; // the status is changed, we may need to send a status
  bool      inhi   : 1;
  uint8_t   num   ; // channels per device
  uint16_t  addr  ; // start address in eeprom

public:
  typedef List1Type List1;
  typedef List3Type List3;
  typedef List4Type List4;
  typedef Device<HalType,List0Type> DeviceType;

  public:
  Channel () : dev(0), change(false), inhi(false), num(0), addr(0) {}

  DeviceType& device () { return *dev; }

  const DeviceType& device () const { return *dev; }

  uint8_t number () const { return num; }

  uint16_t address () const { return addr; }

  uint8_t peers () const { return PeerCount; }

  bool changed () const { return change; }

  void changed (bool c) { change = c; }

  void inhibit (bool value) { inhi = value; }

  bool inhibit () const { return inhi; }

  bool aesActive () const { return getList1().aesActive(); }

  void setup(Device<HalType,List0Type>* dev,uint8_t number,uint16_t addr) {
    this->dev = dev;
    this->num = number;
    this->addr = addr;
  }

  uint16_t size () const {
    uint16_t size = sizeof(Peer);
    size += List3::size() + List4::size();
    size *= PeerCount;
    size += List1::size();
    return size;
  }

  bool has (const Peer& p) const {
    for( uint8_t i=0; i<peers(); ++i ) {
      if( p == peer(i) ) {
        return true;
      }
    }
    return false;
  }

  uint8_t peerfor (const HMID& hmid) const {
    for( uint8_t i=0; i<peers(); ++i ) {
      if( hmid == peer(i) ) {
        return i;
      }
    }
    return 0xff;
  }

  Peer peer (uint8_t idx) const {
    Peer result;
    uint16_t paddr = peerAddress(idx);
    if( paddr != 0 ) {
      storage().getData(paddr,&result);
    }
    return result;
  }


  bool peer (const Peer& p) {
    deletepeer(p);
    uint8_t pidx = findpeer();
    if( pidx != 0xff ) {
      storage().setData(peerAddress(pidx),p);
      getList3(pidx).single();
      return true;
    }
    return false;
  }

  bool peer (const Peer& p1,const Peer& p2) const {
    deletepeer(p1);
    deletepeer(p2);
    uint8_t pidx1 = findpeer();
    if( pidx1 != 0xff ) {
      storage().setData(peerAddress(pidx1),p1);
      uint8_t pidx2 = findpeer();
      if( pidx2 != 0xff ) {
        storage().setData(peerAddress(pidx2),p2);
        if( p1.odd() == true ) {
          getList3(pidx1).odd();
          getList3(pidx2).even();
        }
        else {
          getList3(pidx2).odd();
          getList3(pidx1).even();
        }
        return true;
      }
      else {
        // free already stored data
        deletepeer(p1);
      }
    }
    return false;
  }

  bool deletepeer (uint8_t idx) const {
    return storage().setData(peerAddress(idx),Peer());
  }

  uint8_t findpeer () const {
    for( int i=0; i<peers(); ++i ) {
      if( peer(i).valid()==false ) {
        return i;
      }
    }
    return 0xff;
  }

  bool deletepeer (const Peer& p) const {
    for( uint8_t i=0; i<peers(); ++i ) {
      if( peer(i) == p ) {
        deletepeer(i);
      }
    }
    return true;
  }

  void firstinit () {
    storage().clearData(address(),size());
    List1Type cl = getList1();
    cl.defaults();
  }

  List1Type getList1 () const {
    // we start always with list1
    return List1Type(address());
  }

  List3Type getList3 (const Peer& p) const {
    uint16_t liststart = 0x00;
    if( hasList3() == true ) {
      for( uint8_t i=0; i<peers(); ++i ) {
        if( peer(i) == p ) {
          liststart = peerAddress(i) + sizeof(Peer);
          break;
        }
      }
    }
    return List3Type(liststart);
  }

  List4Type getList4 (const Peer& p) const {
    uint16_t liststart = 0x00;
    if( hasList4() == true ) {
      for( uint8_t i=0; i<peers(); ++i ) {
        if( peer(i) == p ) {
          liststart = peerAddress(i) + sizeof(Peer) + List3::size();
          break;
        }
      }
    }
    return List4Type(liststart);
  }

  List3Type getList3 (uint8_t pidx) const {
    uint16_t liststart = 0x00;
    if( hasList3() == true && pidx < peers() ) {
      liststart = peerAddress(pidx) + sizeof(Peer);
    }
    return List3Type(liststart);
  }

  List4Type getList4 (uint8_t pidx) const {
    uint16_t liststart = 0x00;
    if( hasList4() == true && pidx < peers() ) {
      liststart = peerAddress(pidx) + sizeof(Peer) + List3::size();
    }
    return List4Type(liststart);
  }

  static bool hasList3 () {
    return List3Type::size() > 0;
  }

  static bool hasList4 () {
    return List4Type::size() > 0;
  }

  void stop () {}

  bool process (__attribute__((unused)) const ActionSetMsg& msg) {
    return false;
  }

  bool process (__attribute__((unused)) const ActionCommandMsg& msg) {
    return false;
  }
 
  bool process (__attribute__((unused)) const RemoteEventMsg& msg) {
    return false;
  }

  bool process (__attribute__((unused)) const SensorEventMsg& msg) {
    return false;
  }

  void patchStatus (__attribute__((unused)) Message& msg) {}

  void configChanged () {}

  protected:
  uint16_t peerAddress (uint8_t pidx) const {
    if( pidx < PeerCount ) {
      uint16_t offset = sizeof(Peer);
      offset += List3::size() + List4::size();
      offset *= pidx;
      offset += List1::size();
      return addr + offset;
    }
    return 0x00;
  }
};

template <class HalType,class List1Type,class List3Type,int PeerCount,class List0Type,class StateMachine>
class ActorChannel : public Channel<HalType,List1Type,List3Type,EmptyList,PeerCount,List0Type>, public StateMachine {
public:
  typedef Channel<HalType,List1Type,List3Type,EmptyList,PeerCount,List0Type> BaseChannel;
  uint8_t lastmsgcnt;

public:
  ActorChannel () : BaseChannel(), lastmsgcnt(0xff) {}
  ~ActorChannel() {}

  bool changed () const { return StateMachine::changed(); }
  void changed (bool c) { StateMachine::changed(c); }

  void setup(Device<HalType,List0Type>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev,number,addr);
    StateMachine::setup(this->getList1());
  }

  uint8_t flags () const {
    return StateMachine::flags();
  }

  uint8_t status () const {
    return StateMachine::status();
  }

  void stop () {
    StateMachine::stop();
  }

  bool process (__attribute__((unused)) const ActionCommandMsg& msg) {
    return true;
  }

  bool process (const ActionSetMsg& msg) {
    StateMachine::set( msg.value(), msg.ramp(), msg.delay() );
    return true;
  }

  bool process (const RemoteEventMsg& msg) {
    bool lg = msg.isLong();
    Peer p(msg.peer());
    uint8_t cnt = msg.counter();
    List3Type l3 = BaseChannel::getList3(p);
    if( l3.valid() == true ) {
      // l3.dump();
      typename List3Type::PeerList pl = lg ? l3.lg() : l3.sh();
      // pl.dump();
      if( lg == false || cnt != lastmsgcnt || pl.multiExec() == true ) {
        lastmsgcnt = cnt;
        StateMachine::remote(pl,cnt);
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
    List3Type l3 = BaseChannel::getList3(p);
    if( l3.valid() == true ) {
      // l3.dump();
      typename List3Type::PeerList pl = lg ? l3.lg() : l3.sh();
      // pl.dump();
      StateMachine::sensor(pl,cnt,value);
      return true;
    }
    return false;
  }
};


template <class HalType,class List0Type=List0>
class VirtBaseChannel {
public:
  VirtBaseChannel () {}
  virtual ~VirtBaseChannel () {}

  virtual void setup(Device<HalType,List0Type>* dev,uint8_t number,uint16_t addr) = 0;
  virtual uint16_t size () const = 0;
  virtual uint8_t number () const = 0;
  virtual uint16_t address () const = 0;
  virtual uint8_t peers () const = 0;
  virtual bool changed () const = 0;
  virtual void changed (bool c) = 0;
  virtual void inhibit (bool value) = 0;
  virtual bool inhibit () const = 0;
  virtual bool aesActive () const = 0;
  virtual bool has (const Peer& p) const = 0;
  virtual Peer peer (uint8_t idx) const = 0;
  virtual bool peer (const Peer& p) = 0;
  virtual bool peer (const Peer& p1,const Peer& p2) = 0;
  virtual bool deletepeer (const Peer& p) = 0;
  virtual void firstinit () = 0;

  virtual void stop () = 0;
  virtual bool process (const ActionSetMsg& msg) = 0;
  virtual bool process (const ActionCommandMsg& msg) = 0;
  virtual bool process (const RemoteEventMsg& msg) = 0;
  virtual bool process (const SensorEventMsg& msg) = 0;
  virtual uint8_t status () = 0;
  virtual uint8_t flags () = 0;

  virtual void patchStatus (Message& msg) = 0;
  virtual void configChanged () = 0;

  virtual GenericList getList1 () const = 0;
  virtual GenericList getList3 (const Peer& p) const = 0;
  virtual GenericList getList4 (const Peer& p) const = 0;
  virtual bool hasList3 () const = 0;
  virtual bool hasList4 () const = 0;

};

template <class HalType,class ChannelType,class List0Type=List0>
class VirtChannel : public VirtBaseChannel<HalType,List0Type> {
  ChannelType ch;
public:
  VirtChannel () {}
  virtual ~VirtChannel () {}

  operator ChannelType& () { return ch; }

  virtual void setup(Device<HalType,List0Type>* dev,uint8_t number,uint16_t addr) { ch.setup(dev,number,addr); }
  virtual uint16_t size () const { return ch.size(); }
  virtual uint8_t number () const { return ch.number(); }
  virtual uint16_t address () const { return ch.address(); }
  virtual uint8_t peers () const { return ch.peers(); }
  virtual bool changed () const { return ch.changed(); }
  virtual void changed (bool c) { ch.changed(c); }
  virtual void inhibit (bool value) { ch.inhibit(value); }
  virtual bool inhibit () const { return ch.inhibit(); }
  virtual bool aesActive () const { return ch.aesActive(); }
  virtual bool has (const Peer& p) const { return ch.has(p); };
  virtual Peer peer (uint8_t idx) const { return ch.peer(idx); }
  virtual bool peer (const Peer& p) { return ch.peer(p); }
  virtual bool peer (const Peer& p1,const Peer& p2) { return ch.peer(p1,p2); }
  virtual bool deletepeer (const Peer& p) { return ch.deletepeer(p); }
  virtual void firstinit () { ch.firstinit(); }

  virtual void stop () { ch.stop(); };
  virtual bool process (const ActionSetMsg& msg) { return ch.process(msg); }
  virtual bool process (const ActionCommandMsg& msg) { return ch.process(msg); }
  virtual bool process (const RemoteEventMsg& msg) { return ch.process(msg); }
  virtual bool process (const SensorEventMsg& msg) { return ch.process(msg); }
  virtual uint8_t status () { return ch.status(); }
  virtual uint8_t flags () { return ch.flags(); }

  virtual void patchStatus (Message& msg) { ch.patchStatus(msg); }
  virtual void configChanged () { ch.configChanged(); }

  virtual GenericList getList1 () const { return ch.getList1(); }
  virtual GenericList getList3 (const Peer& p) const { return ch.getList3(p); }
  virtual GenericList getList4 (const Peer& p) const { return ch.getList4(p); }
  virtual bool hasList3 () const { return ChannelType::hasList3(); }
  virtual bool hasList4 () const { return ChannelType::hasList4(); }
};

#define channelISR(chan,pin,mode,type) class __##pin##ISRHandler { \
    public: \
    static void isr () { chan.handleISR(); } \
  }; \
  pinMode(pin,mode); \
  enableInterrupt(pin,__##pin##ISRHandler::isr,type);


}

#endif
