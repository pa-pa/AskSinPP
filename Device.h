
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "HMID.h"
#include "Channel.h"
#include "ChannelList.h"
#include "Message.h"
#include "CC1101.h"

namespace as {

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

  void sendAck (Message& msg) {
    msg.ack().init();
    send(msg,msg.from());
  }

  void sendNack (Message& msg) {
    msg.nack().init();
    send(msg,msg.from());
  }

  template <class ChannelType>
  void sendAck (Message& msg,ChannelType& ch) {
    msg.ackStatus().init(ch,radio->rssi());
    send(msg,msg.from());
    ch.changed(false);
  }

  void sendDeviceInfo () {
    sendDeviceInfo(HMID::boardcast,nextcount());
  }

  void sendDeviceInfo (const HMID& to,uint8_t count) {
    DeviceInfoMsg& pm = msg.deviceInfo();
    pm.init(to,count);
    pm.fill(firmversion,model,serial,subtype,devinfo);
    send(msg,to);
  }

  template <class ChannelType>
  void sendInfoActuatorStatus (const HMID& to,uint8_t count,ChannelType& ch) {
    InfoActuatorStatusMsg& pm = msg.infoActuatorStatus();
    pm.init(count,ch,radio->rssi());
    send(msg,to);
    ch.changed(false);
  }

  void sendInfoParamResponsePairs(HMID to,uint8_t count,const GenericList& list) {
    // setup message for maximal size
    // msg.initWithCount(0x0b-1+(8*2),0x10,Message::BIDI,0x02);
    // msg.count(count);
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
    send(msg,to);
  }

  void writeList (const GenericList& list,const uint8_t* data,uint8_t length) {
    for( uint8_t i=0; i<length; i+=2, data+=2 ) {
      list.writeRegister(*data,*(data+1));
    }
  }

  bool waitForAck(Message& msg,uint8_t timeout) {
    do {
      if( radio->readAck(msg) == true ) {
        return true;
      }
      delay(10); // wait 10ms
      timeout--;
    }
    while( timeout > 0 );
    return false;
  }

};

}

#endif
