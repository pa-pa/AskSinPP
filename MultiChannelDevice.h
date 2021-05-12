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

#if ARDUINO_ARCH_AVR
#include <avr/wdt.h>
#endif

namespace as {

extern void(* resetFunc) (void);

#define REPLAY_NO   0
#define REPLAY_ACK  1
#define REPLAY_NACK 2

template <class HalType,class ChannelType,int ChannelCount,class List0Type=List0>
class ChannelDevice : public Device<HalType,List0Type> {

  List0Type    list0;
  ChannelType* devchannels[ChannelCount];
  uint8_t      cfgChannel;
  GenericList  cfgList;

public:

  typedef Device<HalType,List0Type> DeviceType;

  ChannelDevice (const DeviceInfo& i,uint16_t addr) : DeviceType(i,addr,list0,ChannelCount), list0(addr + this->keystore().size()), cfgChannel(0xff) {}

  virtual ~ChannelDevice () {}

  void registerChannel(ChannelType& ch,uint8_t num) {
    if( num >= 1 && num <= ChannelCount) {
      devchannels[num-1] = &ch;
    }
  }

  void layoutChannels () {
    uint16_t addr = list0.address() + list0.size();
    for( uint8_t i=0; i<this->channels(); ++i ) {
      devchannels[i]->setup(this,i+1,addr);
      addr += devchannels[i]->size();
    }
  }

  void channels (uint8_t num) {
    DeviceType::channels(min(num,(uint8_t)ChannelCount));
  }

  uint8_t channels () const {
    return DeviceType::channels();
  }

  void dumpSize () {
    DPRINT(F("Address Space: "));DDEC(this->keystore().address());DPRINT(F(" - "));DDECLN(getUserStorage().getAddress());
  }

  // get object to access free EEPROM after device data
  UserStorage getUserStorage () {
    ChannelType& ch = channel(this->channels());
    return UserStorage(ch.address() + ch.size());
  }

  uint16_t checksum () {
    uint16_t crc = 0;
#ifndef NOCRC
    // size of keystore data
    crc = HalType::crc16(crc,DeviceType::keystore().size());
    // add register of list0
    for( uint8_t i=0; i<list0.size(); ++i ) {
      crc = HalType::crc16(crc,list0.getRegister(i));
    }
    // add number of channels
    for( uint8_t c=1; c<=this->channels(); ++c ) {
      ChannelType& ch = channel(c);
      // add register list 1
      GenericList l = ch.getList1();
      for( uint8_t i=0; i<l.getSize(); ++i ) {
        crc = HalType::crc16(crc,l.getRegister(i));
      }
      // add register list 3
      l = ch.getList3((uint8_t)0);
      for( uint8_t i=0; i<l.getSize(); ++i ) {
        crc = HalType::crc16(crc,l.getRegister(i));
      }
      // add register list 4
      l = ch.getList4((uint8_t)0);
      for( uint8_t i=0; i<l.getSize(); ++i ) {
        crc = HalType::crc16(crc,l.getRegister(i));
      }
      // add number of peers
      crc = HalType::crc16(crc,ch.peers());
    }
#endif
    return crc;
  }

  List0Type& getList0 () {
    return list0;
  }

  bool init (HalType& hal) {
    layoutChannels();
    dumpSize();
    // first initialize EEProm if needed
    bool first = storage().setup(checksum());
    if( first == true ) {
      firstinit();
      storage().store();
    }
    this->keystore().init();
    this->setHal(hal);
    HMID id;
    this->getDeviceID(id);
    hal.init(id);
    hal.config(this->getConfigArea());
    return first;
  }

  void initDone () {
    // trigger initial config changed - to allow scan/caching of list data
    this->hasConfigChanged(true);
    this->configChanged();
    for( uint8_t cdx=1; cdx<=channels(); ++cdx ) {
      channel(cdx).configChanged();
    }
    // trigger save of storage
    storage().store();
  }

  void firstinit () {
    this->keystore().defaults(); // init aes key infrastructure
    list0.defaults();
    for( uint8_t i=0; i<this->channels(); ++i ) {
      devchannels[i]->firstinit();
    }
  }

  void reset () {
    if( getList0().localResetDisable() == false ) {
      DPRINTLN(F("RESET"));
      storage().reset();
      storage().store();
  #if ARDUINO_ARCH_AVR
      resetFunc();
  #elif ARDUINO_ARCH_STM32F1
      nvic_sys_reset();
  #elif defined (ARDUINO_ARCH_STM32) && defined (STM32L1xx) 
      NVIC_SystemReset();
  #elif ARDUINO_ARCH_ESP32
      ESP.restart();
  #endif
    }
  }

  void bootloader () {
    DPRINTLN(F("BOOTLOADER"));
#if ARDUINO_ARCH_AVR
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
    for( uint8_t i=1; i<=this->channels(); ++i ) {
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
    for( uint8_t i=1; i<=this->channels(); ++i) {
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
    if( (ch==0 && aesActive()) || (this->hasChannel(ch)==true && channel(ch).aesActive()==true) ) {
      return this->requestSignature(msg);
    }
#endif
    return true;
  }

   bool process(Message& msg) {
     uint8_t answer = REPLAY_NO;
     ChannelType* ch = 0;
     HMID devid;
     this->getDeviceID(devid);
     if( msg.to() == devid || this->isBroadcastMsg(msg) ) {
       // we got a message - we do not answer before 100ms
       this->radio().setSendTimeout(); // use default value from radio
       DPRINT(F("-> "));
       msg.dump();
       // ignore repeated messages
       if( this->isRepeat(msg) == true ) {
         return false;
       }
       // start processing the message
       uint8_t mtype = msg.type();
       uint8_t mcomm = msg.command();
       uint8_t msubc = msg.subcommand();
       if( mtype == AS_MESSAGE_CONFIG ) {
         // PAIR_SERIAL
         if( msubc == AS_CONFIG_PAIR_SERIAL && this->isDeviceSerial(msg.data())==true ) {
           this->led().set(LedStates::pairing);
           this->activity().stayAwake( seconds2ticks(20) ); // 20 seconds
           this->sendDeviceInfo(msg.from(),msg.length());
         }
         // CONFIG_PEER_ADD
         else if ( msubc == AS_CONFIG_PEER_ADD ) {
           const ConfigPeerAddMsg& pm = msg.configPeerAdd();
           bool success = false;
           if( this->hasChannel(pm.channel()) == true ) {
             if( validSignature(pm.channel(),msg) == true ) {
               ch = &channel(pm.channel());
               if( pm.peers() == 1 ) {
                 success = ch->peer(pm.peer1());
               }
               else {
                 success = ch->peer(pm.peer1(),pm.peer2());
               }
             }
           }
           if( success == true ) {
             this->hasConfigChanged(true);
             ch->configChanged();
             storage().store();
             answer = REPLAY_ACK;
           }
           else {
             answer = REPLAY_NACK;
           }
         }
         // CONFIG_PEER_REMOVE
         else if ( msubc == AS_CONFIG_PEER_REMOVE ) {
           const ConfigPeerRemoveMsg& pm = msg.configPeerRemove();
           bool success = false;
           if( this->hasChannel(pm.channel()) == true ) {
             if( validSignature(pm.channel(),msg) == true ) {
               ch = &channel(pm.channel());
               success = ch->deletepeer(pm.peer1());
               if( pm.peers() == 2 ) {
                 success &= ch->deletepeer(pm.peer2());
               }
             }
           }
           if( success == true ) {
             this->hasConfigChanged(true);
             ch->configChanged();
             storage().store();
             answer = REPLAY_ACK;
           }
           else {
             answer = REPLAY_NACK;
           }
         }
         // CONFIG_PEER_LIST_REQ
         else if( msubc == AS_CONFIG_PEER_LIST_REQ ) {
           const ConfigPeerListReqMsg& pm = msg.configPeerListReq();
           if( this->hasChannel(pm.channel()) == true ) {
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
             answer = REPLAY_NACK;
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
             answer = REPLAY_ACK;
           }
           else {
             answer = REPLAY_NACK;
           }
         }
         // CONFIG_END
         else if( msubc == AS_CONFIG_END ) {
           this->hasConfigChanged(true);
           if( cfgList.address() == list0.address() ) {
             this->led().set(LedStates::nothing);
             this->configChanged();
           }
           else {
             // signal list update to channel
             channel(cfgChannel).configChanged();
           }
           cfgChannel = 0xff;
           storage().store();
           // TODO cancel alarm
           this->sendAck(msg,Message::WKMEUP);
         }
         else if( msubc == AS_CONFIG_WRITE_INDEX ) {
           const ConfigWriteIndexMsg& pm = msg.configWriteIndex();
           if( validSignature(pm.channel(),msg)==true ) {
             if( cfgChannel == pm.channel() && cfgList.valid() == true ) {
               this->writeList(cfgList,pm.data(),pm.datasize());
             }
             answer = REPLAY_ACK;
           }
           else {
             answer = REPLAY_NACK;
           }
         }
         else if( msubc == AS_CONFIG_SERIAL_REQ ) {
           this->sendSerialInfo(msg.from(),msg.count());
         }
         else {
           answer = REPLAY_NACK;
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
#ifndef SENSOR_ONLY
         else {
           const ActionMsg& pm = msg.action();
           if( this->hasChannel(pm.channel())==true ) {
             ch = &channel(pm.channel());
             if( validSignature(pm.channel(),msg)==true ) {
               switch( mcomm ) {
               case AS_ACTION_INHIBIT_OFF:
                 ch->inhibit(false);
                 answer = REPLAY_ACK;
                 break;
               case AS_ACTION_INHIBIT_ON:
                 ch->inhibit(true);
                 answer = REPLAY_ACK;
                 break;
               case AS_ACTION_STOP_CHANGE:
                 ch->stop();
                 answer = REPLAY_ACK;
                 break;
               case AS_ACTION_SET:
                 if( ch->inhibit() == false ) {
                   answer = ch->process(msg.actionSet()) ? REPLAY_ACK : REPLAY_NACK;
                 }
                 break;
               case AS_ACTION_COMMAND:
                 if( ch->inhibit() == false ) {
                   answer = ch->process(msg.actionCommand()) ? REPLAY_ACK : REPLAY_NACK;
                 }
                 break;
               }
             }
           }
         }
#endif
       }
       else if( mtype == AS_MESSAGE_HAVE_DATA ) {
         DPRINTLN(F("HAVE DATA"));
         answer = REPLAY_ACK;
       }
#ifndef SENSOR_ONLY
       else if (mtype == AS_MESSAGE_SWITCH_EVENT) {
         RemoteEventMsg& pm = msg.switchSim().toEventMsg();
         //DPRINT("X> "); pm.dump();

         for (uint8_t cdx = 1; cdx <= this->channels(); ++cdx) {
           ChannelType* c = &channel(cdx);
           //DPRINT("cnl: "); DPRINTLN(cdx);
           if (c->inhibit() == false && c->has(pm.peer()) == true) {
             c->process(pm);
           }
         }
         answer = REPLAY_ACK;
       }
       else if (mtype == AS_MESSAGE_REMOTE_EVENT || mtype == AS_MESSAGE_SENSOR_EVENT) {
         answer = REPLAY_NACK;
         const RemoteEventMsg& pm = msg.remoteEvent();
         uint8_t processed = 0;
         for( uint8_t cdx=1; cdx<=this->channels(); ++cdx ) {
           ChannelType* c = &channel(cdx);
           if( c->inhibit() == false && c->has(pm.peer()) == true ) {
             if( processed > 0 || validSignature(cdx,msg) == true ) {
               ++processed;
               ch = c;
               switch( mtype ) {
               case AS_MESSAGE_REMOTE_EVENT:
                 ch->process(pm);
                 break;
               case AS_MESSAGE_SENSOR_EVENT:
                 ch->process(msg.sensorEvent());
                 break;
               }
               answer = REPLAY_ACK;
             }
           }
         }
         if( processed > 1 ) {
           // we had more than one channel processed
           // clear channel, so we only send an ACK
           ch = 0;
         }
       }
#endif
#ifdef USE_AES
       else if (mtype == AS_MESSAGE_KEY_EXCHANGE ) {
         if( validSignature(msg) == true ) {
           if( this->keystore().exchange(msg.aesExchange())==true ) answer = REPLAY_ACK;
           else answer = REPLAY_NACK;
         }
       }
#endif
       // default - send Nack if answer is requested
       else {
         answer = REPLAY_NACK;
       }
     }
     else {
#ifndef HIDE_IGNORE_MSG
       DPRINT(F("ignore "));
       msg.dump();
#endif
       return false;
     }
     // send ack/nack
     if( msg.ackRequired() == true && msg.to() == devid ) {
       if( answer == REPLAY_ACK ) {
         if( ch != 0 ) this->sendAck(msg, *ch);
         else this->sendAck(msg);
       }
       else if( answer == REPLAY_NACK ) {
         this->sendNack(msg);
       }
     }
     // we always stay awake after valid communication
     this->activity().stayAwake(millis2ticks(500));
     return true;
   }

   GenericList findList(uint8_t ch,const Peer& peer,uint8_t numlist) {
    if (numlist == 0) {
      return list0;
    } else if (this->hasChannel(ch) == true) {
      ChannelType& c = channel(ch);
      if (numlist == 1) {
        return c.getList1();
      } else if (c.hasList2() && numlist == 2) {
        return c.getList2();
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
