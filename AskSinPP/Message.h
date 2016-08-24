
#ifndef __Message_h__
#define __Message_h__

#include "HMID.h"

#define MaxDataLen   60						// maximum length of received bytes

struct MsgFlags {
  bool WKUP     :1;					// 0x01: send initially to keep the device awake
	bool WKMEUP   :1;					// 0x02: awake - hurry up to send messages
	bool CFG      :1;					// 0x04: Device in Config mode
	bool          :1;
	bool BURST    :1;					// 0x10: set if burst is required by device
	bool BIDI     :1;					// 0x20: response is expected
	bool RPTED    :1;					// 0x40: repeated (repeater operation)
	bool RPTEN    :1;					// 0x80: set in every message. Meaning?
};

class Message {
  uint8_t         len;					  // message length
	uint8_t         cnt;					  // counter, if it is an answer counter has to reflect the answered message, otherwise own counter has to be used
	MsgFlags        flags;				  // see structure of message flags
	uint8_t         typ;					  // type of message
	HMID            fromID;				  // sender ID
	HMID            toID;	                  // receiver id, broadcast for 0
	uint8_t         comm;					  // type of message
	uint8_t         subcom;				  // type of message
	uint8_t         pload[MaxDataLen-12]; // payload

public:
	uint8_t* buffer () {
	  return &len;
	}

	uint8_t* payload () {
	  return pload;
	}

  void init(uint8_t cnt, uint8_t typ, uint8_t comm, uint8_t sub) {
    this->len = 12;
    this->cnt = cnt;
    this->typ = typ;
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

  const HMID& to () {
    return toID;
  }

  uint8_t type () const {
    return typ;
  }

  uint8_t command () const {
    return comm;
  }

  uint8_t subcommand () const {
    return subcom;
  }

  void decode () {
    decode(buffer());
  }

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

  void encode () {
    encode(buffer());
  }

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

  void dump () {
    DHEX(buffer(),len);
  }
};

#endif
