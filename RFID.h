//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-05-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-01-14 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __RFID_H__
#define __RFID_H__

#include "MultiChannelDevice.h"
#include "Register.h"

#ifdef USE_MFRC522_I2C
#include <Wire.h>
#include <MFRC522_I2C.h>
#endif

#ifdef USE_WIEGAND
#include <Wiegand.h> // https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino
#endif

#ifdef USE_MFRC522_SPI
#include <MFRC522.h>
#endif

#ifdef USE_RDM6300
#include <SoftwareSerial.h>
#endif

#define   ID_ADDR_SIZE 8

namespace as {

DEFREGISTER(RFIDReg1,CREG_AES_ACTIVE,0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7)
class RFIDList1 : public RegList1<RFIDReg1> {
public:
  RFIDList1 (uint16_t addr) : RegList1<RFIDReg1>(addr) {}
  void defaults () {
    clear();
  }
};

class ChipIdMsg : public Message {
  public:

	bool free(uint8_t*addr) {
	 bool f = true;
	 for (uint8_t n = 0; n < ID_ADDR_SIZE; n++) {
	  if (addr[n] != 0x00) {
		  f = false;
		  break;
	  }
	 }
	 return f;
	}

    void init(uint8_t msgcnt, uint8_t ch, uint8_t*addr) {
      char hexstr[ID_ADDR_SIZE * 2];
      if (free(addr)) {
    	  for (uint8_t n = 0; n < (ID_ADDR_SIZE * 2); n++)
            hexstr[n] = 0x20;
      } else {
        //convert address to hex-string - from https://stackoverflow.com/questions/6357031/how-do-you-convert-a-byte-array-to-a-hexadecimal-string-in-c
        unsigned char * pin = addr;
        const char * hex = "0123456789ABCDEF";
        char * pout = hexstr;
        uint8_t i = 0;
        for(; i < ID_ADDR_SIZE-1; ++i){
          *pout++ = hex[(*pin>>4)&0xF];
          *pout++ = hex[(*pin++)&0xF];
        }
        *pout++ = hex[(*pin>>4)&0xF];
        *pout++ = hex[(*pin)&0xF];
        *pout = 0;

      }
      //DPRINT("hexstr=");DPRINTLN(hexstr);
      Message::init(0x1a, msgcnt, 0x53, BIDI , ch , hexstr[0]);
      for (uint8_t i = 1; i < (ID_ADDR_SIZE * 2); i++) {
    	  pload[i-1] = hexstr[i];
      }
    }
};

template<class HALTYPE,int PEERCOUNT,class List0Type=List0>
class RFIDChannel : public Channel<HALTYPE,RFIDList1,EmptyList,DefList4,PEERCOUNT,List0Type>, Alarm {
//  ChipIdMsg chipIdMsg;

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
    ChipIdMsg& chipIdMsg = (ChipIdMsg&)this->device().message();
    chipIdMsg.init(this->device().nextcount(), this->number(), buf);
    _delay_ms(400); //need some small delay for ccu for appropriate message processing
    this->device().sendPeerEvent(chipIdMsg, *this);
  }

  void finish () {
      uint8_t s = none;
      // 3 or 6 matches are longpress and longlongpress
      if( (matches & 0b00111111) == 0b00000111 || (matches & 0b00111111) == 0b00111111 ) {
        s = longpressed;
        DPRINTLN(F("longpressed"));
        this->device().buzzer().on();
        // clear longlong
        matches &= 0b11000111;
      }
      // check for long release
      else if( (matches & 0b00001111) == 0b00001110 ) {
        s = longreleased;
        DPRINTLN(F("longreleased"));
        this->device().buzzer().off();
      }
      // check for release
      else if( (matches & 0b00000011) == 0b00000010 ) {
        s = released;
        DPRINTLN(F("released"));
        this->device().buzzer().on(millis2ticks(100));
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
      this->getList1().readRegister(0xe3) == 0x00 &&
      this->getList1().readRegister(0xe4) == 0x00 &&
      this->getList1().readRegister(0xe5) == 0x00 &&
      this->getList1().readRegister(0xe6) == 0x00 &&
      this->getList1().readRegister(0xe7) == 0x00
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
    if ( (msg.len() == ID_ADDR_SIZE) || (msg.len() == 1 && msg.value(0) == 0xcc) ) {
      for( uint8_t n=0; n < ID_ADDR_SIZE; ++n ) {
    	uint8_t val =  msg.len() == 1 ? 0x00:msg.value(n);
        this->getList1().writeRegister(0xe0+n,val);
      }
      state = 0;
      this->device().getHal().buzzer.on(millis2ticks(300), millis2ticks(200),2);
      sendChipID();
      this->changed(true);
    }

    if (msg.value(0) == 0xba) {
      if (msg.len() == 3)
    	this->device().getHal().buzzer.on(decis2ticks(msg.value(1)),decis2ticks(msg.value(2)), -1);
      
      if (msg.len() == 4)
        this->device().getHal().buzzer.on(decis2ticks(msg.value(1)),decis2ticks(msg.value(2)),msg.value(3));
    }
      
    if (msg.len() == 1 && msg.value(0) == 0xb1) {
    	this->device().getHal().buzzer.on();
    }

    if (msg.len() == 1 && msg.value(0) == 0xb0) {
    	this->device().getHal().buzzer.off(true);
    }

    if (msg.len() == 1 && msg.value(0) == 0xfe) {
    	sendChipID();
    }

    if (msg.len() == 2 && msg.value(0) == 0xff) {
    	this->device().getHal().standbyLedInvert(msg.value(1) == 0x01);
    }

    return true; 
  }

  bool process (__attribute__((unused)) const RemoteEventMsg& msg)   {return false; }
  bool process (__attribute__((unused)) const SensorEventMsg& msg)   {return false; }
};

#ifdef USE_WIEGAND
template <class RFIDDev,class RFIDChannel,WIEGAND& rdrDev,int LED_GREEN,int LED_RED>
#endif
#if (defined(USE_MFRC522_I2C) || defined(USE_MFRC522_SPI))
template <class RFIDDev,class RFIDChannel,MFRC522& rdrDev,int LED_GREEN,int LED_RED>
#endif
#ifdef USE_RDM6300
template <class RFIDDev,class RFIDChannel,SoftwareSerial& rdrDev,int LED_GREEN,int LED_RED>
#endif


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
    DPRINTLN(F(""));
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
#ifdef USE_WIEGAND
   if (rdrDev.available()) {
     memset(addr,0x00, ID_ADDR_SIZE);
     unsigned long wgAddr = rdrDev.getCode();
     byte addrArr[8];
     for (uint8_t i = 0; i < ID_ADDR_SIZE; i++)
       addrArr[i] = wgAddr >> (i*8) & 0xff;
     memcpy(addr, addrArr, ID_ADDR_SIZE);

     //DADDR(addr);
     return true;
   } 
   return false;
#endif

#if (defined(USE_MFRC522_I2C) || defined(USE_MFRC522_SPI))
   if (!rdrDev.PICC_IsNewCardPresent())
     if (!rdrDev.PICC_IsNewCardPresent())
      return false;
   if (!rdrDev.PICC_ReadCardSerial()) return false;
   memset(addr,0x00,ID_ADDR_SIZE);
   memcpy(addr,rdrDev.uid.uidByte,rdrDev.uid.size);

   //DADDR(addr);
   return true;
#endif

#ifdef USE_RDM6300
   while (rdrDev.available()) {
     char d = rdrDev.read();
     static uint8_t bytecount = 0;
     switch (d) {
       case 0x02:
         bytecount = 0;
         break;
       case 0x03:
         bytecount = 0;
         while (rdrDev.available()) rdrDev.read(); //empty rx buffer
         return true;
         break;
       default:
         if (bytecount < 8)
           addr[bytecount++] =  (d > 57) ? d -= 55 : d -= 48;
       break;
     }
   }
   return false;
#endif
  }

  bool readRfid(uint8_t *addr) {
   uint8_t iD[ID_ADDR_SIZE];
   static uint8_t last_addr[ID_ADDR_SIZE];

   bool success = false;

   memset(addr,0,ID_ADDR_SIZE); 
   if (getRfidAddress(iD)) {
     memcpy(addr,iD,ID_ADDR_SIZE);
     if (memcmp(addr, last_addr, ID_ADDR_SIZE) != 0) {
         dev.buzzer().on(millis2ticks(100));
     }
     success = true;
   }
   memcpy(last_addr,addr,ID_ADDR_SIZE);

   return success;
  }
   
  void scan () {
    uint8_t addr[ID_ADDR_SIZE];

    start();
    bool readID = readRfid(addr);

    if( check(addr) == true ) {
      led.ledOn(millis2ticks(500),0);
    } else {
    	if (readID == true )
    		dev.buzzer().on(millis2ticks(40),millis2ticks(40),3);
    }
    finish();
  }

  bool learn (RFIDChannel* lc) {
    uint8_t addr[ID_ADDR_SIZE];
    while( readRfid(addr) == true ) {
      if( find(addr) == 0 ) {
        lc->storeID(addr);
        dev.buzzer().on(millis2ticks(40), millis2ticks(50),10);
        return true;
      }
    }
    return false;
  }

  void trigger (AlarmClock& clock) {
	// reactivate
	set(millis2ticks(500));
	clock.add(*this);
    ++cnt;
    // check if we have a learning channel
    RFIDChannel* lc = learning();
    if( lc != 0 ) {
      uint8_t cycle = cnt & 0x01;
      led.ledOn(cycle == 0 ? tick : 0, cycle == 0 ? 0 : tick);
      dev.buzzer().on(millis2ticks(40));
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
