
#ifndef __SWITCHDEVICE_H__
#define __SWITCHDEVICE_H__

#include "Device.h"
#include "SwitchChannel.h"

class SwitchDevice : public Device {

  SwitchChannel schannels[4];

public:
  SwitchDevice (uint16_t addr) {
    for( uint8_t i=0; i<channels(); ++i ) {
      schannels[i].setup(this,i,addr,4);
      addr += schannels[i].size();
    }
  }
  virtual ~SwitchDevice () {}

  uint8_t channels () const {
    return sizeof(schannels)/sizeof(SwitchChannel);
  }

  void firstinit () {
    for( uint8_t i=0; i<channels(); ++i ) {
      schannels[i].firstinit();
    }
  }

  virtual Channel& channel(uint8_t idx) {
    return schannels[idx];
  }

  virtual bool addPeer (uint8_t channel,const Peer& p);
  virtual bool addPeer (uint8_t channel,const Peer& odd, const Peer& even);

};


#endif
