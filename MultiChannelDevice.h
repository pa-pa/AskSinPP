//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __MULTICHANNELDEVICE_H__
#define __MULTICHANNELDEVICE_H__

#include <Device.h>
#include <Defines.h>
#include <cm.h>
#include <Led.h>
#include <Activity.h>

#include <avr/wdt.h>

namespace as {

template <class ChannelType,int ChannelCount,class List0Type=List0>
class MultiChannelDevice : public Device {

  List0Type   list0;
  ChannelType devchannels[ChannelCount];
  uint8_t     numChannels;
  uint8_t     cfgChannel;
  GenericList cfgList;

public:
  MultiChannelDevice (uint16_t addr) : Device(addr), list0(addr + kstore.size()), numChannels(ChannelCount), cfgChannel(0xff) {
    addr = list0.address() + list0.size();
    for( uint8_t i=0; i<channels(); ++i ) {
      devchannels[i].setup(this,i+1,addr);
      addr += devchannels[i].size();
    }
  }
  virtual ~MultiChannelDevice () {}

  uint16_t checksum () {
    uint16_t crc = 0;
    // size of keystore data
    crc = EEProm::crc16(crc,kstore.size());
    // add register of list0
    for( uint8_t i=0; i<list0.size(); ++i ) {
      crc = EEProm::crc16(crc,list0.getRegister(i));
    }
    // add number of channels
    crc = EEProm::crc16(crc,ChannelCount);
    // add register list 1
    for( uint8_t i=0; i<ChannelType::List1::size(); ++i ) {
      crc = EEProm::crc16(crc,ChannelType::List1::getRegister(i));
    }
    // add register list 3
    for( uint8_t i=0; i<ChannelType::List3::size(); ++i ) {
      crc = EEProm::crc16(crc,ChannelType::List3::getRegister(i));
    }
    // add register list 4
    for( uint8_t i=0; i<ChannelType::List4::size(); ++i ) {
      crc = EEProm::crc16(crc,ChannelType::List4::getRegister(i));
    }
    // add number of peers
    crc = EEProm::crc16(crc,channel(1).peers());
    return crc;
  }

  void channels (uint8_t num) {
    numChannels = min(num,ChannelCount);
  }

  uint8_t channels () const {
    return numChannels;
  }

  bool hasChannel (uint8_t number) const {
    return number != 0 && number <= channels();
  }

  void init (CC1101& r,const HMID& id,const char* serial) {
    kstore.init();
    // read master id from flash
    setMasterID(list0.masterid());
    setDeviceID(id);
    setSerial(serial);
    setRadio(r);
  }

  void init (CC1101& r,uint16_t idaddr, uint16_t serialaddr) {
    kstore.init();
    // read master id from flash
    setMasterID(list0.masterid());
    // read id & serial from bootloader
    setDeviceID(idaddr);
    setSerial(serialaddr);
    setRadio(r);
  }

  void firstinit () {
    kstore.defaults(); // init aes key infrastructure
    list0.defaults();
    for( uint8_t i=0; i<channels(); ++i ) {
      devchannels[i].firstinit();
    }
  }

  void reset () {
    DPRINTLN(F("RESET"));
    firstinit();
    kstore.init();
    setMasterID(list0.masterid());
    sled.set(StatusLed::welcome);
  }

  void bootloader () {
    DPRINTLN(F("BOOTLOADER"));
    wdt_enable(WDTO_250MS);
    while(1);
  }

  void startPairing () {
    sled.set(StatusLed::pairing);
    activity.stayAwake( seconds2ticks(20) ); // 20 seconds
    sendDeviceInfo();
  }

  ChannelType& channel(uint8_t ch) {
    return devchannels[ch-1];
  }

  bool pollRadio () {
    bool worked = Device::pollRadio();
    for( uint8_t i=1; i<=channels(); ++i ) {
      ChannelType& ch = channel(i);
      if( ch.changed() == true ) {
        sendInfoActuatorStatus(getMasterID(),nextcount(),ch);
        worked = true;
      }
    }
    return worked;
  }

   void process(Message& msg) {
     if( msg.to() == getDeviceID() || (msg.to() == HMID::boardcast && isBoardcastMsg(msg))) {
       DPRINT(F("-> "));
       msg.dump();
       // ignore repeated messages
       if( isRepeat(msg) == true ) {
         if( msg.ackRequired() == true ) {
           sendNack(msg);
         }
         return;
       }
       uint8_t mtype = msg.type();
       uint8_t mcomm = msg.command();
       uint8_t msubc = msg.subcommand();
       if( mtype == AS_MESSAGE_CONFIG ) {
         // PAIR_SERIAL
         if( msubc == AS_CONFIG_PAIR_SERIAL && memcmp(msg.data(),getSerial(),10)==0 ) {
           sled.set(StatusLed::pairing);
           activity.stayAwake( seconds2ticks(20) ); // 20 seconds
           sendDeviceInfo(getMasterID(),msg.length());
         }
         // CONFIG_PEER_ADD
         else if ( msubc == AS_CONFIG_PEER_ADD ) {
           const ConfigPeerAddMsg& pm = msg.configPeerAdd();
           bool success = false;
           if( hasChannel(pm.channel()) == true ) {
             ChannelType& ch = channel(pm.channel());
#ifdef USE_AES
             if( ch.getList1().aesActive() == false || requestSignature(msg) == true )
#endif
             {
               if( pm.peers() == 1 ) {
                 success = addPeer(ch,pm.peer1());
               }
               else {
                 success = addPeer(ch,pm.peer1(),pm.peer2());
               }
             }
           }
           success == true ? sendAck(msg) : sendNack(msg);
         }
         // CONFIG_PEER_REMOVE
         else if ( msubc == AS_CONFIG_PEER_REMOVE ) {
           const ConfigPeerRemoveMsg& pm = msg.configPeerRemove();
           bool success = false;
           if( hasChannel(pm.channel()) == true ) {
             ChannelType& ch = channel(pm.channel());
#ifdef USE_AES
             if( ch.getList1().aesActive() == false || requestSignature(msg) == true )
#endif
             {
               success = ch.deletepeer(pm.peer1());
               if( pm.peers() == 2 ) {
                 success &= ch.deletepeer(pm.peer2());
               }
             }
           }
           success == true ? sendAck(msg) : sendNack(msg);
         }
         // CONFIG_PEER_LIST_REQ
         else if( msubc == AS_CONFIG_PEER_LIST_REQ ) {
           const ConfigPeerListReqMsg& pm = msg.configPeerListReq();
           if( hasChannel(pm.channel()) == true ) {
             sendInfoPeerList(msg.from(),msg.count(),channel(pm.channel()));
           }
         }
         // CONFIG_PARAM_REQ
         else if (msubc == AS_CONFIG_PARAM_REQ ) {
           const ConfigParamReqMsg& pm = msg.configParamReq();
           GenericList gl = findList(pm.channel(),pm.peer(),pm.list());
           if( gl.valid() == true ) {
             sendInfoParamResponsePairs(msg.from(),msg.count(),gl);
           }
         }
         // CONFIG_STATUS_REQUEST
         else if (msubc == AS_CONFIG_STATUS_REQUEST ) {
           sendInfoActuatorStatus(msg.from(),msg.count(),channel(msg.command()));
         }
         // CONFIG_START
         else if( msubc == AS_CONFIG_START ) {
           const ConfigStartMsg& pm = msg.configStart();
#ifdef USE_AES
           ChannelType& ch = channel(pm.channel());
           if( ch.getList1().aesActive() == true && requestSignature(msg) == false ) {
             sendNack(msg);
           }
           else
#endif
           {
             cfgChannel = pm.channel();
             cfgList = findList(cfgChannel,pm.peer(),pm.list());
             // TODO setup alarm to disable after 2000ms
             sendAck(msg);
           }
         }
         // CONFIG_END
         else if( msubc == AS_CONFIG_END ) {
           if( cfgList.address() == list0.address() ) {
             setMasterID(list0.masterid());
             sled.set(StatusLed::nothing);
           }
           else {
             // signal list update to channel
             channel(cfgChannel).configChanged();
           }
           cfgChannel = 0xff;
           // TODO cancel alarm
           sendAck(msg);
         }
         else if( msubc == AS_CONFIG_WRITE_INDEX ) {
           const ConfigWriteIndexMsg& pm = msg.configWriteIndex();
#ifdef USE_AES
           ChannelType& ch = channel(pm.channel());
           if( ch.getList1().aesActive() == true && requestSignature(msg) == false ) {
             sendNack(msg);
           }
           else
#endif
           {
             if( cfgChannel == pm.channel() && cfgList.valid() == true ) {
               writeList(cfgList,pm.data(),pm.datasize());
             }
             sendAck(msg);
           }
         }
         // default - send Nack if answer is requested
         else {
           if( msg.ackRequired() == true ) {
             sendNack(msg);
           }
         }
       }
       else if( mtype == AS_MESSAGE_ACTION ) {
         if ( mcomm == AS_ACTION_RESET || mcomm == AS_ACTION_ENTER_BOOTLOADER ) {
#ifdef USE_AES
           if( requestSignature(msg) == true )
#endif
           {
             sendAck(msg);
             if( mcomm == AS_ACTION_ENTER_BOOTLOADER ) {
               bootloader();
             }
             else {
               reset();
             }
           }
         }
         else {
           bool ack=false;
           const ActionMsg& pm = msg.action();
           ChannelType& ch = channel(pm.channel());
#ifdef USE_AES
           if( ch.getList1().aesActive() == false || requestSignature(msg) == true )
#endif
           {
             switch( mcomm ) {
             case AS_ACTION_INHIBIT_OFF:
               ch.inhibit(false);
               ack = true;
               break;
             case AS_ACTION_INHIBIT_ON:
               ch.inhibit(true);
               ack = true;
               break;
             case AS_ACTION_SET:
               if( ch.inhibit() == false ) {
                 ack = ch.process(msg.actionSet());
               }
               break;
             }
           }
           if( ack == true ) sendAck(msg,ch);
           else sendNack(msg);
         }
       }
       else if (mtype == AS_MESSAGE_REMOTE_EVENT || mtype == AS_MESSAGE_SENSOR_EVENT) {
         const RemoteEventMsg& pm = msg.remoteEvent();
         uint8_t cdx = channelForPeer(pm.peer());
         bool ack=false;
         if( cdx != 0 ) {
           ChannelType& ch = channel(cdx);
           if( ch.inhibit() == false ) {
#ifdef USE_AES
             if( ch.getList1().aesActive() == false || requestSignature(msg) == true )
#endif
             {
               switch( mtype ) {
               case AS_MESSAGE_REMOTE_EVENT:
                 ack = ch.process(pm);
                 break;
               case AS_MESSAGE_SENSOR_EVENT:
                 ack = ch.process(msg.sensorEvent());
                 break;
               }
               if( ack == true ) sendAck(msg,ch);
             }
           }
         }
         if( ack == false ) sendNack(msg);
       }
#ifdef USE_AES
       else if (mtype == AS_MESSAGE_KEY_EXCHANGE ) {
         if( requestSignature(msg) == true ) {
           if( kstore.exchange(msg.aesExchange())==true ) sendAck(msg);
           else sendNack(msg);
         }
       }
#endif
       // default - send Nack if answer is requested
       else {
         if( msg.ackRequired() == true ) {
           sendNack(msg);
         }
       }
     }
     else {
//       DPRINT(F("ignore "));
//       msg.dump();
     }
   }

   uint8_t channelForPeer (const Peer& p) {
     for( uint8_t x=1; x<=channels(); ++x ) {
       ChannelType& ch = channel(x);
       for( uint8_t y=0; y<ch.peers(); ++y ) {
         if( ch.peer(y) == p ) {
           return x;
         }
       }
     }
     return 0;
   }

   GenericList findList(uint8_t ch,const Peer& peer,uint8_t numlist) {
    if (numlist == 0) {
      return list0;
    } else if (hasChannel(ch) == true) {
      ChannelType& c = channel(ch);
      if (numlist == 1) {
        return c.getList1();
      } else if (ChannelType::hasList3() && numlist == 3) {
        return c.getList3(peer);
      } else if (ChannelType::hasList4() && numlist == 4) {
        return c.getList4(peer);
      }
    }
    return GenericList();
   }

   void sendPeerEvent (Message& msg,const ChannelType& ch) {
     // we send only to peers if there is no config message pending
     if( cfgChannel != 0xff ) {
       Device::sendPeerEvent(msg,ch);
     }
   }
};

}

#endif
