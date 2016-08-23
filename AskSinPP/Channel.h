
#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "Peer.h"
#include "EEProm.h"

class Device;

class Channel {
  Device*   dev;
  uint8_t   num          : 3;  // max 7 channels per device
  uint16_t  addr         : 13; // max address 8191
  uint8_t   numpeers     : 3;  // max 7 peers
  uint8_t   list1size    : 5;  // max 31 byte
  uint8_t   list3size;         // max 255 byte

  public:
  Channel (uint8_t l1size, uint8_t l3size) : dev(0), num(0), addr(0), numpeers(0) {
    list1size = l1size;
    list3size = l3size;
  }

  Device& device () { return *dev; }

  uint8_t number () const { return num; }

  uint16_t address () const { return addr; }

  uint8_t peers () const { return numpeers; }

  void setup(Device* dev,uint8_t number,uint16_t addr,uint8_t peers) {
    this->dev = dev;
    this->num = number;
    this->addr = addr;
    this->numpeers = peers;
  }

  uint16_t size () const {
    uint16_t size = sizeof(Peer);
    size += list3size;
    size *= numpeers;
    size += list1size;
    return size;
  }

  Peer peer (uint8_t idx) const {
    Peer result;
    uint16_t paddr = peerAddress(idx);
    if( paddr != 0 ) {
      eeprom.getData(paddr,&result);
    }
    return result;
  }


  bool peer (uint8_t idx,const Peer& p) const{
    return eeprom.setData(peerAddress(idx),p);
  }

  uint8_t findpeer () const;

  bool deletepeer (const Peer& p);

  uint16_t peerAddress (uint8_t idx) const;

  uint16_t listAddress (uint8_t list,const Peer& p) const;

  uint16_t listAddress (uint8_t list, uint8_t peer=0) const;

};

#endif
