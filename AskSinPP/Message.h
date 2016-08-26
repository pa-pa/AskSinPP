
#ifndef __Message_h__
#define __Message_h__

#include "HMID.h"
#include "Debug.h"

//#define MaxDataLen   60						// maximum length of received bytes
#define MaxDataLen   25

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

private:
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

	uint8_t buffersize () {
	  return sizeof(Message)-1;
	}

	uint8_t* data () {
	  return pload;
	}

	uint8_t datasize () {
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

  void init(uint8_t l, uint8_t cnt, uint8_t typ, uint8_t flags, uint8_t comm, uint8_t sub) {
    this->len = l;
    this->cnt = cnt;
    this->typ = typ;
    this->flag = flags;
    this->comm = comm;
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
/*
  void decode(uint8_t *buf) {
    uint8_t prev = buf[1];
    buf[1] = (~buf[1]) ^ 0x89;
    uint8_t i, t;
    for (i=2; i<buf[0]; i++) {
      t = buf[i];
      buf[i] = (prev + 0xdc) ^ buf[i];
      prev = t;
    }
    buf[i] ^= buf[2];
  }
*/
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
/*
  void encode(uint8_t *buf) {
    buf[1] = (~buf[1]) ^ 0x89;
    uint8_t buf2 = buf[2];
    uint8_t prev = buf[1];
    uint8_t i;
    for (i=2; i<buf[0]; i++) {
      prev = (prev + 0xdc) ^ buf[i];
      buf[i] = prev;
    }
    buf[i] ^= buf2;
  }
*/
  void dump () {
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

  bool isPairSerial () const {
    return typ==0x01 && subcom==0x0a;
  }

  bool isAck () const {
    return typ==0x02 && (comm & 0x80) == 0;
  }

  bool isNack () const {
    return typ==0x02 && (comm & 0x80) == 0x80;
  }
};

#endif
