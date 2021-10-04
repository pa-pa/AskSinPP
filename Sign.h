//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-01-15 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SIGN_H__
#define __SIGN_H__

#include <ChannelList.h>
#include <Message.h>

#ifdef USE_AES
#include <aes.h>
#endif


namespace as {

#ifdef USE_AES

#define AES_KEY_SIZE 16

#ifndef HM_DEF_KEY
  #error No HM_DEF_KEY defined.
#endif
#ifndef HM_DEF_KEY_INDEX
  #error No HM_DEF_KEY_INDEX defined.
#endif


class KeyStore : public BaseList {

public:
  uint8_t      count;
  uint8_t      auth[4];
  uint8_t      keytmp[8];

  KeyStore(uint16_t a) : BaseList(a), count(0) {}

  // return space needed in Storage
  static uint16_t size () {
    return AES_KEY_SIZE + 1;
  }

  void defaults () const {
    static uint8_t aes_def_key[] = {HM_DEF_KEY};
    setIndex(HM_DEF_KEY_INDEX); // default index
    writeKey(aes_def_key); // default key
  }

  bool readKey(uint8_t* key) const {
    return getData(1,key,AES_KEY_SIZE);
  }

  bool writeKey(uint8_t* key) const {
    return setData(1,key,AES_KEY_SIZE);
  }

  uint8_t getIndex () const {
    return getByte(0);
  }

  bool setIndex (uint8_t idx) const {
    return setByte(0,idx);
  }

  void init () {}

  void storeAuth (uint8_t c,const uint8_t* a) {
    count = c;
    memcpy(auth,a,4);
  }

  void addAuth (Message& msg) {
    if( msg.count() == count ) {
      msg.append(auth,4);
      count = 0;
    }
  }

  void fillInitVector (const Message& msg,uint8_t* initvector) {
    uint8_t n = msg.length()-10;
    memcpy(initvector,msg.buffer()+10,n);
    memset(initvector+n,0x00,16-n);
  }

  void applyVector (uint8_t* data,uint8_t* initvector) {
    for( uint8_t i=0; i<16; i++ ) {
      data[i] ^= initvector[i];
    }
  }

  bool challengeKey (const uint8_t* challenge,uint8_t index,aes128_ctx_t& ctx) {
    if( hasKey(index) == true ) {
      uint8_t key[AES_KEY_SIZE];
      readKey(key);
      for( uint8_t i=0; i<6; ++i ) {
        key[i] ^= challenge[i];
      }
      aes128_init(key,&ctx);
      return true;
    }
    return false;
  }

  bool hasKey (uint8_t index) {
    return getIndex() == index;
  }

  bool exchange (AesExchangeMsg& msg) {
    uint8_t key[AES_KEY_SIZE];
    aes128_ctx_t ctx;
    readKey(key);
    aes128_init(key,&ctx);
    uint8_t* data = msg.data();
//    DHEX(data,10);
    aes128_dec(data,&ctx);
//    DHEX(data,10);
    if( data[0] == 0x01 ) {
      if( (data[1] & 0x01) == 0x00 ) {
        memcpy(keytmp,data+2,8);
      }
      else {
        memcpy(key,keytmp,8);
        memcpy(key+8,data+2,8);
        DPRINT(F("New Key: "));DHEX(key,16);
        DPRINT(F("Index: "));DHEXLN((uint8_t)(data[1] & 0xfe));
        writeKey(key);
        setIndex(data[1] & 0xfe);
      }
      return true;
    }
    return false;
  }
};

#else

#define AES_KEY_SIZE 0

class KeyStore : public BaseList {
public:
  KeyStore(uint16_t a) : BaseList(a) {}

  // return space needed in Storage
  static uint16_t size () {
    return 0;
  }

  void defaults () {}

  void init () {}

  void addAuth (__attribute__((unused)) Message& msg) {}

};

#endif

}

#endif
