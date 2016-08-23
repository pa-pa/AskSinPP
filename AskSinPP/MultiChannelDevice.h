
#ifndef __MULTICHANNELDEVICE_H__
#define __MULTICHANNELDEVICE_H__

#include "Device.h"


template <class ChannelType,int ChannelNumber>
class MultiChannelDevice : public Device {

  ChannelType devchannels[ChannelNumber];

public:
  MultiChannelDevice (uint16_t addr) {
    for( uint8_t i=0; i<channels(); ++i ) {
      devchannels[i].setup(this,i,addr);
      addr += devchannels[i].size();
    }
  }
  virtual ~MultiChannelDevice () {}

  uint8_t channels () const {
    return ChannelNumber;
  }

  void firstinit () {
    for( uint8_t i=0; i<channels(); ++i ) {
      devchannels[i].firstinit();
    }
  }

  ChannelType& channel(uint8_t idx) {
    return devchannels[idx];
  }

  bool addPeer (uint8_t channel,const Peer& p) {
     ChannelType& sc = devchannels[channel];
     sc.deletepeer(p);
     uint8_t pidx = sc.findpeer();
     if( pidx != 0xff ) {
       sc.peer(pidx,p);
       sc.getList3(pidx).single();
       return true;
     }
     return false;
   }

   bool addPeer(uint8_t channel,const Peer& odd, const Peer& even) {
     ChannelType& sc = devchannels[channel];
     sc.deletepeer(odd);
     sc.deletepeer(even);
     uint8_t pidx1 = sc.findpeer();
     if( pidx1 != 0xff ) {
       sc.peer(pidx1,odd);
       uint8_t pidx2 = sc.findpeer();
       if( pidx2 != 0xff ) {
         sc.peer(pidx2,even);
         sc.getList3(pidx1).odd();
         sc.getList3(pidx2).even();
         return true;
       }
       else {
         // free already stored data
         sc.peer(pidx1,Peer());
       }
     }
     return false;
   }

};

#endif
