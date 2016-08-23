
#include "Channel.h"


  uint8_t Channel::findpeer () const {
    for( int i=0; i<peers(); ++i ) {
      if( peer(i).valid()==false ) {
        return i;
      }
    }
    return 0xff;
  }

  bool Channel::deletepeer (const Peer& p) {
    for( uint8_t i=0; i<peers(); ++i ) {
      if( peer(i) == p ) {
        peer(i,Peer());
      }
    }
    return true;
  }


  uint16_t Channel::peerAddress (uint8_t idx) const {
    if( idx < numpeers ) {
      uint16_t offset = sizeof(Peer);
      offset += list3size;
      offset *= idx;
      offset += list1size;
      return addr + offset;
    }
    return 0x00;
  }

  uint16_t Channel::listAddress (uint8_t list,const Peer& p) const {
    uint16_t addr = 0x00;
    for( int i=0; addr==0 && i<peers(); ++i ) {
      if( peer(i) == p ) {
        addr = listAddress(list,i);
      }
    }
    return addr;
  }

  uint16_t Channel::listAddress (uint8_t list, uint8_t peer) const {
    if( list == 1 ) {
      return address();
    }
    else if( list == 3 && peer < peers() ) {
      return peerAddress(peer) + sizeof(Peer);
    }
    return 0x00;
  }
