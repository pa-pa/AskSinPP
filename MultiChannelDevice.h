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

#if ARDUINO_ARCH_AVR or ARDUINO_ARCH_ATMEGA32
#include <avr/wdt.h>
#endif

namespace as {

void(* resetFunc) (void) = 0;


template <class HalType,class ChannelType,int ChannelCount,class List0Type=List0>
class ChannelDevice : public Device<HalType> {

  List0Type    list0;
  ChannelType* devchannels[ChannelCount];
  uint8_t      numChannels;
  uint8_t      cfgChannel;
  GenericList  cfgList;

public:

  typedef Device<HalType> DeviceType;

  ChannelDevice (const DeviceInfo& i,uint16_t addr) : Device<HalType>(i,addr,list0), list0(addr + this->keystore().size()), numChannels(ChannelCount), cfgChannel(0xff) {}

  virtual ~ChannelDevice () {}

  void registerChannel(ChannelType& ch,uint8_t num) {
    if( num >= 1 && num <= ChannelCount) {
      devchannels[num-1] = &ch;
    }
  }

  void layoutChannels () {
    uint16_t addr = list0.address() + list0.size();
    for( uint8_t i=0; i<channels(); ++i ) {
      devchannels[i]->setup(this,i+1,addr);
      addr += devchannels[i]->size();
    }
  }

  void dumpSize () {
    ChannelType& ch = channel(channels());
    DPRINT("Address Space: ");DDEC(this->keystore().address());DPRINT(" - ");DDECLN((uint16_t)(ch.address() + ch.size()));
  }

  uint16_t checksum () {
    uint16_t crc = 0;
    // size of keystore data
    crc = HalType::crc16(crc,DeviceType::keystore().size());
    // add register of list0
    for( uint8_t i=0; i<list0.size(); ++i ) {
      crc = HalType::crc16(crc,list0.getRegister(i));
    }
    // add number of channels
    for( uint8_t c=1; c<=channels(); ++c ) {
      ChannelType& ch = channel(c);
      // add register list 1
      GenericList l = ch.getList1();
      for( uint8_t i=0; i<l.getSize(); ++i ) {
        crc = HalType::crc16(crc,l.getRegister(i));
      }
      // add register list 3
      l = ch.getList3(0);
      for( uint8_t i=0; i<l.getSize(); ++i ) {
        crc = HalType::crc16(crc,l.getRegister(i));
      }
      // add register list 4
      l = ch.getList4(0);
      for( uint8_t i=0; i<l.getSize(); ++i ) {
        crc = HalType::crc16(crc,l.getRegister(i));
      }
      // add number of peers
      crc = HalType::crc16(crc,ch.peers());
    }
    return crc;
  }

  List0Type& getList0 () {
    return list0;
  }

  void getDeviceInfo (uint8_t* info) {
    DeviceType::getDeviceInfo(info);
    // patch real channel count into device info
    *info = channels();
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

  void init (HalType& hal) {
    layoutChannels();
    dumpSize();
    // first initialize EEProm if needed
    if( storage.setup(checksum()) == true ) {
      firstinit();
      storage.store();
    }
    this->keystore().init();
    this->setHal(hal);
    HMID id;
    this->getDeviceID(id);
    hal.init(id);
    this->configChanged();
  }

  void firstinit () {
    this->keystore().defaults(); // init aes key infrastructure
    list0.defaults();
    for( uint8_t i=0; i<channels(); ++i ) {
      devchannels[i]->firstinit();
    }
  }

  void reset () {
    DPRINTLN(F("RESET"));
    firstinit();
    resetFunc();
  }

  void bootloader () {
    DPRINTLN(F("BOOTLOADER"));
#if ARDUINO_ARCH_AVR or ARDUINO_ARCH_ATMEGA32
    wdt_enable(WDTO_250MS);
    while(1);
#endif
  }

  void startPairing () {
    this->sendDeviceInfo();
    this->led().set(LedStates::pairing);
    this->activity().stayAwake( seconds2ticks(20) ); // 20 seconds
  }

  ChannelType& channel(uint8_t ch) {
    return *devchannels[ch-1];
  }

  bool pollRadio () {
    bool worked = DeviceType::pollRadio();
    for( uint8_t i=1; i<=channels(); ++i ) {
      ChannelType& ch = channel(i);
      if( ch.changed() == true ) {
        this->sendInfoActuatorStatus(this->getMasterID(),this->nextcount(),ch);
        worked = true;
      }
    }
    return worked;
  }

  bool aesActive () {
    if( getList0().aesActive() == true ) {
      return true;
    }
    for( uint8_t i=1; i<=channels(); ++i) {
      if( channel(i).aesActive() == true ) {
        return true;
      }
    }
    return false;
  }

  bool validSignature(Message& msg) {
#ifdef USE_AES
    if( aesActive() == true ) {
      return this->requestSignature(msg);
    }
#endif
    return true;
  }

  bool validSignature(uint8_t ch,Message& msg) {
#ifdef USE_AES
    if( (ch==0 && aesActive()) || (hasChannel(ch)==true && channel(ch).aesActive()==true) ) {
      return this->requestSignature(msg);
    }
#endif
    return true;
  }

   bool process(Message& msg) {
     HMID devid;
     this->getDeviceID(devid);
     if( msg.to() == devid || (msg.to() == HMID::broadcast && this->isBoardcastMsg(msg))) {
       DPRINT(F("-> "));
       msg.dump();
       // ignore repeated messages
       if( this->isRepeat(msg) == true ) {
         return false;
       }
       uint8_t mtype = msg.type();
       uint8_t mcomm = msg.command();
       uint8_t msubc = msg.subcommand();
       if( mtype == AS_MESSAGE_CONFIG ) {
         // PAIR_SERIAL
         if( msubc == AS_CONFIG_PAIR_SERIAL && this->isDeviceSerial(msg.data())==true ) {
           this->led().set(LedStates::pairing);
           this->activity().stayAwake( seconds2ticks(20) ); // 20 seconds
           this->sendDeviceInfo(this->getMasterID(),msg.length());
         }
         // CONFIG_PEER_ADD
         else if ( msubc == AS_CONFIG_PEER_ADD ) {
           const ConfigPeerAddMsg& pm = msg.configPeerAdd();
           bool success = false;
           if( hasChannel(pm.channel()) == true ) {
             if( validSignature(pm.channel(),msg) == true ) {
               ChannelType& ch = channel(pm.channel());
               if( pm.peers() == 1 ) {
                 success = ch.peer(pm.peer1());
               }
               else {
                 success = ch.peer(pm.peer1(),pm.peer2());
               }
             }
           }
           if( success == true ) {
             storage.store();
             this->sendAck(msg);
           }
           else {
             this->sendNack(msg);
           }
         }
         // CONFIG_PEER_REMOVE
         else if ( msubc == AS_CONFIG_PEER_REMOVE ) {
           const ConfigPeerRemoveMsg& pm = msg.configPeerRemove();
           bool success = false;
           if( hasChannel(pm.channel()) == true ) {
             if( validSignature(pm.channel(),msg) == true ) {
               ChannelType& ch = channel(pm.channel());
               success = ch.deletepeer(pm.peer1());
               if( pm.peers() == 2 ) {
                 success &= ch.deletepeer(pm.peer2());
               }
             }
           }
           if( success == true ) {
             storage.store();
             this->sendAck(msg);
           }
           else {
             this->sendNack(msg);
           }
         }
         // CONFIG_PEER_LIST_REQ
         else if( msubc == AS_CONFIG_PEER_LIST_REQ ) {
           const ConfigPeerListReqMsg& pm = msg.configPeerListReq();
           if( hasChannel(pm.channel()) == true ) {
             this->sendInfoPeerList(msg.from(),msg.count(),channel(pm.channel()));
           }
         }
         // CONFIG_PARAM_REQ
         else if (msubc == AS_CONFIG_PARAM_REQ ) {
           const ConfigParamReqMsg& pm = msg.configParamReq();
           GenericList gl = findList(pm.channel(),pm.peer(),pm.list());
           if( gl.valid() == true ) {
             this->sendInfoParamResponsePairs(msg.from(),msg.count(),gl);
           }
           else {
             this->sendNack(msg);
           }
         }
         // CONFIG_STATUS_REQUEST
         else if (msubc == AS_CONFIG_STATUS_REQUEST ) {
           // this is an answer to a request - so we need no ack
           this->sendInfoActuatorStatus(msg.from(),msg.count(),channel(msg.command()),false);
         }
         // CONFIG_START
         else if( msubc == AS_CONFIG_START ) {
           const ConfigStartMsg& pm = msg.configStart();
           if( validSignature(pm.channel(),msg) == true ) {
             cfgChannel = pm.channel();
             cfgList = findList(cfgChannel,pm.peer(),pm.list());
             // TODO setup alarm to disable after 2000ms
             this->sendAck(msg);
           }
           else {
             this->sendNack(msg);
           }
         }
         // CONFIG_END
         else if( msubc == AS_CONFIG_END ) {
           if( cfgList.address() == list0.address() ) {
             this->led().set(LedStates::nothing);
             this->configChanged();
           }
           else {
             // signal list update to channel
             channel(cfgChannel).configChanged();
           }
           cfgChannel = 0xff;
           storage.store();
           // TODO cancel alarm
           this->sendAck(msg,Message::WKMEUP);
         }
         else if( msubc == AS_CONFIG_WRITE_INDEX ) {
           const ConfigWriteIndexMsg& pm = msg.configWriteIndex();
           if( validSignature(pm.channel(),msg)==true ) {
             if( cfgChannel == pm.channel() && cfgList.valid() == true ) {
               this->writeList(cfgList,pm.data(),pm.datasize());
             }
             this->sendAck(msg);
           }
           else {
             this->sendNack(msg);
           }
         }
         else if( msubc == AS_CONFIG_SERIAL_REQ ) {
           this->sendSerialInfo(msg.from(),msg.count());
         }
         // default - send Nack if answer is requested
         else {
           if( msg.ackRequired() == true ) {
             this->sendNack(msg);
           }
         }
       }
       else if( mtype == AS_MESSAGE_ACTION ) {
         if ( mcomm == AS_ACTION_RESET || mcomm == AS_ACTION_ENTER_BOOTLOADER ) {
           if( validSignature(msg) == true ) {
             this->sendAck(msg);
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
           if( hasChannel(pm.channel())==true ) {
             ChannelType& ch = channel(pm.channel());
             if( validSignature(pm.channel(),msg)==true ) {
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
             if( ack == true ) this->sendAck(msg,ch);
           }
           if( ack==false) this->sendNack(msg);
         }
       }
       else if( mtype == AS_MESSAGE_HAVE_DATA ) {
         DPRINTLN(F("HAVE DATA"));
         this->sendAck(msg);
       }
       else if (mtype == AS_MESSAGE_REMOTE_EVENT || mtype == AS_MESSAGE_SENSOR_EVENT) {
         const RemoteEventMsg& pm = msg.remoteEvent();
         uint8_t cdx = channelForPeer(pm.peer());
         bool ack=false;
         if( cdx != 0 ) {
           ChannelType& ch = channel(cdx);
           if( ch.inhibit() == false ) {
             if( validSignature(cdx,msg)==true ) {
               switch( mtype ) {
               case AS_MESSAGE_REMOTE_EVENT:
                 ack = ch.process(pm);
                 break;
               case AS_MESSAGE_SENSOR_EVENT:
                 ack = ch.process(msg.sensorEvent());
                 break;
               }
               if( ack == true ) DeviceType::sendAck(msg,ch);
             }
           }
         }
         if( ack == false ) DeviceType::sendNack(msg);
       }
#ifdef USE_AES
       else if (mtype == AS_MESSAGE_KEY_EXCHANGE ) {
         if( validSignature(msg) == true ) {
           if( this->keystore().exchange(msg.aesExchange())==true ) this->sendAck(msg);
           else this->sendNack(msg);
         }
       }
#endif
       // default - send Nack if answer is requested
       else {
         if( msg.ackRequired() == true ) {
           this->sendNack(msg);
         }
       }
     }
     else {
       DPRINT(F("ignore "));
       msg.dump();
       return false;
     }
     // we always stay awake after valid communication
     this->activity().stayAwake(millis2ticks(500));
     return true;
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
      } else if (c.hasList3() && numlist == 3) {
        return c.getList3(peer);
      } else if (c.hasList4() && numlist == 4) {
        return c.getList4(peer);
      }
    }
    return GenericList();
   }

   void sendPeerEvent (Message& msg,const ChannelType& ch) {
     // we send only to peers if there is no config message pending
     if( cfgChannel != 0xff ) {
       DeviceType::sendPeerEvent(msg,ch);
     }
   }
};


template <class HalType,class ChannelType,int ChannelCount,class List0Type=List0>
class MultiChannelDevice : public ChannelDevice<HalType,ChannelType,ChannelCount,List0Type> {

  ChannelType cdata[ChannelCount];

public:

  typedef ChannelDevice<HalType,ChannelType,ChannelCount,List0Type> DeviceType;

  MultiChannelDevice (const DeviceInfo& i,uint16_t addr) : ChannelDevice<HalType,ChannelType,ChannelCount,List0Type>(i,addr) {
    for( uint8_t i=0; i<ChannelCount; ++i ) {
      this->registerChannel(cdata[i], i+1);
    }
  }

  virtual ~MultiChannelDevice () {}
};

}

#endif
