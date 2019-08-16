//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __Message_h__
#define __Message_h__

#include "HMID.h"
#include "Peer.h"
#include "Defines.h"
#include "Debug.h"

//#define MaxDataLen   60						// maximum length of received bytes
#define MaxDataLen   30

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
class Ack2Msg;
class AckStatusMsg;
class NackMsg;
class AckAesMsg;
class AesChallengeMsg;
class AesResponseMsg;
class AesExchangeMsg;

class InfoActuatorStatusMsg;
class InfoParamResponsePairsMsg;
class InfoPeerListMsg;

class SerialInfoMsg;
class DeviceInfoMsg;
class RemoteEventMsg;
class SensorEventMsg;
class ActionMsg;
class ActionSetMsg;
class ActionCommandMsg;

class ValuesMsg;

class Message {
public:

  enum Flags {
    WKUP = 0x01,   // send initially to keep the device awake
    WKMEUP = 0x02, // awake - hurry up to send messages
    BCAST = 0x04,  // broadcast message
    BURST = 0x10,  // this message was sent with burst
    BIDI = 0x20,   // response is expected
    RPTED = 0x40,  // repeated (repeater operation)
    RPTEN = 0x80,  // this message is allowed to be repeated
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

  HMID& from () {
    return fromID;
  }

  void to(const HMID& hmid) {
    toID = hmid;
    if( hmid == HMID::broadcast ) {
      flag |= BCAST;
      flag &= ~BIDI;
    }
  }

  const HMID& to () const {
    return toID;
  }

  HMID& to () {
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
    DHEX(count());
    DPRINT(F(" "));
    DHEX(flags());
    DPRINT(F(" "));
    DHEX(type());
    DPRINT(F(" "));
    from().dump();
    DPRINT(F(" "));
    to().dump();
    DPRINT(F(" "));
    DHEX(buffer()+9,length()-9);
    DPRINT(F(" - "));
    DDECLN((uint32_t)millis());
  }

  void setRepeated () {
    flag |= RPTED;
  }

  void setRpten () {
    flag |= RPTEN;
  }

  void setAck () {
    flag |= BIDI;
  }

  void clearAck () {
    flag &= ~BIDI;
  }

  bool isKeepAwake () const {
    return (flag & WKUP) == WKUP;
  }

  bool isWakeMeUp () const {
    return (flag & WKMEUP) == WKMEUP;
  }

  void setWakeMeUp () {
    flag |= WKMEUP;
  }

  void setBroadcast () {
    flag |= BCAST;
  }

  bool isBroadcast () const {
    return (flag & BCAST) == BCAST;
  }

  bool isRepeated () const {
    return (flag & RPTED) == RPTED;
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
    return typ==AS_MESSAGE_CONFIG && subcom==AS_CONFIG_PAIR_SERIAL;
  }

  bool isAck () const {
    return typ==AS_MESSAGE_RESPONSE && (comm & AS_RESPONSE_NACK) == AS_RESPONSE_ACK;
  }

  bool isNack () const {
    return typ==AS_MESSAGE_RESPONSE && (comm & AS_RESPONSE_NACK) == AS_RESPONSE_NACK;
  }

  bool isResponseAes () const {
    return typ==AS_MESSAGE_RESPONSE_AES;
  }

  bool isChallengeAes () const {
    return typ==AS_MESSAGE_RESPONSE && (comm & AS_RESPONSE_AES_CHALLANGE) == AS_RESPONSE_AES_CHALLANGE;
  }

  bool isRemoteEvent () const {
    return typ==AS_MESSAGE_REMOTE_EVENT;
  }

  bool isSensorEvent () const {
    return typ==AS_MESSAGE_SENSOR_EVENT;
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
  const SensorEventMsg& sensorEvent () const { return *(SensorEventMsg*)this; }
  const ActionMsg& action () const { return *(ActionMsg*)this; }
  const ActionSetMsg& actionSet () const { return *(ActionSetMsg*)this; }
  const ActionCommandMsg& actionCommand () const { return *(ActionCommandMsg*)this; }

  // cast to write message types
  AckMsg& ack () { return *(AckMsg*)this; }
  Ack2Msg& ack2 () { return *(Ack2Msg*)this; }
  AckStatusMsg& ackStatus () { return *(AckStatusMsg*)this; }
  NackMsg& nack () { return *(NackMsg*)this; }
  AckAesMsg& ackAes () { return *(AckAesMsg*)this; }
  AesChallengeMsg& aesChallenge () { return *(AesChallengeMsg*)this; }
  AesResponseMsg& aesResponse () { return *(AesResponseMsg*)this; }
  AesExchangeMsg& aesExchange () { return *(AesExchangeMsg*)this; }

  InfoActuatorStatusMsg& infoActuatorStatus () { return *(InfoActuatorStatusMsg*)this; }
  InfoParamResponsePairsMsg& infoParamResponsePairs () { return *(InfoParamResponsePairsMsg*)this; }
  InfoPeerListMsg& infoPeerList () { return *(InfoPeerListMsg*)this; }

  DeviceInfoMsg& deviceInfo () { return *(DeviceInfoMsg*)this; }
  SerialInfoMsg& serialInfo () { return *(SerialInfoMsg*)this; }

  ValuesMsg& values () { return *(ValuesMsg*)this; }
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
  // if channel of peer2 == 0 or
  // both channels are the same then we peer single mode
  uint8_t peers () const {
    Peer p2 = peer2();
    return (p2.channel() == 0) || (p2 == peer1()) ? 1 : 2;
  }
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
  void init(uint8_t msgcnt,uint8_t ch,uint8_t counter,bool lg,bool lowbat) {
    uint8_t flags = lg ? 0x40 : 0x00;
    if( lowbat == true ) {
      flags |= 0x80; // low battery
    }
    Message::init(0xb,msgcnt,0x40, BIDI|WKMEUP,(ch & 0x3f) | flags,counter);
  }

  Peer peer () const { return Peer(from(),command() & 0x3f); }
  uint8_t counter () const { return subcommand(); }
  bool isLong () const { return (command() & 0x40) == 0x40; }
};

class SensorEventMsg : public RemoteEventMsg {
protected:
  SensorEventMsg() {}
public:
  void init(uint8_t msgcnt,uint8_t ch,uint8_t counter,uint8_t value,bool lowbat) {
      init(msgcnt,ch,counter,value,false,lowbat);
  }
  void init(uint8_t msgcnt,uint8_t ch,uint8_t counter,uint8_t value,bool lg,bool lowbat) {
      uint8_t flags = lg ? 0x40 : 0x00;
      if( lowbat == true ) {
          flags |= 0x80; // low battery
      }
      Message::init(0xc,msgcnt,0x41, BIDI|WKMEUP,(ch & 0x3f) | flags,counter);
      *data() = value;
  }
  uint8_t value () const { return *data(); }
};

class ActionMsg : public Message {
protected:
  ActionMsg() {}
public:
  uint8_t channel () const { return subcommand(); }
};

class ActionSetMsg : public ActionMsg {
protected:
  ActionSetMsg() {}
public:
  uint8_t channel () const { return subcommand(); }
  uint8_t value () const { return *data(); }
  uint16_t ramp () const {
    uint16_t value = 0;
    if( datasize() >= 3) {
      value = (*(data()+1) << 8) + *(data()+2);
    }
    return value;
  }
  uint16_t delay () const {
    uint16_t dly = 0xffff;
    if( datasize() >= 5) {
      dly = (*(data()+3) << 8) + *(data()+4);
    }
    if( dly == 0 ) dly = 0xffff;
    return dly;
  }
};

class ActionCommandMsg : public ActionMsg {
protected:
    ActionCommandMsg() {}
public:
    uint8_t channel () const { return subcommand(); }
    uint8_t len () const { return (datasize()); }
    uint8_t value (uint8_t idx) const { return *(data()+idx); }
    bool eot (uint8_t eot_char=AS_ACTION_COMMAND_EOT) const { return (length() >= 12) ? (*(data()+length()-12) == eot_char) : false; }
};

class AckMsg : public Message {
public:
  void init(uint8_t flags=0x00) {
    initWithCount(0x0a,AS_MESSAGE_RESPONSE,flags,AS_RESPONSE_ACK);
  }
};

class Ack2Msg : public Message {
public:
  void init(uint8_t flags=0x00) {
    initWithCount(0x0a,AS_MESSAGE_RESPONSE,flags,AS_RESPONSE_ACK2);
  }
};

class AckStatusMsg : public Message {
public:
  template <class ChannelType>
  void init(ChannelType& ch,uint8_t rssi) {
    initWithCount(0x0e,AS_MESSAGE_RESPONSE,0x00,AS_RESPONSE_ACK_STATUS);
    subcom = ch.number();
    pload[0] = ch.status();
    pload[1] = ch.flags();
    pload[2] = rssi;
  }
};

class NackMsg : public Message {
public:
  void init() {
    initWithCount(0x0a,AS_MESSAGE_RESPONSE,0x00,AS_RESPONSE_NACK);
  }
};

class AckAesMsg : public Message {
public:
  void init(const uint8_t* data) {
    initWithCount(0x12,AS_MESSAGE_RESPONSE,0x00,AS_RESPONSE_ACK);
    subcom = data[0];
    pload[0] = data[1];
    pload[1] = data[2];
    pload[2] = data[3];
  }
};

class AesChallengeMsg : public Message {
public:
  void init(const Message& msg,uint8_t keyidx) {
    initWithCount(0x11,AS_MESSAGE_RESPONSE,RPTEN|BIDI,AS_RESPONSE_AES_CHALLANGE);
    to(msg.from());
    from(msg.to());
    count(msg.count());
    uint8_t* tmp = data()-1;
    for( uint8_t i=0; i<6; i++ ) {
      *tmp = (uint8_t)rand();
      tmp++;
    }
    *tmp = keyidx;
  }
  const uint8_t* challenge () const {
    return data()-1;
  }
  uint8_t keyindex () const {
    return *(challenge()+6);
  }
};

class AesResponseMsg : public Message {
public:
  void init(const Message& msg) {
    initWithCount(0x19,AS_MESSAGE_RESPONSE_AES,BIDI,0x00);
    count(msg.count());
  }
  uint8_t* data () {
    return Message::data()-2;
  }
};

class AesExchangeMsg : public Message {
public:
  uint8_t* data () {
    return Message::data()-2;
  }
};

class InfoActuatorStatusMsg : public Message {
public:
  template <class ChannelType>
  void init (uint8_t count,ChannelType& ch,uint8_t rssi) {
    Message::init(0x0e,count,0x10,BIDI|WKMEUP,0x06,ch.number());
    pload[0] = ch.status();
    pload[1] = ch.flags();
    pload[2] = rssi;
  }
};

class InfoParamResponsePairsMsg : public Message {
public:
  void init (uint8_t count) {
    initWithCount(0x0b-1+(8*2),0x10,BIDI,0x02);
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
  void init (__attribute__((unused)) const HMID& to,uint8_t count) {
    Message::init(0x1a,count,0x00,0x00,0x00,0x00);
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
  void fill(uint8_t firmversion,uint8_t subtype) {
    uint8_t* buf = data();
    *buf = firmversion;
    *(buf+13) = subtype;
  }
  uint8_t* serial () { return data() + 3; }
  uint8_t* model () { return data() + 1; }
  uint8_t* info () { return data() + 14; }
};

class SerialInfoMsg : public Message {
public:
  void init (__attribute__((unused)) const HMID& to,uint8_t count) {
    Message::init(0x14,count,0x10,0x00,0x00,0x00);
  }
  uint8_t* data() { return Message::data()-4; }
  void fill(const char* serial) {
    uint8_t* buf = data();
    memcpy(buf+3,serial,10);
  }
  uint8_t* serial () { return data() + 3; }
};

class ValuesMsg : public Message {
public:
  void init(uint8_t msgcnt,uint8_t ch) {
    Message::init(0x0b,msgcnt,0x53,BIDI|WKMEUP,ch,0);
  }
  template <typename T>
  void add (T value) {
    uint8_t* values = buffer() + len + sizeof(T) - 1;
    uint8_t num = sizeof(T);
    while( num > 0 ) {
      *values = value & 0xff;
      value >>= 8;
      --values;
      --num;
    }
    // update length of message
    len += sizeof(T);
    // store number of values inside this message
    subcommand(subcommand()+1);
  }
};

}

#endif
