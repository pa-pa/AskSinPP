
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
  uint8_t     cfgChannel;
  Peer        cfgPeer;
  uint8_t     cfgList;

public:
  MultiChannelDevice (uint16_t addr) : list0(addr), cfgChannel(0xff), cfgList(0) {
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
         }
         // CONFIG_STATUS_REQUEST
         else if (msg.subcommand() == AS_CONFIG_STATUS_REQUEST ) {
           sendInfoActuatorStatus(msg.from(),msg.count(),channel(msg.command()));
         }
         // CONFIG_START
         else if( msg.subcommand() == AS_CONFIG_START ) {
           const ConfigStartMsg& pm = msg.configStart();
           cfgChannel = pm.channel();
           cfgPeer = pm.peer();
           cfgList = pm.list();
           // TODO setup alarm to disable after 2000ms
           sendAck(msg);
         }
         // CONFIG_END
         else if( msg.subcommand() == AS_CONFIG_END ) {
           if( cfgList == 0 ) {
             setMasterID(list0.masterid());
           }
           cfgChannel = 0xff;
           // TODO cancel alarm
           sendAck(msg);
         }
         else if( msg.subcommand() == AS_CONFIG_WRITE_INDEX ) {
           const ConfigWriteIndexMsg& pm = msg.configWriteIndex();
           if( cfgChannel == pm.channel() ) {
             if( cfgList == 0 ) {
               writeList(list0,pm.data(),pm.datasize());
             }
             else if( hasChannel(pm.channel()) == true ) {
               ChannelType& ch = channel(pm.channel());
               if (cfgList == 1) {
                 typename ChannelType::List1 l1 = ch.getList1();
                 writeList(l1,pm.data(),pm.datasize());
               } else if (cfgList == 3) {
                 typename ChannelType::List3 l3 = ch.getList3(cfgPeer);
                 if (l3.valid() == true) {
                   writeList(l3,pm.data(),pm.datasize());
                 }
               }
             }
           }
           sendAck(msg);
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
         bool lg = (msg.command() & 0x40) == 0x40;
         Peer p(msg.from(),msg.command() & 0x3f);
//         p.dump();
         for( uint8_t i=1; i<=channels(); ++i ) {
           ChannelType& ch = channel(i);
           typename ChannelType::List3 l3 = ch.getList3(p);
           if( l3.valid() == true ) {
             // l3.dump();
             typename ChannelType::List3::PeerList pl = lg ? l3.lg() : l3.sh();
             // l3->actiontype
             // pl.dump();
             ch.jumpToTarget(pl);
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
