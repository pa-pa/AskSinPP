
#ifndef __Message_h__
#define __Message_h__

#include "HMID.h"
#include "Peer.h"
#include "Defines.h"
#include "Debug.h"

//#define MaxDataLen   60						// maximum length of received bytes
#define MaxDataLen   25

namespace as {

// some forward declarations
class ConfigPeerAddMsg;
class ConfigPeerRemoveMsg;
class ConfigPeerListReqMsg;
class ConfigParamReqMsg;
class ConfigStartMsg;
class ConfigEndMsg;
class ConfigWriteIndexMsg;

class AckMsg;
class AckStatusMsg;
class NackMsg;

class InfoActuatorStatusMsg;
class InfoParamResponsePairsMsg;
class InfoPeerListMsg;

class DeviceInfoMsg;
class RemoteEventMsg;
class ActionSetMsg;

class Message {
public:

  enum Flags {
    WKUP = 0x01,   // send initially to keep the device awake
    WKMEUP = 0x02, // awake - hurry up to send messages
    CFG = 0x04,    // Device in Config mode
    BURST = 0x10,  // set if burst is required by device
    BIDI = 0x20,   // response is expected
    RPTED = 0x40,  // repeated (repeater operation)
    RPTEN = 0x80,  // set in every message. Meaning?
  };

protected:
  uint8_t         len;					  // message length
	uint8_t         cnt;					  // counter, if it is an answer counter has to reflect the answered message, otherwise own counter has to be used
	uint8_t         flag;				  // see structure of message flags
	uint8_t         typ;					  // type of message
	HMID            fromID;				  // sender ID
	HMID            toID;	          // receiver id, broadcast for 0
	uint8_t         comm;					  // type of message
	uint8_t         subcom;				  // type of message
	uint8_t         pload[MaxDataLen]; // payload

public:
	Message () : len(0), cnt(0), flag(0), typ(0), comm(0), subcom(0) {}

	void clear () {
	  len = 0;
	}

	uint8_t* buffer () {
	  return &cnt;
	}

  const uint8_t* buffer () const {
    return &cnt;
  }

	uint8_t buffersize () {
	  return sizeof(Message)-1;
	}

	uint8_t* data () {
	  return pload;
	}

  const uint8_t* data () const {
    return pload;
  }

	uint8_t datasize () const {
	  return len >= 11 ? len - 11 : 0;
	}

	void append (uint8_t data) {
	  *(buffer() + len) = data;
	  len++;
	}

  void append (void* data,uint8_t l) {
    memcpy(buffer()+len,data,l);
    len += l;
  }

  void append (uint8_t d1, uint8_t d2, uint8_t d3) {
    uint8_t* ptr = buffer() + len;
    *ptr = d1;
    *(ptr+1) = d2;
    *(ptr+2) = d3;
    len += 3;
  }

  void append (uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4) {
    uint8_t* ptr = buffer() + len;
    *ptr = d1;
    *(ptr+1) = d2;
    *(ptr+2) = d3;
    *(ptr+3) = d4;
    len += 4;
  }

  void initWithCount(uint8_t length, uint8_t typ, uint8_t flags, uint8_t comm) {
    this->len = length;
    this->typ = typ;
    this->flag = flags;
    this->comm = comm;
  }

  void init(uint8_t length, uint8_t cnt, uint8_t typ, uint8_t flags, uint8_t comm, uint8_t sub) {
    initWithCount(length,typ,flags,comm);
    this->cnt = cnt;
    this->subcom = sub;
  }

  void length (uint8_t l) {
    len = l;
  }

  uint8_t length () const {
    return len;
  }

  void count (uint8_t c) {
    cnt = c;
  }

  uint8_t count () const {
    return cnt;
  }

  void from(const HMID& hmid) {
    fromID = hmid;
  }

  const HMID& from () const {
    return fromID;
  }

  void to(const HMID& hmid) {
    toID = hmid;
  }

  const HMID& to () const {
    return toID;
  }

  void type (uint8_t t) {
    typ=t;
  }

  uint8_t type () const {
    return typ;
  }

  void flags (uint8_t f) {
    flag = f;
  }

  uint8_t flags () const {
    return flag;
  }

  void command (uint8_t c) {
    comm = c;
  }

  uint8_t command () const {
    return comm;
  }

  void subcommand (uint8_t c) {
    subcom = c;
  }

  uint8_t subcommand () const {
    return subcom;
  }

  void decode () {
    decode(buffer(),length());
  }

  void decode(uint8_t *buf, uint8_t len) {
    uint8_t prev = buf[0];
    buf[0] = (~buf[0]) ^ 0x89;
    uint8_t i, t;
    for (i=1; i<len-1; i++) {
      t = buf[i];
      buf[i] = (prev + 0xdc) ^ buf[i];
      prev = t;
    }
    buf[i] ^= buf[1];
  }

  void encode () {
    encode(buffer(),length());
  }

  void encode(uint8_t *buf,uint8_t len) {
    buf[0] = (~buf[0]) ^ 0x89;
    uint8_t buf2 = buf[1];
    uint8_t prev = buf[0];
    uint8_t i;
    for (i=1; i<len-1; i++) {
      prev = (prev + 0xdc) ^ buf[i];
      buf[i] = prev;
    }
    buf[i] ^= buf2;
  }

  void dump () const {
    DHEX(length());
    DPRINT(F(" "));
    DHEX(buffer(),length());
  }

  bool ackRequired () const {
    return (flag & BIDI) == BIDI;
  }

  bool burstRequired () const {
    return (flag & BURST) == BURST;
  }

  void burstRequired (bool value) {
    if( value == true ) {
      flag |= BURST;
    }
    else {
      flag &= ~BURST;
    }
  }

  bool isPairSerial () const {
    return typ==0x01 && subcom==0x0a;
  }

  bool isAck () const {
    return typ==0x02 && (comm & 0x80) == 0;
  }

  bool isNack () const {
    return typ==0x02 && (comm & 0x80) == 0x80;
  }

  // cast to specific read-only message types
  const ConfigPeerAddMsg& configPeerAdd () const { return *(ConfigPeerAddMsg*)this; }
  const ConfigPeerRemoveMsg& configPeerRemove () const { return *(ConfigPeerRemoveMsg*)this; }
  const ConfigPeerListReqMsg& configPeerListReq () const { return *(ConfigPeerListReqMsg*)this; }
  const ConfigParamReqMsg& configParamReq () const { return *(ConfigParamReqMsg*)this; }
  const ConfigStartMsg& configStart () const { return *(ConfigStartMsg*)this; }
  const ConfigEndMsg& configEnd () const { return *(ConfigEndMsg*)this; }
  const ConfigWriteIndexMsg& configWriteIndex () const { return *(ConfigWriteIndexMsg*)this; }

  const RemoteEventMsg& remoteEvent () const { return *(RemoteEventMsg*)this; }
  const ActionSetMsg& action () const { return *(ActionSetMsg*)this; }

  // cast to write message types
  AckMsg& ack () { return *(AckMsg*)this; }
  AckStatusMsg& ackStatus () { return *(AckStatusMsg*)this; }
  NackMsg& nack () { return *(NackMsg*)this; }

  InfoActuatorStatusMsg& infoActuatorStatus () { return *(InfoActuatorStatusMsg*)this; }
  InfoParamResponsePairsMsg& infoParamResponsePairs () { return *(InfoParamResponsePairsMsg*)this; }
  InfoPeerListMsg& infoPeerList () { return *(InfoPeerListMsg*)this; }

  DeviceInfoMsg& deviceInfo () { return *(DeviceInfoMsg*)this; }
};


class ConfigMsg : public Message {
protected:
  ConfigMsg () {}
public:
  uint8_t channel () const { return command(); }
};

class ConfigPeerAddMsg : public ConfigMsg {
protected:
  ConfigPeerAddMsg () {}
public:
  const Peer& peer1 () const { return *((const Peer*)data()); }
  Peer peer2 () const { return Peer(peer1(),*(data()+sizeof(Peer))); }
  uint8_t peers () const { return *(data()+sizeof(Peer))!=0 ? 2 : 1; }
};

class ConfigPeerRemoveMsg : public ConfigPeerAddMsg {
protected:
  ConfigPeerRemoveMsg () {}
};

class ConfigPeerListReqMsg : public ConfigMsg {
protected:
  ConfigPeerListReqMsg () {}
};

class ConfigParamReqMsg : public ConfigMsg {
protected:
  ConfigParamReqMsg () {}
public:
  const Peer& peer () const { return *((const Peer*)data()); }
  uint8_t list () const { return *(data()+sizeof(Peer)); }
};

class ConfigStartMsg : public ConfigParamReqMsg {
protected:
  ConfigStartMsg () {}
};

class ConfigEndMsg : public ConfigMsg {
protected:
  ConfigEndMsg () {}
};

class ConfigWriteIndexMsg : public ConfigMsg {
protected:
  ConfigWriteIndexMsg () {}
};

class RemoteEventMsg : public Message {
protected:
  RemoteEventMsg() {}
public:
  Peer peer () const { return Peer(from(),command() & 0x3f); }
  uint8_t counter () const { return subcommand(); }
  bool isLong () const { return (command() & 0x40) == 0x40; }
};

class ActionSetMsg : public Message {
protected:
  ActionSetMsg() {}
public:
  uint8_t channel () const { return subcommand(); }
  uint8_t value () const { return *data(); }
  uint16_t delay () const {
    uint16_t dly = 0xffff;
    if( datasize() >= 5) {
      dly = (*(data()+3) << 8) + *(data()+4);
    }
    if( dly == 0 ) dly = 0xffff;
    return dly;
  }
};



class AckMsg : public Message {
public:
  void init() {
    initWithCount(0x0a,0x02,0x00,0x00);
  }
};

class AckStatusMsg : public Message {
public:
  template <class ChannelType>
  void init(const ChannelType& ch,uint8_t rssi) {
    initWithCount(0x0e,0x02,0x00,0x01);
    subcom = ch.number();
    pload[0] = ch.status();
    pload[1] = ch.flags();
    pload[2] = rssi;
  }
};

class NackMsg : public Message {
public:
  void init() {
    initWithCount(0x0a,0x02,0x00,0x80);
  }
};


class InfoActuatorStatusMsg : public Message {
public:
  template <class ChannelType>
  void init (uint8_t count,const ChannelType& ch,uint8_t rssi) {
    Message::init(0x0e,count,0x10,Message::BIDI,0x06,ch.number());
    pload[0] = ch.status();
    pload[1] = ch.flags();
    pload[2] = rssi;
  }
};

class InfoParamResponsePairsMsg : public Message {
public:
  void init (uint8_t count) {
    initWithCount(0x0b-1+(8*2),0x10,Message::BIDI,0x02);
    cnt = count;
  }
  uint8_t* data() { return Message::data()-1; }
  void entries (uint8_t num) { length(0x0b-1+(num*2)); };
};

class InfoPeerListMsg : public Message {
public:
  void init (uint8_t count) {
    initWithCount(0x0b-1+(4*sizeof(Peer)),0x10,Message::BIDI,0x01);
    cnt = count;
  }
  uint8_t* data() { return Message::data()-1; }
  void entries (uint8_t num) { length(0x0b-1+(num*sizeof(Peer))); };
};

class DeviceInfoMsg : public Message {
public:
  void init (const HMID& to,uint8_t count) {
    Message::init(0x1a,count,0x00, to.valid() ? Message::BIDI : 0x00,0x00,0x00);
  }
  uint8_t* data() { return Message::data()-2; }
  void fill(uint8_t firmversion,uint8_t modelid[2],const char* serial,uint8_t subtype,uint8_t devinfo[3]) {
    uint8_t* buf = data();
    *buf = firmversion;
    memcpy(buf+1,modelid,2);
    memcpy(buf+3,serial,10);
    *(buf+13) = subtype;
    memcpy(buf+14,devinfo,3);
  }
};

}

#endif
