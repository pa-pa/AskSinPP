//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-05-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-01-14 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __RFID_H__
#define __RFID_H__

#include "MultiChannelDevice.h"
#include "Register.h"
#include <MFRC522.h>

#define   ID_ADDR_SIZE 4

namespace as {

DEFREGISTER(RFIDReg1,CREG_AES_ACTIVE,0xe0,0xe1,0xe2,0xe3)
class RFIDList1 : public RegList1<RFIDReg1> {
public:
  RFIDList1 (uint16_t addr) : RegList1<RFIDReg1>(addr) {}
  void defaults () {
    clear();
  }
};

class ChipIdMsg : public Message {
  public:
    void init(uint8_t msgcnt, uint8_t ch, uint8_t*addr) {
      Message::init(0x13, msgcnt, 0x53, BIDI , 0x00, ch);
      //convert address to hex-string - from https://stackoverflow.com/questions/6357031/how-do-you-convert-a-byte-array-to-a-hexadecimal-string-in-c
      char hexstr[8];
      unsigned char * pin = addr;
      const char * hex = "0123456789ABCDEF";
      char * pout = hexstr;
      uint8_t i = 0;
      for(; i < (sizeof(addr) * 2)-1; ++i){
        *pout++ = hex[(*pin>>4)&0xF];
        *pout++ = hex[(*pin++)&0xF];
      }
      *pout++ = hex[(*pin>>4)&0xF];
      *pout++ = hex[(*pin)&0xF];
      *pout = 0;
      //DPRINT("hexstr=");DPRINTLN(hexstr);
      memcpy(pload, hexstr, 8);
    }
};

template<class HALTYPE,int PEERCOUNT,class List0Type=List0>
class RFIDChannel : public Channel<HALTYPE,RFIDList1,EmptyList,DefList4,PEERCOUNT,List0Type>, Alarm {
  ChipIdMsg chipIdMsg;

  enum { none=0, released, longpressed, longreleased };

  uint8_t state, matches, repeatcnt;

public:
  typedef Channel<HALTYPE,RFIDList1,EmptyList,DefList4,PEERCOUNT,List0Type> BaseChannel;

  RFIDChannel () : BaseChannel(), Alarm(0), state(0), matches(0),repeatcnt(0) {}
  virtual ~RFIDChannel () {}

  virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
    state = 0;
    this->changed(true);
  }

  uint8_t status () const {
    return state;
  }

  uint8_t flags () const {
    return 0;
  }

  void start () {
    matches <<= 1;
  }

  bool check (uint8_t* addr) {
    if( free() == false && isID(addr) == true ) {
      matches |= 0b00000001;
      return true;
    }
    return false;
  }

  void sendChipID() {
    uint8_t buf[ID_ADDR_SIZE];
    RFIDList1 l = this->getList1();
    for( uint8_t n=0; n< ID_ADDR_SIZE; ++n ) {
      buf[n] = l.readRegister(0xe0+n);
    }
    chipIdMsg.init(this->device().nextcount(), this->number(), buf);
    this->device().sendPeerEvent(chipIdMsg, *this);
  }

  void finish () {
      uint8_t s = none;
      // 3 or 6 matches are longpress and longlongpress
      if( (matches & 0b00111111) == 0b00000111 || (matches & 0b00111111) == 0b00111111 ) {
        s = longpressed;
        DPRINTLN("longpressed");
        if (this->device().getList0().buzzerEnabled() == true)
          this->device().getHal().buzzer.ledOn();
        // clear longlong
        matches &= 0b11000111;
      }
      // check for long release
      else if( (matches & 0b00001111) == 0b00001110 ) {
        s = longreleased;
        DPRINTLN("longreleased");
        if (this->device().getList0().buzzerEnabled() == true)
            this->device().getHal().buzzer.ledOff();
      }
      // check for release
      else if( (matches & 0b00000011) == 0b00000010 ) {
        s = released;
        DPRINTLN("released");
        if (this->device().getList0().buzzerEnabled() == true)
          this->device().getHal().buzzer.ledOn(millis2ticks(100));
      }
      if( s != none ) {
        RemoteEventMsg& msg = (RemoteEventMsg&)this->device().message();
        msg.init(this->device().nextcount(),this->number(),repeatcnt,(s==longreleased || s==longpressed),this->device().battery().low());
        if( s == released || s == longreleased) {
          // send the message to every peer
          this->device().sendPeerEvent(msg,*this);
          repeatcnt++;
        }
        else if (s == longpressed) {
          // broadcast the message
          this->device().broadcastPeerEvent(msg,*this);
        }
      }
  }

  bool match (uint8_t* addr) {
    start();
    bool res = check(addr);
    finish();
    return res;
  }

  bool isID (uint8_t* buf) {
    RFIDList1 l = this->getList1();
    for( uint8_t n=0; n< ID_ADDR_SIZE; ++n ) {
      if( l.readRegister(0xe0+n) != buf[n] ) {
        return false;
      }
    }
    return true;
  }

  void storeID (uint8_t* buf) {
    if( learn() == true ) {
      for( uint8_t n=0; n < ID_ADDR_SIZE; ++n ) {
        this->getList1().writeRegister(0xe0+n,buf[n]);
      }
      state = 0;
      sendChipID();
      this->changed(true);
      sysclock.cancel(*this);
    }
  }

  bool free () {
    return { 
      this->getList1().readRegister(0xe0) == 0x00 &&
      this->getList1().readRegister(0xe1) == 0x00 &&
      this->getList1().readRegister(0xe2) == 0x00 &&
      this->getList1().readRegister(0xe3) == 0x00 
    };
  }

  bool learn () const {
    return state == 200;
  }

  bool process (const ActionSetMsg& msg) {
    state = msg.value();
    this->changed(true);
    if( state != 0 ) {
      sysclock.cancel(*this);
      set(seconds2ticks(60));
      sysclock.add(*this);
    }
    return true;
  }

  bool process (const ActionCommandMsg& msg) {    
    if (msg.len() == ID_ADDR_SIZE) {
      for( uint8_t n=0; n < ID_ADDR_SIZE; ++n ) {
        this->getList1().writeRegister(0xe0+n,msg.value(n));
      }
      state = 0;
      sendChipID();
      this->changed(true);
    }
    return true; 
  }

  bool process (__attribute__((unused)) const RemoteEventMsg& msg)   {return false; }
  bool process (__attribute__((unused)) const SensorEventMsg& msg)   {return false; }
};

template <class RFIDDev,class RFIDChannel,MFRC522& m,int LED_GREEN,int LED_RED>
class RFIDScanner : public Alarm {
  RFIDDev& dev;
  DualStatusLed<LED_GREEN,LED_RED> led;
  uint8_t cnt;
public:
  RFIDScanner (RFIDDev& d) : Alarm(millis2ticks(500)), dev(d), cnt(0) {
    led.init();
  }
  virtual ~RFIDScanner () {}

  RFIDChannel* learning () {
    for( uint8_t i=0; i<dev.rfidCount(); ++i ) {
      RFIDChannel& rc = dev.rfidChannel(i);
      if( rc.learn() == true ) {
        return &rc;
      }
    }
    return 0;
  }

  RFIDChannel* matches (uint8_t* addr) {
    for( uint8_t i=0; i<dev.rfidCount(); ++i ) {
      RFIDChannel& rc = dev.rfidChannel(i);
      if( rc.match(addr) == true ) {
        return &rc;
      }
    }
    return 0;
  }

  RFIDChannel* find (uint8_t* addr) {
    for( uint8_t i=0; i<dev.rfidCount(); ++i ) {
      RFIDChannel& rc = dev.rfidChannel(i);
      if( rc.isID(addr) == true ) {
        return &rc;
      }
    }
    return 0;
  }

  void DADDR(uint8_t * addr) {
    for (uint8_t i = 0; i < ID_ADDR_SIZE; i++)
      DHEX(addr[i]);
    DPRINTLN("");
  }
 
  void start () {
    for( uint8_t i=0; i<dev.rfidCount(); ++i ) {
      RFIDChannel& rc = dev.rfidChannel(i);
      rc.start();
    }
  }

  void finish () {
    for( uint8_t i=0; i<dev.rfidCount(); ++i ) {
      RFIDChannel& rc = dev.rfidChannel(i);
      rc.finish();
    }
  }

  bool check (uint8_t* addr) {
    bool res = false;
    for( uint8_t i=0; i<dev.rfidCount(); ++i ) {
      RFIDChannel& rc = dev.rfidChannel(i);
      res |= rc.check(addr);
    }
    return res;
  }

  bool getRfidAddress(uint8_t *addr) {
    if (!m.PICC_IsNewCardPresent())
      if (!m.PICC_IsNewCardPresent())
       return false;
    if (!m.PICC_ReadCardSerial()) return false; 
    memcpy(addr,m.uid.uidByte,ID_ADDR_SIZE);
    //DADDR(addr);
    return true;
  }

  bool readRfid(uint8_t *addr) {
   uint8_t read1[ID_ADDR_SIZE];
   memset(addr,0,ID_ADDR_SIZE); 
   if (getRfidAddress(read1)) {
     memcpy(addr,read1,ID_ADDR_SIZE);
    return true;
   } else {
     return false;
   }
  }
   
  void scan () {
    uint8_t addr[ID_ADDR_SIZE];
    start();
    readRfid(addr);
    if( check(addr) == true ) {
      led.ledOn(millis2ticks(500),0);
      if (dev.getList0().buzzerEnabled() == true)
        dev.getHal().buzzer.ledOn(millis2ticks(50));
    }
    finish();
  }

  bool learn (RFIDChannel* lc) {
    uint8_t addr[ID_ADDR_SIZE];
    while( readRfid(addr) == true ) {
      if( find(addr) == 0 ) {
        lc->storeID(addr);
        return true;
      }
    }
    return false;
  }

  void trigger (AlarmClock& clock) {
    // reactivate
    set(millis2ticks(200));
    clock.add(*this);
    ++cnt;
    // check if we have a learning channel
    RFIDChannel* lc = learning();
    if( lc != 0 ) {
      uint8_t cycle = cnt & 0x01;
      led.ledOn(cycle == 0 ? tick : 0, cycle == 0 ? 0 : tick);
      // if we have learned a new ID
      if( learn(lc) == true ) {
        clock.cancel(*this);
        set(seconds2ticks(5));
        led.ledOff();
        led.ledOn(tick);
        clock.add(*this);
      }
    }
    else {
      // scan the bus now
      scan();
    }
  }
};

}

#endif
