//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "Peer.h"
#include "Storage.h"

namespace as {

template <class HalType> class Device;
class ActionSetMsg;
class RemoteEventMsg;
class SensorEventMsg;

template<class HalType,class List1Type,class List3Type,class List4Type,int PeerCount>
class Channel {
  Device<HalType>*   dev;
  bool      change; // the status is changed, we may need to send a status
  bool      inhi;
  uint8_t   num   ; // channels per device
  uint16_t  addr  ; // start address in eeprom

public:
  typedef List1Type List1;
  typedef List3Type List3;
  typedef List4Type List4;
  typedef Device<HalType> DeviceType;

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

  void setup(Device<HalType>* dev,uint8_t number,uint16_t addr) {
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

  Peer peer (uint8_t idx) const {
    Peer result;
    uint16_t paddr = peerAddress(idx);
    if( paddr != 0 ) {
      storage.getData(paddr,&result);
    }
    return result;
  }


  bool peer (uint8_t idx,const Peer& p) const {
    return storage.setData(peerAddress(idx),p);
  }

  uint8_t findpeer () const {
    for( int i=0; i<peers(); ++i ) {
      if( peer(i).valid()==false ) {
        return i;
      }
    }
    return 0xff;
  }

  bool deletepeer (const Peer& p) {
    for( uint8_t i=0; i<peers(); ++i ) {
      if( peer(i) == p ) {
        peer(i,Peer());
      }
    }
    return true;
  }

  void firstinit () {
    storage.clearData(address(),size());
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

  bool process (__attribute__((unused)) const ActionSetMsg& msg) {
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

}

#endif
