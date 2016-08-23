
#include "SwitchDevice.h"

  bool SwitchDevice::addPeer (uint8_t channel,const Peer& p) {
    SwitchChannel& sc = schannels[channel];
    sc.deletepeer(p);
    uint8_t pidx = sc.findpeer();
    if( pidx != 0xff ) {
      sc.peer(pidx,p);
      SwitchStateList(sc.listAddress(3,pidx)).single();
      return true;
    }
    return false;
  }

  bool SwitchDevice::addPeer(uint8_t channel,const Peer& odd, const Peer& even) {
    SwitchChannel& sc = schannels[channel];
    sc.deletepeer(odd);
    sc.deletepeer(even);
    uint8_t pidx1 = sc.findpeer();
    if( pidx1 != 0xff ) {
      sc.peer(pidx1,odd);
      uint8_t pidx2 = sc.findpeer();
      if( pidx2 != 0xff ) {
        sc.peer(pidx2,even);
        SwitchStateList(sc.listAddress(3,pidx1)).odd();
        SwitchStateList(sc.listAddress(3,pidx2)).even();
        return true;
      }
      else {
        // free already stored data
        sc.peer(pidx1,Peer());
      }
    }
    return false;
  }
