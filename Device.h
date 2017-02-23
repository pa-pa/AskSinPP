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
  HMID  devid;
  HMID  master;
  char serial[11];

  uint8_t firmversion;
  uint8_t model[2];
  uint8_t subtype;
  uint8_t devinfo[3];

  HalType* hal;
  uint8_t msgcount;

  HMID    lastdev;
  uint8_t lastmsg;

protected:
  Message     msg;
  KeyStore    kstore;


public:
  Device (uint16_t addr) : firmversion(0), subtype(0), hal(0), msgcount(0), lastmsg(0), kstore(addr) {
    // TODO init seed
  }
  virtual ~Device () {}

  LedType& led ()  { return hal->led; }
  BatteryType& battery ()  { return hal->battery; }
  RadioType& radio () { return hal->radio; }
  KeyStore& keystore () { return this->kstore; }
  Activity& activity () { return hal->activity; }


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

  void setFirmwareVersion (uint8_t v) {
    firmversion = v;
  }

  void setModel (uint8_t m1, uint8_t m2) {
    model[0] = m1;
    model[1] = m2;
  }

  const uint8_t* const getModel () const {
    return model;
  }

  void setModel (uint16_t address) {
    pgm_read(model,address,sizeof(model));
  }

  void setSubType (uint8_t st) {
    subtype = st;
  }

  void setInfo (uint8_t i1, uint8_t i2, uint8_t i3) {
    devinfo[0] = i1;
    devinfo[1] = i2;
    devinfo[2] = i3;
  }

  void setMasterID (const HMID& id) {
    master = id;
  }

  const HMID& getMasterID () const {
    return master;
  }

  void setDeviceID (const HMID& id) {
    devid=id;
  }

  void setDeviceID (uint16_t address) {
    pgm_read((uint8_t*)&devid,address,sizeof(devid));
  }

  const HMID& getDeviceID () const {
    return devid;
  }

  void setSerial (const char* ser) {
    memcpy(serial,ser,10);
    serial[10] = 0;
  }

  void setSerial (uint16_t address) {
    pgm_read((uint8_t*)serial,address,10);
  }

  const char* getSerial () const {
    return serial;
  }

  template <class ChannelType>
  bool addPeer (ChannelType& ch,const Peer& p) {
    ch.deletepeer(p);
    uint8_t pidx = ch.findpeer();
    if( pidx != 0xff ) {
      ch.peer(pidx,p);
      ch.getList3(pidx).single();
      return true;
    }
    return false;
  }

  template <class ChannelType>
  bool addPeer(ChannelType& ch,const Peer& p1, const Peer& p2) {
    ch.deletepeer(p1);
    ch.deletepeer(p2);
    uint8_t pidx1 = ch.findpeer();
    if( pidx1 != 0xff ) {
      ch.peer(pidx1,p1);
      uint8_t pidx2 = ch.findpeer();
      if( pidx2 != 0xff ) {
        ch.peer(pidx2,p2);
        if( p1.odd() == true ) {
          ch.getList3(pidx1).odd();
          ch.getList3(pidx2).even();
        }
        else {
          ch.getList3(pidx2).odd();
          ch.getList3(pidx1).even();
        }
        return true;
      }
      else {
        // free already stored data
        ch.peer(pidx1,Peer());
      }
    }
    return false;
  }


  bool pollRadio () {
    uint8_t num = radio().read(msg);
    if( num > 0 ) {
      process(msg);
    }
    return num > 0;
  }

  uint8_t nextcount () {
    return ++msgcount;
  }

  virtual void process(Message& msg) {}

  bool isBoardcastMsg(Message msg) {
    return msg.isPairSerial();
  }

  bool send(Message& msg,const HMID& to) {
    msg.to(to);
    msg.from(devid);
    msg.setRpten(); // has to be set always
    bool result = false;
    uint8_t maxsend = 6;
    led().set(StatusLed::send);
    while( result == false && maxsend > 0 ) {
      DPRINT(F("<- "));
      msg.dump();
      maxsend--;
      result = radio().write(msg,msg.burstRequired());
      if( result == true && msg.ackRequired() == true && to.valid() == true ) {
        Message response;
        if( (result=waitResponse(msg,response,30)) ) { // 300ms
  #ifdef USE_AES
          if( response.isChallengeAes() == true ) {
            AesChallengeMsg& cm = response.aesChallenge();
            result = processChallenge(msg,cm.challenge(),cm.keyindex());
          }
          else
  #endif
          {
            result = response.isAck();
          }
        }
        DPRINT(F("waitAck: ")); DHEX((uint8_t)result); DPRINTLN(F(""));
      }
    }
    if( result == true ) led().set(StatusLed::ack);
    else led().set(StatusLed::nack);
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
    pm.fill(firmversion,model,serial,subtype,devinfo);
    send(msg,to);
  }

  void sendSerialInfo (const HMID& to,uint8_t count) {
    SerialInfoMsg& pm = msg.serialInfo();
    pm.init(to,count);
    pm.fill(serial);
    send(msg,to);
  }

  template <class ChannelType>
  void sendInfoActuatorStatus (const HMID& to,uint8_t count,ChannelType& ch) {
    InfoActuatorStatusMsg& pm = msg.infoActuatorStatus();
    pm.init(count,ch,radio().rssi());
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
//        response.dump();
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

  void pgm_read(uint8_t* dest,uint16_t adr,uint8_t size) {
    for( int i=0; i<size; ++i, ++dest ) {
      *dest = pgm_read_byte(adr + i);
    }
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
