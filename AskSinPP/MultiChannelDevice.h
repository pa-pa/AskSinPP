
#ifndef __MULTICHANNELDEVICE_H__
#define __MULTICHANNELDEVICE_H__

#include "Device.h"
#include "Defines.h"
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
       if( msg.type() == AS_MESSAGE_CONFIG ) {
         // PAIR_SERIAL
         if( msg.subcommand() == AS_CONFIG_PAIR_SERIAL && memcmp(msg.data(),getSerial(),10)==0 ) {
           list0.masterid(msg.from());
           setMasterID(list0.masterid());
           sendDeviceInfo(getMasterID(),msg.count());
         }
         // CONFIG_PEER_LIST_REQ
         else if( msg.subcommand() == AS_CONFIG_PEER_LIST_REQ ) {
           uint8_t ch = msg.command();
           if( ch > 0 && ch <= channels() ) {
             ChannelType& c = channel(ch);
             sendInfoPeerList(msg.from(),msg.count(),c);
           }
         }
         // CONFIG_PARAM_REQ
         else if (msg.subcommand() == AS_CONFIG_PARAM_REQ ) {
           uint8_t ch = msg.command();
           if( ch == 0 ) {
             // channel 0 only has list0
             sendInfoParamResponsePairs(msg.from(),msg.count(),list0);
           }
           else if ( ch <= channels() ) { // TODO hasChannel
             uint8_t numlist = *(msg.data()+4);
             ChannelType& c = channel(ch);
             if( numlist == 1 ) {
               sendInfoParamResponsePairs(msg.from(),msg.count(),c.getList1());
             }
             else if( numlist == 3 ) {
               Peer* p = (Peer*)msg.data();
               typename ChannelType::List3 l3 = c.getList3(*p);
               if( l3.valid() == true ) {
                 sendInfoParamResponsePairs(msg.from(),msg.count(),l3);
               }
             }
           }
         }
         // CONFIG_STATUS_REQUEST
         else if (msg.subcommand() == AS_CONFIG_STATUS_REQUEST ) {
           sendInfoActuatorStatus(msg.from(),msg.count(),channel(msg.command()));
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
