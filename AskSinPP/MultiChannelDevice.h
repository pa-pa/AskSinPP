
#ifndef __MULTICHANNELDEVICE_H__
#define __MULTICHANNELDEVICE_H__

#include "Device.h"
#include "List0.h"

template <class ChannelType,int ChannelNumber>
class MultiChannelDevice : public Device {

  List0       list0;
  ChannelType devchannels[ChannelNumber];

public:
  MultiChannelDevice (uint16_t addr) : list0(addr) {
    addr += list0.size();
    for( uint8_t i=0; i<channels(); ++i ) {
      devchannels[i].setup(this,i+1,addr);
      addr += devchannels[i].size();
    }
  }
  virtual ~MultiChannelDevice () {}


  uint8_t channels () const {
    return ChannelNumber;
  }

  void init (CC1101& r,const HMID& id,const char* serial) {
    // read master id from flash
    setMasterID(list0.masterid());
    setDeviceID(id);
    setSerial(serial);
    setRadio(r);
  }

  void firstinit () {
    list0.defaults();
    for( uint8_t i=0; i<channels(); ++i ) {
      devchannels[i].firstinit();
    }
  }

  ChannelType& channel(uint8_t ch) {
    return devchannels[ch-1];
  }

  bool addPeer (uint8_t ch,const Peer& p) {
     ChannelType& sc = channel(ch);
     sc.deletepeer(p);
     uint8_t pidx = sc.findpeer();
     if( pidx != 0xff ) {
       sc.peer(pidx,p);
       sc.getList3(pidx).single();
       return true;
     }
     return false;
   }

   bool addPeer(uint8_t ch,const Peer& odd, const Peer& even) {
     ChannelType& sc = channel(ch);
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

   void process(Message& msg) {
     if( msg.to() == getDeviceID() || (msg.to() == HMID::boardcast && isBoardcastMsg(msg))) {
       DPRINT(F("-> "));
       msg.dump();
       if( msg.type() == 0x01 ) { // CONFIG_
         // PAIR_SERIAL
         if( msg.subcommand() == 0x0a && memcmp(msg.data(),getSerial(),10)==0 ) {
           sendDeviceInfo(msg.from(),msg.count());
         }
         // CONFIG_PARAM_REQ
         else if (msg.subcommand() == 0x04 ) {

         }
         // CONFIG_STATUS_REQUEST
         else if (msg.subcommand() == 0x0e ) {
           uint8_t ch = msg.command();
           sendInfoActuatorStatus(msg.from(),msg.count(),ch,channel(ch).status());
         }
       }
     }
     else {
       DPRINT(F("ignore "));
       msg.dump();
     }
   }

};

#endif
