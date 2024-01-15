//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ASKSINPP_h__
#define __ASKSINPP_h__

#define ASKSIN_PLUS_PLUS_MAJOR 5
#define ASKSIN_PLUS_PLUS_MINOR 0
#define ASKSIN_PLUS_PLUS_SERVICE 3

#define ASKSIN_PLUS_PLUS_VERSION (ASKSIN_PLUS_PLUS_MAJOR * 10000 \
  + ASKSIN_PLUS_PLUS_MINOR * 100 + ASKSIN_PLUS_PLUS_SERVICE)

#define STRINGIZE2(s) #s
#ifndef STRINGIZE
#define STRINGIZE(s) STRINGIZE2(s)
#endif
#define ASKSIN_PLUS_PLUS_VERSION_STR STRINGIZE(ASKSIN_PLUS_PLUS_MAJOR) \
  "." STRINGIZE(ASKSIN_PLUS_PLUS_MINOR) "." STRINGIZE(ASKSIN_PLUS_PLUS_SERVICE)

#define ASKSIN_PLUS_PLUS_IDENTIFIER F("AskSin++ v" ASKSIN_PLUS_PLUS_VERSION_STR " (" __DATE__ " " __TIME__ ")")


#define CONFIG_FREQ1     0
#define CONFIG_FREQ2     1
#define CONFIG_BOOTSTATE 2  //location of current boot state for ResetOnBoot

#include "Debug.h"
#include <stdint.h>

#ifdef ARDUINO_ARCH_EFM32
  #define NOT_AN_INTERRUPT 0
  #define enableInterrupt(pin,handler,mode) attachInterrupt(pin,handler,mode)
  typedef unsigned long WiringPinMode;
  typedef uint8_t uint8;
#elif ARDUINO_ARCH_STM32F1
  #define _delay_us(us) delayMicroseconds(us)
  inline void _delay_ms(uint32_t ms) { do { delayMicroseconds(1000); } while( (--ms) > 0); }

  #define NOT_AN_INTERRUPT -1
  #define digitalPinToInterrupt(pin) (pin)
  #define enableInterrupt(pin,handler,mode) attachInterrupt(pin,handler,mode)
  #define disableInterrupt(pin) detachInterrupt(pin)
  #define memcmp_P(src,dst,count) memcmp((src),(dst),(count))
#elif (defined ARDUINO_ARCH_STM32) || (defined ARDUINO_ARCH_ESP32) || (defined ARDUINO_ARCH_RP2040)
  #define _delay_us(us) delayMicroseconds(us)
  //inline void _delay_ms(uint32_t ms) { do { delayMicroseconds(1000); } while ((--ms) > 0); }
  inline void _delay_ms(uint32_t ms) { delay(ms); }
  typedef uint32_t WiringPinMode;
  typedef uint8_t uint8;

  #define enableInterrupt(pin,handler,mode) attachInterrupt(pin,handler,mode)
  #define disableInterrupt(pin) detachInterrupt(pin)
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
#include "Debug.h"
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

/**
 * AskSinBase provides basic methods for general use.
 */
class AskSinBase {
  Storage storage;
public:
  /**
   * Read data from program space (AVR).
   * \param dest pointer to destiantion in RAM
   * \param adr address of program space to read data from
   * \param size of data to read
   */
  static void pgm_read(uint8_t* dest,uint16_t adr,uint8_t size) {
    for( int i=0; i<size; ++i, ++dest ) {
#ifdef ARDUINO_ARCH_RP2040
      *dest = pgm_read_byte((const void*)(adr + i));
#else
      *dest = pgm_read_byte(adr + i);
#endif
    }
  }
  /**
   * Convert numeric value to character for printing.
   * \param c numeric value to convert
   * \return character for output
   */
  static uint8_t toChar (uint8_t c) {
    return *(__gb_chartable+c);
  }
  /**
   * Update CRC16 value
   * \param crc current checksum value
   * \param data to add
   * \return new crc checksum value
   */
  static uint16_t crc16 (uint16_t crc,uint8_t d) {
    crc ^= d;
    for( uint8_t i = 8; i != 0; --i ) {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0 );
    }
    return crc;
  }

  /**
   * Calculate CRC24 value from data
   * \param data pointer to data to calculate CRC checksum
   * \param len number of bytes to use for calculation
   * \return CRC24 checksum
   */
  static uint32_t crc24(const uint8_t* data,int len) {
    uint32_t crc = 0xB704CEL;
    for( uint8_t i=0; i<len; ++i) {
      crc = crc24(crc,data[i]);
    }
    return crc;
  }

  /**
   * Update CRC24 value
   * \param crc current checksum value
   * \param data to add
   * \return new crc checksum value
   */
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

    uint32_t tByte;
    if ((iTime & 0x1F) != 0) {
      tByte = 2;
      for (uint8_t i = 1; i < (iTime & 0x1F); i++) tByte *= 2;
    } else tByte = 1;

    return decis2ticks( (uint32_t)tByte*(iTime>>5) );
  }

  /** Calculate time until next send slot
   * \param id Homematic ID of the device
   * \param msgcnt current message counter
   * \return next send slot in sysclock ticks
   */
  static uint32_t nextSendSlot (const HMID& id,uint8_t msgcnt) {
    uint32_t value = ((uint32_t)id) << 8 | msgcnt;
    value = (value * 1103515245 + 12345) >> 16;
    value = (value & 0xFF) + 480;
    value *= 250;

    DDEC(value / 1000);DPRINT(".");DDECLN(value % 1000);

    return value;
  }
  /**
   * Read status of pin.
   * \param pinnr the number of the pin to read
   * \param enablenr pin to set to high for enabling the read
   * \param ms milli seconds to wait between enablement and pin read
   * \return status of read pin
   */
  static uint8_t readPin(uint8_t pinnr,uint8_t enablenr=0,uint8_t ms=0) {
    uint8_t value=0;
    pinMode(pinnr,INPUT_PULLUP);
    if( enablenr != 0 && enablenr != 0xff) {
      digitalWrite(enablenr,HIGH);
      if( ms != 0 ) {
        _delay_ms(ms);
      }
    }
    value = digitalRead(pinnr);
    // pinMode(pinnr,OUTPUT);
    // digitalWrite(pinnr,LOW);
    // 01/2022 TM: Ruhezustand des Eingangs ist nicht mehr Output/Low sondern High-Z
    // https://homematic-forum.de/forum/viewtopic.php?f=76&t=71854
    pinMode(pinnr,INPUT);
    if( enablenr != 0 && enablenr != 0xff) {
      digitalWrite(enablenr,LOW);
    }
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
    led.init();
    buzzer.init();
    bool ccinitOK = radio.init();
    radio.enable();
    // start the system timer
    sysclock.init();
    // signal start to user
    led.set(ccinitOK ? LedStates::welcome : LedStates::failure);
    // delay first send by 'random' time
    radio.setSendTimeout((id.id2()*10)+10);
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
      // check if CONFIG_FREQ1 is in range 0x60...0x6A -> 868,3MHz -550kHz/+567kHz
      if( f1 >= 0x60 && f1 <= 0x6A ) {
        DPRINT(F("Config Freq: 0x21"));DHEX(f1);DHEXLN(f2);
        radio.tuneFreq(0x21, f1, f2);
      }
    }
  }

  bool runready () {
    return sysclock.runready();
  }

  // call by the device before the device sends a message
  // can be used to update data by the device sketch
  void prepareSend (__attribute__((unused)) Message& msg) {}

  // call after send a message to a peer
  void sendPeer () {}

  // use radio timer to wait given millis
  void waitTimeout(uint16_t millis) {
    radio.waitTimeout(millis);
  }

  void setIdle () {
    radio.setIdle();
    battery.setIdle();
  }

  void unsetIdle () {
    battery.unsetIdle();
  }

  void wakeup () {
    radio.wakeup();
  }

#if defined(ARDUINO_ARCH_AVR) && ! ( defined(ARDUINO_AVR_ATmega32) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega128__))
  template <bool ENABLETIMER2=false, bool ENABLEADC=false>
  void idle () { activity.savePower< Idle<ENABLETIMER2,ENABLEADC> >(*this); }

  template <bool ENABLETIMER2=false>
  void sleep () { activity.savePower< Sleep<ENABLETIMER2> >(*this); }

  void sleepForever () { activity.sleepForever(*this); }
#endif
#if defined(ARDUINO_ARCH_RP2040)
  void sleep () { activity.savePower<Sleep>(*this); }
#endif
};

#ifndef NORTC
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

#if defined(ARDUINO_ARCH_AVR) && ! ( defined(ARDUINO_AVR_ATmega32) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega128__))
  template <bool ENABLETIMER2=false>
  void sleep () { this->activity.template savePower< SleepRTC >(*this); }
#endif
};
#endif

template <class HAL,int TIMEOUT=60>
class RadioWatchdog : public Alarm {
public:
  RadioWatchdog () : Alarm(0,false) {}
  virtual ~RadioWatchdog () {}

  virtual void trigger (AlarmClock& clock) {
    typename HAL::RadioType& radio = HAL::RadioType::instance();
    bool alive = radio.clearAlive();
    if( alive == false ) {
      radio.init();
      radio.enable();
    }
    set(seconds2ticks(TIMEOUT));
    clock.add(*this);
  }

  void start (AlarmClock& clock) {
    trigger(clock);
  }
};

}

#endif
