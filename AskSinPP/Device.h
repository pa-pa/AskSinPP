
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "HMID.h"
#include "Channel.h"
#include "Message.h"
#include "CC1101.h"

class Device {
  HMID  devid;
  HMID  master;
  char serial[11];

  uint8_t firmversion;
  uint8_t model[2];
  uint8_t subtype;
  uint8_t devinfo[3];

  CC1101* radio;
  uint8_t msgcount;

protected:
  Message msg;

public:
  virtual ~Device () {}

  void setRadio(CC1101& r) {
    msgcount=0;
    radio = &r;
  }

  CC1101& getRadio () {
    return *radio;
  }

  void setFirmwareVersion (uint8_t v) {
    firmversion = v;
  }

  void setModel (uint8_t m1, uint8_t m2) {
    model[0] = m1;
    model[1] = m2;
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

  const HMID& getDeviceID () const {
    return devid;
  }

  void setSerial (const char* ser) {
    memcpy(serial,ser,10);
    serial[10] = 0;
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


  void pollRadio () {
    uint8_t num = getRadio().read(msg);
    if( num > 0 ) {
      process(msg);
    }
  }

  uint8_t nextcount () {
    return ++msgcount;
  }

  virtual void process(Message& msg);

  bool isBoardcastMsg(Message msg) {
    return msg.isPairSerial();
  }

  bool send(Message& msg,const HMID& to);

  bool waitForAck(Message& msg,uint8_t timeout);

  void sendAck (Message& msg) {
    //msg.initWithCount(0x0a,0x02,0x00,0x00);
    msg.ack().init();
    send(msg,msg.from());
  }

  void sendNack (Message& msg) {
    //msg.initWithCount(0x0a,0x02,0x00,0x80);
    msg.nack().init();
    send(msg,msg.from());
  }

  template <class ChannelType>
  void sendAck (Message& msg,ChannelType& ch) {
    /*
    msg.initWithCount(0x0e,0x02,0x00,0x01);
    msg.subcommand(ch.number());
    *msg.data() = ch.status();
    // TODO battery status
    *(msg.data()+1) = ch.flags();
    *(msg.data()+2) = radio->rssi();
    */
    msg.ackStatus().init(ch,radio->rssi());
    send(msg,msg.from());
    ch.changed(false);
  }

  void sendDeviceInfo () {
    sendDeviceInfo(HMID::boardcast,nextcount());
  }

  void sendDeviceInfo (const HMID& to,uint8_t count);

  template <class ChannelType>
  void sendInfoActuatorStatus (const HMID& to,uint8_t count,ChannelType& ch) {
    /*
    msg.init(0x0b+3,count,0x10,Message::BIDI,0x06,ch.number());
    *msg.data() = ch.status();
    // TODO battery status
    *(msg.data()+1) = ch.flags();
    *(msg.data()+2) = radio->rssi();
    */
    InfoActuatorStatusMsg& pm = msg.infoActuatorStatus();
    pm.init(count,ch,radio->rssi());
    send(msg,to);
    ch.changed(false);
  }

  template <class ListType>
  void sendInfoParamResponsePairs(HMID to,uint8_t count,const ListType& list) {
    // setup message for maximal size
    // msg.initWithCount(0x0b-1+(8*2),0x10,Message::BIDI,0x02);
    // msg.count(count);
    InfoParamResponsePairsMsg& pm = msg.infoParamResponsePairs();
    pm.init(count);
    uint8_t  current=0;
    uint8_t* buf=pm.data();
    for( int i=0; i<list.size(); ++i ) {
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
    // msg.initWithCount(0x0b-1+(current*2),0x10,Message::BIDI,0x02);
    // msg.count(count);
    send(msg,to);
  }

  template <class ChannelType>
  void sendInfoPeerList (HMID to,uint8_t count,const ChannelType& channel) {
    // setup message for maximal size
    // msg.initWithCount(0x0b-1+(4*sizeof(Peer)),0x10,Message::BIDI,0x01);
    //msg.count(count);
    InfoPeerListMsg& pm = msg.infoPeerList();
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
    // msg.initWithCount(0x0b-1+(current*sizeof(Peer)),0x10,Message::BIDI,0x01);
    // msg.count(count);
    send(msg,to);
  }
};

#endif
