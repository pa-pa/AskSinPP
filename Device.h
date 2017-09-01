//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "Sign.h"
#include "HMID.h"
#include "Channel.h"
#include "ChannelList.h"
#include "Message.h"
#include "Radio.h"
#include "Led.h"

#define OTA_CONFIG_START 0x7fe0 // start address of 16 byte config data in bootloader
#define OTA_MODEL_START  0x7ff0 // start address of 2 byte model id in bootloader
#define OTA_SERIAL_START 0x7ff2 // start address of 10 byte serial number in bootloader
#define OTA_HMID_START   0x7ffc // start address of 3 byte device id in bootloader

namespace as {

class DeviceType {
public:
enum Types {
  AlarmControl = 0x01,
  Switch = 0x10,
  OutputUnit = 0x12,
  Dimmer = 0x20,
  BlindActuator = 0x30,
  ClimateControl = 0x39,
  Remote = 0x40,
  Sensor = 0x41,
  Swi = 0x42,
  PushButton = 0x43,
  SingleButton = 0x44,
  PowerMeter = 0x51,
  Thermostat = 0x58,
  KFM100 = 0x60,
  THSensor = 0x70,
  ThreeStateSensor = 0x80,
  MotionDetector = 0x81,
  KeyMatic = 0xC0,
  WinMatic = 0xC1,
  TipTronic = 0xC3,
  SmokeDetector = 0xCD,
};
};


template <class HalType>
class Device {

public:
  typedef typename HalType::LedType LedType;
  typedef typename HalType::BatteryType BatteryType;
  typedef typename HalType::RadioType RadioType;

private:
  HalType* hal;
  List0&   list0;
  uint8_t  msgcount;

  HMID    lastdev;
  uint8_t lastmsg;

protected:
  Message     msg;
  KeyStore    kstore;


public:
  Device (uint16_t addr,List0& l) : hal(0), list0(l), msgcount(0), lastmsg(0), kstore(addr) {
    // TODO init seed
  }
  virtual ~Device () {}

  LedType& led ()  { return hal->led; }
  BatteryType& battery ()  { return hal->battery; }
  const BatteryType& battery () const { return hal->battery; }
  RadioType& radio () { return hal->radio; }
  KeyStore& keystore () { return this->kstore; }
  Activity& activity () { return hal->activity; }

  Message& message () { return msg; }

  bool isRepeat(const Message& m) {
    if( m.isRepeated() && lastdev == m.from() && lastmsg == m.count() ) {
      return true;
    }
    // store last message data
    lastdev = m.from();
    lastmsg = m.count();
    return false;
  }

  void setHal (HalType& h) {
    hal = &h;
  }

  uint8_t getConfigByte (uint8_t offset) {
    uint8_t data=0;
#ifdef USE_OTA_BOOTLOADER
    if( offset < 16 ) {
      HalType::pgm_read(&data,OTA_CONFIG_START+offset,1);
    }
#elif defined(DEVICE_CONFIG)
    uint8_t tmp[] = {DEVICE_CONFIG};
    if( offset < sizeof(tmp) ) {
      data = tmp[offset];
    }
#endif
    return data;
  }

  void getDeviceID (HMID& id) {
#ifdef USE_OTA_BOOTLOADER
    HalType::pgm_read((uint8_t*)&id,OTA_HMID_START,sizeof(id));
#else
    id = DEVICE_ID;
#endif
  }

  void getDeviceSerial (uint8_t* serial) {
#ifdef USE_OTA_BOOTLOADER
    HalType::pgm_read((uint8_t*)serial,OTA_SERIAL_START,10);
#else
    memcpy(serial,DEVICE_SERIAL,10);
#endif
  }

  bool isDeviceSerial (const uint8_t* serial) {
    uint8_t tmp[10];
    getDeviceSerial(tmp);
    return memcmp(tmp,serial,10)==0;
  }

  void getDeviceModel (uint8_t* model) {
#ifdef USE_OTA_BOOTLOADER
    HalType::pgm_read(model,OTA_MODEL_START,2);
#else
    uint8_t dm[2] = {DEVICE_MODEL};
    memcpy(model,dm,sizeof(dm));
#endif
  }

  void getDeviceInfo (uint8_t* info) {
    uint8_t di[3] = {DEVICE_INFO};
    memcpy(info,di,sizeof(di));
  }

  HMID getMasterID () {
    return list0.masterid();
  }

  const List0& getList0 () {
    return list0;
  }

  bool pollRadio () {
    uint8_t num = radio().read(msg);
    if( num > 0 ) {
      return process(msg);
    }
    return false;
  }

  uint8_t nextcount () {
    return ++msgcount;
  }

  virtual void configChanged () {}

  virtual bool process(__attribute__((unused)) Message& msg) { return false; }

  bool isBoardcastMsg(Message msg) {
    return msg.isPairSerial();
  }

  bool send(Message& msg,const HMID& to) {
    msg.to(to);
    getDeviceID(msg.from());
    msg.setRpten(); // has to be set always
    bool result = false;
    uint8_t maxsend = 6;
    led().set(LedStates::send);
    while( result == false && maxsend > 0 ) {
      DPRINT(F("<- "));
      msg.dump();
      maxsend--;
      result = radio().write(msg,msg.burstRequired());
      if( result == true && msg.ackRequired() == true && to.valid() == true ) {
        Message response;
        if( (result=waitResponse(msg,response,60)) ) { // 600ms
  #ifdef USE_AES
          if( response.isChallengeAes() == true ) {
            AesChallengeMsg& cm = response.aesChallenge();
            result = processChallenge(msg,cm.challenge(),cm.keyindex());
          }
          else
  #endif
          {
            result = response.isAck();
            // we request wakeup
            // we got the fag to stay awake
            if( msg.isWakeMeUp() || response.isKeepAwake() ) {
              activity().stayAwake(millis2ticks(500));
            }
          }
        }
        DPRINT(F("waitAck: ")); DHEX((uint8_t)result); DPRINTLN(F(""));
      }
    }
    if( result == true ) led().set(LedStates::ack);
    else led().set(LedStates::nack);
    return result;
  }


  void sendAck (Message& msg,uint8_t flag=0x00) {
    msg.ack().init(flag);
    kstore.addAuth(msg);
    send(msg,msg.from());
  }

  void sendAck2 (Message& msg,uint8_t flag=0x00) {
    msg.ack2().init(flag);
    kstore.addAuth(msg);
    send(msg,msg.from());
  }

  void sendNack (Message& msg) {
    msg.nack().init();
    send(msg,msg.from());
  }

  template <class ChannelType>
  void sendAck (Message& msg,ChannelType& ch) {
    msg.ackStatus().init(ch,radio().rssi());
    ch.patchStatus(msg);
    kstore.addAuth(msg);
    send(msg,msg.from());
    ch.changed(false);
  }

  void sendDeviceInfo () {
    sendDeviceInfo(getMasterID(),nextcount());
  }

  void sendDeviceInfo (const HMID& to,uint8_t count) {
    DeviceInfoMsg& pm = msg.deviceInfo();
    pm.init(to,count);
    pm.fill(DEVICE_FIRMWARE,DEVICE_TYPE);
    getDeviceModel(pm.model());
    getDeviceSerial(pm.serial());
    getDeviceInfo(pm.info());
    send(msg,to);
  }

  void sendSerialInfo (const HMID& to,uint8_t count) {
    SerialInfoMsg& pm = msg.serialInfo();
    pm.init(to,count);
    getDeviceSerial(pm.serial());
    send(msg,to);
  }

  template <class ChannelType>
  void sendInfoActuatorStatus (const HMID& to,uint8_t count,ChannelType& ch,bool ack=true) {
    InfoActuatorStatusMsg& pm = msg.infoActuatorStatus();
    pm.init(count,ch,radio().rssi());
    if( ack == false ) {
      pm.clearAck();
    }
    ch.patchStatus(msg);
    send(msg,to);
    ch.changed(false);
  }

  void sendInfoParamResponsePairs(HMID to,uint8_t count,const GenericList& list) {
    InfoParamResponsePairsMsg& pm = msg.infoParamResponsePairs();
    // setup message for maximal size
    pm.init(count);
    uint8_t  current=0;
    uint8_t* buf=pm.data();
    for( int i=0; i<list.getSize(); ++i ) {
      *buf++ = list.getRegister(i);
      *buf++ = list.getByte(i);
      current++;
      if( current == 8 ) {
        // reset to zero
        current=0;
        buf=pm.data();
        if( send(msg,to) == false ) {
          // exit loop in case of error
          break;
        }
      }
    }
    *buf++ = 0;
    *buf++ = 0;
    current++;
    pm.entries(current);
    pm.clearAck();
    send(msg,to);
  }

  template <class ChannelType>
  void sendInfoPeerList (HMID to,uint8_t count,const ChannelType& channel) {
    InfoPeerListMsg& pm = msg.infoPeerList();
    // setup message for maximal size
    pm.init(count);
    uint8_t  current=0;
    uint8_t* buf=pm.data();
    for( uint8_t i=0; i<channel.peers(); ++i ) {
      Peer p = channel.peer(i);
      if( p.valid() == true ) {
        memcpy(buf,&p,sizeof(Peer));
        buf+=sizeof(Peer);
        current++;
        if( current == 4 ) {
          // reset to zero
          current=0;
          buf=pm.data();
          if( send(msg,to) == false ) {
            // exit loop in case of error
            break;
          }
        }
      }
    }
    memset(buf,0,sizeof(Peer));
    current++;
    pm.entries(current);
    pm.clearAck();
    send(msg,to);
  }

  template <class ChannelType>
  void sendPeerEvent (Message& msg,const ChannelType& ch) {
    bool sendtopeer=false;
    for( int i=0; i<ch.peers(); ++i ){
      Peer p = ch.peer(i);
      if( p.valid() == true ) {
        typename ChannelType::List4 l4 = ch.getList4(p);
        msg.burstRequired( l4.burst() );
        send(msg,p);
        sendtopeer = true;
      }
    }
    // if we have no peer - send to master/broadcast
    if( sendtopeer == false ) {
      send(msg,getMasterID());
    }
    // signal that we have send to peer
    hal->sendPeer();
  }

  void writeList (const GenericList& list,const uint8_t* data,uint8_t length) {
    for( uint8_t i=0; i<length; i+=2, data+=2 ) {
      list.writeRegister(*data,*(data+1));
    }
  }

  bool waitForAck(Message& msg,uint8_t timeout) {
    do {
      if( radio().readAck(msg) == true ) {
        return true;
      }
      _delay_ms(10); // wait 10ms
      timeout--;
    }
    while( timeout > 0 );
    return false;
  }

  bool waitResponse(const Message& msg,Message& response,uint8_t timeout) {
    do {
      uint8_t num = radio().read(response);
      if( num > 0 ) {
        DPRINT(F("-> ")); response.dump();
        if( msg.count() == response.count() &&
            msg.to() == response.from() ) {
          return true;
        }
      }
      _delay_ms(10); // wait 10ms
      timeout--;
    }
    while( timeout > 0 );
    return false;
  }

#ifdef USE_AES
  void sendAckAes (Message& msg,const uint8_t* data) {
    msg.ackAes().init(data);
    send(msg,msg.from());
  }

  bool requestSignature(const Message& msg) {
    AesChallengeMsg signmsg;
    signmsg.init(msg,kstore.getIndex());
    kstore.challengeKey(signmsg.challenge(),kstore.getIndex());
    // TODO re-send message handling
    DPRINT(F("<- ")); signmsg.dump();
    radio().write(signmsg,signmsg.burstRequired());
    // read answer
    if( waitForAesResponse(msg.from(),signmsg,60) == true ) {
      AesResponseMsg& response = signmsg.aesResponse();
  //    DPRINT("AES ");DHEX(response.data(),16);
      // fill initial vector with message to sign
      kstore.fillInitVector(msg);
  //    DPRINT("IV ");DHEX(iv,16);
      // decrypt response
      uint8_t* data = response.data();
      aes128_dec(data,&kstore.ctx);
      // xor encrypted data with initial vector
      kstore.applyVector(data);
      // store data for sending ack
      kstore.storeAuth(response.count(),data);
      // decrypt response
      aes128_dec(data,&kstore.ctx);
  //    DPRINT("r "); DHEX(response.data()+6,10);
  //    DPRINT("s "); DHEX(msg.buffer(),10);
      // compare decrypted message with original message
      if( memcmp(data+6,msg.buffer(),10) == 0 ) {
        DPRINTLN(F("Signature OK"));
        return true;
      }
      else {
        DPRINTLN(F("Signature FAILED"));
      }
    }
    else {
      DPRINTLN(F("waitForAesResponse failed"));
    }
    return false;
  }

  bool processChallenge(const Message& msg,const uint8_t* challenge,uint8_t keyidx) {
    if( kstore.challengeKey(challenge,keyidx) == true ) {
      DPRINT("Process Challenge - Key: ");DHEXLN(keyidx);
      AesResponseMsg answer;
      answer.init(msg);
      // fill initial vector with message to sign
      kstore.fillInitVector(msg);
      uint8_t* data = answer.data();
      for( uint8_t i=0; i<6; ++i ) {
        data[i] = (uint8_t)rand();
      }
      memcpy(data+6,msg.buffer(),10); // TODO - check message to short possible
      aes128_enc(data,&kstore.ctx);
      kstore.applyVector(data);
      aes128_enc(data,&kstore.ctx);
      return send(answer,msg.to());
    }
    return false;
  }

  bool waitForAesResponse(const HMID& from,Message& answer,uint8_t timeout) {
    do {
      uint8_t num = radio().read(answer);
      if( num > 0 ) {
        DPRINT(F("-> ")); answer.dump();
        if( answer.isResponseAes() && from == answer.from() ) {
          return true;
        }
      }
      _delay_ms(10); // wait 10ms
      timeout--;
    }
    while( timeout > 0 );
    return false;
  }

#endif
};

}

#endif
