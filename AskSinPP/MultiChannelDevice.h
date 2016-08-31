
#ifndef __MULTICHANNELDEVICE_H__
#define __MULTICHANNELDEVICE_H__

#include "Device.h"
#include "Defines.h"
#include "List0.h"
#include "cm.h"

template <class ChannelType,int ChannelCount>
class MultiChannelDevice : public Device {

  List0       list0;
  ChannelType devchannels[ChannelCount];

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
    return ChannelCount;
  }

  bool hasChannel (uint8_t number) const {
    return number != 0 && number <= ChannelCount;
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

  void pollRadio () {
    Device::pollRadio();
    for( uint8_t i=0; i<channels(); ++i ) {
      ChannelType& ch = channel(i);
      if( ch.changed() == true ) {
        sendInfoActuatorStatus(getMasterID(),nextcount(),ch);
      }
    }
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
         // CONFIG_PEER_ADD
         else if ( msg.subcommand() == AS_CONFIG_PEER_ADD ) {
           const ConfigPeerAddMsg& pm = msg.configPeerAdd();
           bool success = false;
           /*
           uint8_t ch = msg.command();
           Peer* p1 = (Peer*)msg.data();
           if( *(msg.data()+sizeof(Peer)) == 0x00 ) {
             success = addPeer(channel(ch),*p1);
           }
           else {
             Peer p2(*p1,*(msg.data()+sizeof(Peer)));
             success = addPeer(channel(ch),*p1,p2);
           }
           */
           if( hasChannel(pm.channel()) == true ) {
             ChannelType& ch = channel(pm.channel());
             if( pm.peers() == 1 ) {
               success = addPeer(ch,pm.peer1());
             }
             else {
               success = addPeer(ch,pm.peer1(),pm.peer2());
             }
           }
           success == true ? sendAck(msg) : sendNack(msg);
         }
         // CONFIG_PEER_REMOVE
         else if ( msg.subcommand() == AS_CONFIG_PEER_REMOVE ) {
           const ConfigPeerRemoveMsg& pm = msg.configPeerRemove();
           bool success = false;
           if( hasChannel(pm.channel()) == true ) {
             ChannelType& ch = channel(pm.channel());
             success = ch.deletepeer(pm.peer1());
             if( pm.peers() == 2 ) {
               success &= ch.deletepeer(pm.peer2());
             }
           }
           /*
           uint8_t ch = msg.command();
           Peer* p1 = (Peer*)msg.data();
           success = channel(ch).deletepeer(*p1);
           if( *(msg.data()+sizeof(Peer)) != 0x00 ) {
             Peer p2(*p1,*(msg.data()+sizeof(Peer)));
             success = channel(ch).deletepeer(p2);
           }
           */
           success == true ? sendAck(msg) : sendNack(msg);
         }
         // CONFIG_PEER_LIST_REQ
         else if( msg.subcommand() == AS_CONFIG_PEER_LIST_REQ ) {
           const ConfigPeerListReqMsg& pm = msg.configPeerListReq();
           if( hasChannel(pm.channel()) == true ) {
             sendInfoPeerList(msg.from(),msg.count(),channel(pm.channel()));
           }
         }
         // CONFIG_PARAM_REQ
         else if (msg.subcommand() == AS_CONFIG_PARAM_REQ ) {
           const ConfigParamReqMsg& pm = msg.configParamReq();
           if( pm.channel() == 0 ) {
             // channel 0 only has list0
             sendInfoParamResponsePairs(msg.from(),msg.count(),list0);
           }
           else if( hasChannel(pm.channel()) == true ) {
             uint8_t numlist = pm.list();
             ChannelType& ch = channel(pm.channel());
             if( numlist == 1 ) {
               sendInfoParamResponsePairs(msg.from(),msg.count(),ch.getList1());
             }
             else if( numlist == 3 ) {
               typename ChannelType::List3 l3 = ch.getList3(pm.peer());
               if( l3.valid() == true ) {
                 sendInfoParamResponsePairs(msg.from(),msg.count(),l3);
               }
             }
           }
           /*
           uint8_t ch = msg.command();
           if( ch == 0 ) {
             // channel 0 only has list0
             sendInfoParamResponsePairs(msg.from(),msg.count(),list0);
           }
           else if ( ch <= channels() ) {
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
           */
         }
         // CONFIG_STATUS_REQUEST
         else if (msg.subcommand() == AS_CONFIG_STATUS_REQUEST ) {
           sendInfoActuatorStatus(msg.from(),msg.count(),channel(msg.command()));
         }
       }
       else if( msg.type() == AS_MESSAGE_ACTION ) {
         if( msg.command() == AS_ACTION_SET ) {
           ChannelType& c = channel(msg.subcommand());
           uint8_t value = *msg.data();
           uint16_t delay = 0xffff;
           if( msg.datasize() >= 5) {
             delay = (*(msg.data()+3) << 8) + *(msg.data()+4);
           }
           if( delay == 0 ) delay = 0xffff;
           c.setState( value == 0 ? AS_CM_JT_OFF : AS_CM_JT_ON, delay );
           sendAck(msg,c);
         }
       }
       else if (msg.type() == AS_MESSAGE_REMOTE_EVENT ) {
         Peer p(msg.from(),msg.command() & 0x3f);
//         p.dump();
         for( uint8_t i=1; i<=channels(); ++i ) {
           ChannelType& ch = channel(i);
           typename ChannelType::List3 l3 = ch.getList3(p);
           if( l3.valid() == true ) {
             // l3.dump();
             // TODO long press / l3->actiontype
             ch.jumpToTarget(l3.sh());
             sendAck(msg,ch);
           }
         }
       }
     }
     else {
//       DPRINT(F("ignore "));
//       msg.dump();
     }
   }

};

#endif
