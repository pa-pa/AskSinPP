//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ASKSINPP_h__
#define __ASKSINPP_h__

#define ASKSIN_PLUS_PLUS_VERSION "4.1.1"

#define ASKSIN_PLUS_PLUS_IDENTIFIER F("AskSin++ V" ASKSIN_PLUS_PLUS_VERSION " (" __DATE__ " " __TIME__ ")")


#define CONFIG_FREQ1 0
#define CONFIG_FREQ2 1

#include <Debug.h>
#include <stdint.h>

#ifdef ARDUINO_ARCH_STM32F1
  #define _delay_us(us) delayMicroseconds(us)
  inline void _delay_ms(uint32_t ms) { do { delayMicroseconds(1000); } while( (--ms) > 0); }

  #define NOT_AN_INTERRUPT -1
  #define digitalPinToInterrupt(pin) (pin)
  #define enableInterrupt(pin,handler,mode) attachInterrupt(pin,handler,mode)
  #define disableInterrupt(pin) detachInterrupt(pin)
  #define memcmp_P(src,dst,count) memcmp((src),(dst),(count))
#else
  typedef uint8_t uint8;
  typedef uint16_t uint16;
  // if we have no EnableInterrupt Library - and also no PCINT - use polling
  #ifndef EnableInterrupt_h
    #define enableInterrupt(pin,handler,mode) pinpolling##pin().enable(pin,handler,mode)
    #define disableInterrupt(pin) pinpolling##pin().disable()
  #endif
#endif


#include <Storage.h>
#include <Pins.h>
#include <Debug.h>
#include <Activity.h>
#include <Led.h>
#include <Buzzer.h>
#include <AlarmClock.h>
#include <Message.h>
#include <Button.h>
#include <Radio.h>
#include <BatterySensor.h>


namespace as {

extern const char* __gb_chartable;

class AskSinBase {
  Storage storage;
public:

  static void pgm_read(uint8_t* dest,uint16_t adr,uint8_t size) {
    for( int i=0; i<size; ++i, ++dest ) {
      *dest = pgm_read_byte(adr + i);
    }
  }

  static uint8_t toChar (uint8_t c) {
    return *(__gb_chartable+c);
  }

  static uint16_t crc16 (uint16_t crc,uint8_t d) {
    crc ^= d;
    for( uint8_t i = 8; i != 0; --i ) {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0 );
    }
    return crc;
  }

  static uint32_t crc24(const uint8_t* data,int len) {
    uint32_t crc = 0xB704CEL;
    for( uint8_t i=0; i<len; ++i) {
      crc = crc24(crc,data[i]);
    }
    return crc;
  }

  static uint32_t crc24(uint32_t crc,uint8_t d) {
    crc ^= ((uint32_t)d) << 16;
    for( uint8_t i = 0; i < 8; i++) {
      crc <<= 1;
      if (crc & 0x1000000) {
        crc ^= 0x1864CFBL;
      }
    }
    return crc & 0xFFFFFFL;
  }

  // get timer count in ticks
  static uint32_t byteTimeCvt(uint8_t tTime) {
    if( tTime == 0xff ) return 0xffffffff;
    const uint16_t c[8] = {1,10,50,100,600,3000,6000,36000};
    return decis2ticks( (uint32_t)(tTime & 0x1F) * c[tTime >> 5] );
  }

  static uint32_t byteTimeCvtSeconds (uint8_t tTime) {
    const uint8_t c[2] = {1,60};
    return seconds2ticks( (uint32_t)(tTime & 0x7F) * c[tTime >> 7] );
  }

  // get timer count in ticks
  static uint32_t intTimeCvt(uint16_t iTime) {
    if (iTime == 0x00) return 0x00;
    if (iTime == 0xffff) return 0xffffffff;

    uint8_t tByte;
    if ((iTime & 0x1F) != 0) {
      tByte = 2;
      for (uint8_t i = 1; i < (iTime & 0x1F); i++) tByte *= 2;
    } else tByte = 1;

    return decis2ticks( (uint32_t)tByte*(iTime>>5) );
  }

  // calculate time until next send slot
  static uint32_t nextSendSlot (const HMID& id,uint8_t msgcnt) {
    uint32_t value = ((uint32_t)id) << 8 | msgcnt;
    value = (value * 1103515245 + 12345) >> 16;
    value = (value & 0xFF) + 480;
    value *= 250;

    DDEC(value / 1000);DPRINT(".");DDECLN(value % 1000);

    return value;
  }

};

template <class StatusLed,class Battery,class Radio,class Buzzer=NoBuzzer>
class AskSin : public AskSinBase {

public:
  typedef StatusLed LedType;
  typedef Battery   BatteryType;
  typedef Radio     RadioType;
  typedef Buzzer    BuzzerType;

  LedType      led;
  BatteryType  battery;
  Activity     activity;
  RadioType    radio;
  BuzzerType   buzzer;

  void init (const HMID& id) {
    srand((unsigned int&)id);
    led.init();
    buzzer.init();
    radio.init();
    radio.enable();
    // start the system timer
    sysclock.init();
    // signal start to user
    led.set(LedStates::welcome);
    // delay first send by random time
    radio.setSendTimeout((rand() % 3500)+1000);
  }

  void initBattery(uint16_t interval,uint8_t low,uint8_t critical) {
    battery.init(seconds2ticks(interval),sysclock);
    battery.low(low);
    battery.critical(critical);
  }

  void config (const StorageConfig& sc) {
    if( sc.valid() == true ) {
      uint8_t f1 = sc.getByte(CONFIG_FREQ1);
      uint8_t f2 = sc.getByte(CONFIG_FREQ2);
      if( f1 != 0 ) {
        DPRINT(F("Config Freq: 0x21"));DHEX(f1);DHEXLN(f2);
        radio.initReg(CC1101_FREQ2, 0x21);
        radio.initReg(CC1101_FREQ1, f1);
        radio.initReg(CC1101_FREQ0, f2);
      }
    }
  }

  bool runready () {
    return sysclock.runready();
  }

  void sendPeer () {}

  // use radio timer to wait given millis
  void waitTimeout(uint16_t millis) {
    radio.waitTimeout(millis);
  }

#ifdef ARDUINO_ARCH_AVR
  template <bool ENABLETIMER2=false, bool ENABLEADC=false>
  void idle () { activity.savePower< Idle<ENABLETIMER2,ENABLEADC> >(*this); }

  template <bool ENABLETIMER2=false>
  void sleep () { activity.savePower< Sleep<ENABLETIMER2> >(*this); }
#endif
};


template <class StatusLed,class Battery,class Radio,class Buzzer=NoBuzzer>
class AskSinRTC : public AskSin<StatusLed,Battery,Radio,Buzzer> {
public:
  void init (const HMID& id) {
    AskSin<StatusLed,Battery,Radio,Buzzer>::init(id);
    // init real time clock - 1 tick per second
    rtc.init();
  }

  void initBattery(uint16_t interval,uint8_t low,uint8_t critical) {
    this->battery.init(interval,rtc);
    this->battery.low(low);
    this->battery.critical(critical);
  }


  bool runready () {
      return rtc.runready() || AskSin<StatusLed,Battery,Radio,Buzzer>::runready();
  }

#ifdef ARDUINO_ARCH_AVR
  template <bool ENABLETIMER2=false>
  void sleep () { this->activity.template savePower< SleepRTC >(*this); }
#endif
};

}

#endif
