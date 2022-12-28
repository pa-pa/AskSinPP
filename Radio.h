//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- AskSin cc1101 functions -----------------------------------------------------------------------------------------------
//- with a lot of copy and paste from culfw firmware
//- -----------------------------------------------------------------------------------------------------------------------
//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-03-31 stan23 Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef _RADIO_H
#define _RADIO_H

#ifdef ARDUINO_ARCH_ESP32
  #include <SPI.h>
  typedef uint8_t BitOrder;
#endif

#if (defined ARDUINO_ARCH_EFM32) || ((defined ARDUINO_ARCH_RP2040) && (defined _HARDWARE_SPI_H))
  #define SPI_MODE0     0
#endif

#if defined ARDUINO_ARCH_EFM32
  // TODO: do other architectures support multi-transfer as well?
  #define SPI_MULTIBYTE_TRANSFER
#endif

#ifdef ARDUINO_ARCH_EFM32

#define SPIDRV_MASTER_USART0_LOC0 {                                                                         \
    USART0,                       /* USART port                       */    \
    _USART_ROUTE_LOCATION_LOC0,   /* USART pins location number       */    \
    CLOCK,                        /* Bitrate                          */    \
    8,                            /* Frame length                     */    \
    0,                            /* Dummy tx value for rx only funcs */    \
    spidrvMaster,                 /* SPI mode                         */    \
    spidrvBitOrderMsbFirst,       /* Bit order on bus                 */    \
    spidrvClockMode0,             /* SPI clock/phase mode             */    \
    spidrvCsControlApplication,   /* CS controlled by the driver      */    \
    spidrvSlaveStartImmediate     /* Slave start transfers immediately*/    \
  }
#endif

#include "Message.h"
#include "AlarmClock.h"

#if defined ARDUINO_ARCH_AVR // && !defined Adafruit_SPIDevice_h
  #include <util/delay.h>
  typedef uint8_t BitOrder;
#endif

//#ifndef Adafruit_SPIDevice_h
  #define SPI_BITORDER_MSBFIRST MSBFIRST
  #define SPI_BITORDER_LSBFIRST LSBFIRST
//#endif

// #define USE_CCA

#ifndef USE_OTA_BOOTLOADER
  // we can not reuse the frequency if the ota bootloader
  #undef USE_OTA_BOOTLOADER_FREQUENCY
#endif

#include "Radio-CC1101.h"
#include "Radio-Si4431.h"
#include "Radio-RFM69.h"

namespace as {

#ifdef ARDUINO_ARCH_AVR

template <uint8_t CS,uint8_t MOSI,uint8_t MISO,uint8_t SCLK, class PINTYPE=ArduinoPins>
class AvrSPI {

public:
  uint8_t send (uint8_t data) {
    SPDR = data;                  // send byte
    while (!(SPSR & _BV(SPIF)));  // wait until transfer finished
    return SPDR;
  }

  void waitMiso () {
    while(PINTYPE::getState(MISO));
  }

  void init () {
    PINTYPE::setOutput(CS);
    PINTYPE::setOutput(MOSI);
    PINTYPE::setInput(MISO);
    PINTYPE::setOutput(SCLK);
    // SPI enable, master, speed = CLK/4
    SPCR = _BV(SPE) | _BV(MSTR);
    PINTYPE::setHigh(CS);
    // Set SCLK = 1 and SI = 0, to avoid potential problems with pin control mode
    PINTYPE::setHigh(SCLK);
    PINTYPE::setLow(MOSI);
  }

  void shutdown () {
    PINTYPE::setInput(CS);
    PINTYPE::setInput(MOSI);
    PINTYPE::setInput(SCLK);
  }

  void select () {
    PINTYPE::setLow(CS);
  }

  void deselect () {
    PINTYPE::setHigh(CS);
  }

  void ping () {
    select();                                     // wake up the communication module
    waitMiso();
    deselect();
  }
  
  uint8_t strobe(uint8_t cmd) {
    select();                                     // select  radio module
    waitMiso();                                   // wait until MISO goes low
    uint8_t ret = send(cmd);                      // send strobe command
    deselect();                                   // deselect  radio module
    return ret;
  }

  void readBurst(uint8_t * buf, uint8_t regAddr, uint8_t len) {
    select();                                     // select  radio module
    waitMiso();                                   // wait until MISO goes low
    send(regAddr | READ_BURST);                   // send register address
    for(uint8_t i=0 ; i<len ; i++) {
      buf[i] = send(0x00);                        // read result byte by byte
      //dbg << i << ":" << buf[i] << '\n';
    }
    deselect();                                   // deselect  radio module
  }

  void writeBurst(uint8_t regAddr, uint8_t* buf, uint8_t len) {
    select();                                     // select  radio module
    waitMiso();                                   // wait until MISO goes low
    send(regAddr | WRITE_BURST);                  // send register address
    for(uint8_t i=0 ; i<len ; i++)
      send(buf[i]);                               // send value
    deselect();                                   // deselect  radio module
  }

  uint8_t readReg(uint8_t regAddr, uint8_t regType) {
    select();                                     // select  radio module
    waitMiso();                                   // wait until MISO goes low
    send(regAddr | regType);                      // send register address
    uint8_t val = send(0x00);                     // read result
    deselect();                                   // deselect  radio module
    return val;
  }

  void writeReg(uint8_t regAddr, uint8_t val) {
    select();                                     // select  radio module
    waitMiso();                                   // wait until MISO goes low
    send(regAddr);                                // send register address
    send(val);                                    // send value
    deselect();                                   // deselect  radio module
  }

};

#endif

#ifdef SPI_MODE0
template <uint8_t CS,uint32_t CLOCK=2000000, BitOrder BITORDER=SPI_BITORDER_MSBFIRST, uint8_t MODE=SPI_MODE0>
class LibSPI {
public:
#ifdef ARDUINO_ARCH_EFM32
  SPIClass SPI;
#endif
  LibSPI () {}
  void init () {
#if defined ARDUINO_ARCH_STM32 && defined STM32L1xx
    SPI.setMOSI(PIN_SPI_MOSI);
    SPI.setMISO(PIN_SPI_MISO);
    SPI.setSCLK(PIN_SPI_SCK);
#endif
    pinMode(CS, OUTPUT);

#if defined ARDUINO_ARCH_EFM32
    SPIDRV_Init_t spiInitData = SPIDRV_MASTER_USART0_LOC0;
    SPI.begin(&spiInitData);
#else
    SPI.begin();
#endif
  }

  void shutdown () {
    SPI.end();
    pinMode(CS, INPUT);
#if defined ARDUINO_ARCH_STM32 && defined STM32L1xx
    pinMode(PIN_SPI_MOSI, INPUT);
    pinMode(PIN_SPI_SCK, INPUT);
#else
    pinMode(MOSI, INPUT);
    pinMode(SCK, INPUT);
#endif
  }

  void select () {
    digitalWrite(CS,LOW);
  }

  void deselect () {
    digitalWrite(CS,HIGH);
  }

  void ping () {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // wake up the communication module
    SPI.transfer(0); // ????
    deselect();
    SPI.endTransaction();
  }
  
  void waitMiso () {
#ifdef ARDUINO_ARCH_STM32F1
    while(digitalRead(SPI.misoPin()));
#elif defined (PIN_SPI_MISO)
    while(digitalRead(PIN_SPI_MISO));
#else
    _delay_us(10);
#endif
  }

  uint8_t send (uint8_t data) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    uint8_t ret = SPI.transfer(data);
    SPI.endTransaction();
    return ret;
  }

  uint8_t strobe(uint8_t cmd) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // select  radio module
    uint8_t ret = SPI.transfer(cmd);
    deselect();                                   // deselect  radio module
    SPI.endTransaction();
    return ret;
  }

  void readBurst(uint8_t * buf, uint8_t regAddr, uint8_t len) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // select  radio module
    SPI.transfer(regAddr);                        // send register address
#if defined SPI_MULTIBYTE_TRANSFER
    SPI.receive(buf, len);                        // receive len bytes at once
#else
    for(uint8_t i=0 ; i<len ; i++) {
      buf[i] = SPI.transfer(0x00);                // read result byte by byte
      //dbg << i << ":" << buf[i] << '\n';
    }
#endif
    deselect();                                   // deselect  radio module
    SPI.endTransaction();
  }

  void writeBurst(uint8_t regAddr, const uint8_t* buf, uint8_t len) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // select  radio module
    SPI.transfer(regAddr);                        // send register address
#if defined SPI_MULTIBYTE_TRANSFER
    SPI.transmit(buf, len);                       // send the whole buffer
#else
    for(uint8_t i=0 ; i<len ; i++)
      SPI.transfer(buf[i]);                       // send value
#endif
    deselect();                                   // deselect  radio module
    SPI.endTransaction();
  }

  uint8_t readReg(uint8_t regAddr, uint8_t regType=0) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // select  radio module
    SPI.transfer(regAddr | regType);              // send register address
    uint8_t val = SPI.transfer(0x00);             // read result
    deselect();                                   // deselect  radio module
    SPI.endTransaction();
    return val;
  }

  void writeReg(uint8_t regAddr, uint8_t val) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // select radio module
    SPI.transfer(regAddr);                        // send register address
    SPI.transfer(val);                            // send value
    deselect();                                   // deselect  radio module
    SPI.endTransaction();
  }

};
#endif

extern void* __gb_radio;

class NoRadio {
public:
  NoRadio () {}

  /// @brief Gibt zurück, ob bei einem Interupt auf die steigende oder fallende Flanke getriggert werden soll. Muss in der jeweiligen Radioklasse implementiert werden.
  /// @return 0 = FALLING, 1 = RISING
  uint8_t interruptMode() { return 0; };
  bool detectBurst () { return false; }
  void disable () {};
  void enable () {}
  void flushrx() {}
  uint8_t getGDO0 () { return 0; }
  bool init () { return true; }
  bool isIdle () { return true; }
  uint8_t read (__attribute__ ((unused)) Message& msg) { return 0; }
  uint8_t reset () { return 0; }
  uint8_t rssi () { return 0; }
  void setIdle () {}
  void setSendTimeout (__attribute__ ((unused)) uint16_t timeout=0) {}
  void waitTimeout (__attribute__ ((unused)) uint16_t timeout) {}
  void wakeup () {}
  void initReg(__attribute__ ((unused)) uint8_t val0, __attribute__ ((unused)) uint8_t val1) {}
  bool write (__attribute__ ((unused)) const Message& msg, __attribute__ ((unused)) uint8_t burst) { return false; }
};

template <class SPIType ,uint8_t GDO0, uint8_t PWRPIN=0xff, int SENDDELAY=100,class HWRADIO=CC1101<SPIType,PWRPIN> >
class Radio : public HWRADIO {

  static void isr () {
    instance().handleInt();
  }

  class MinSendTimeout : public Alarm {
    volatile bool wait;
  public:
    MinSendTimeout () : Alarm(0), wait(false) { async(true); }
    virtual ~MinSendTimeout () {}
    void waitTimeout () {
      // wait until time out over
      while( wait==true ) {
        // if( sysclock.runwait() == false ) {
          _delay_ms(1);
        // }
      }
      if( SENDDELAY > 0) {
        set(millis2ticks(SENDDELAY));
        // signal new wait cycle
        wait = true;
        // add to system clock
        sysclock.add(*this);
      }
    }

    void setTimeout (uint16_t millis=SENDDELAY) {
      if( millis > 0 ) {
        // cancel possible old timeout
        sysclock.cancel(*this);
        // set to 100ms
        set(millis2ticks(millis));
        // signal new wait cycle
        wait = true;
        // add to system clock
        sysclock.add(*this);
      }
    }

    virtual void trigger(__attribute__ ((unused)) AlarmClock& clock) {
      // signal wait cycle over
      wait = false;
    }
  } timeout;

public:
  //  this will delay next send by given millis
  void setSendTimeout(uint16_t millis=SENDDELAY) {
    timeout.setTimeout(millis);
  }
  // use the radio timer to wait given millis
  void waitTimeout (uint16_t millis) {
    timeout.setTimeout(millis);
    timeout.waitTimeout();
  }

  static Radio<SPIType,GDO0,PWRPIN,SENDDELAY,HWRADIO>& instance () {
    return *((Radio<SPIType,GDO0,PWRPIN,SENDDELAY,HWRADIO>*)__gb_radio);
  }

private:
  enum States { IDLE=0x1, SENDING=0x2, READ=0x4, ALIVE=0x8, READ_ALIVE=0xc };
  volatile uint8_t state;
  Message buffer;

  bool isState(States s) { return (state & s)==s; }
  void setState(States s) { state |= s; }
  void unsetState(States s) { state &= ~s; }

public:   //---------------------------------------------------------------------------------------------------------
  Radio () : state(ALIVE) {}

  bool init () {
    // ensure ISR if off before we start to init CC1101
    // OTA boot loader may leave it on
    disable();
    __gb_radio = this;
    DPRINT(F("Radio init"));
    pinMode(GDO0,INPUT);

    DPRINTLN(F("1"));

    bool initOK = HWRADIO::init();
    if (initOK) HWRADIO::wakeup(true);
    //DPRINT(F("CC init "));DPRINTLN(initOK ? F("OK"):F("FAIL"));
    return initOK;
  }

  void setIdle () {
    if( isState(IDLE) == false ) {
      HWRADIO::setIdle();
      setState(IDLE);
    }
  }

  void wakeup (bool flush=true) {
    if( isState(IDLE) == true ) {
      HWRADIO::wakeup(flush);
      unsetState(IDLE);
    }
  }

  bool isIdle () {
    return isState(IDLE);
  }

  void handleInt () {
    if( isState(SENDING) == false ) {
//      DPRINT(" * "); DPRINTLN(millis());
      setState(READ_ALIVE);
    }
  }

  bool clearAlive () {
    bool result = isState(ALIVE);
    unsetState(ALIVE);
    return result;
  }

  bool detectBurst () {
    if( isIdle() == true ) {
      wakeup();
      // let radio some time to get carrier signal
      _delay_ms(3);
    }
    return HWRADIO::detectBurst();
  }

  uint8_t getGDO0 () {
    return digitalRead(GDO0);
  }

void enable () {
#ifdef EnableInterrupt_h
  if( digitalPinToInterrupt(GDO0) == NOT_AN_INTERRUPT )
	// interruptMode() muss in der jeweiligen Radioklasse implementiert werden und 0 = FALLING oder 1 = RISING zurückgeben.
    enableInterrupt(GDO0, isr, HWRADIO::interruptMode() == 0 ? FALLING : RISING);
      else
#endif
        attachInterrupt(digitalPinToInterrupt(GDO0), isr, HWRADIO::interruptMode() == 0 ? FALLING : RISING);
}
void disable () {
#ifdef EnableInterrupt_h
  if( digitalPinToInterrupt(GDO0) == NOT_AN_INTERRUPT )
    disableInterrupt(GDO0);
  else
#endif
    detachInterrupt(digitalPinToInterrupt(GDO0));
}

  // read the message form the internal buffer, if any
  uint8_t read (Message& msg) {
    if( isState(READ) == false )
      return 0;

    unsetState(READ);
    uint8_t len = this->rcvData(buffer.buffer(),buffer.buffersize());
    if( len > 0 ) {
      buffer.length(len);
      // decode the message
      buffer.decode();
      // copy buffer to message
      memcpy(msg.buffer(),buffer.buffer(),len);
    }

    msg.length(len);
    // reset buffer
    buffer.clear();
    wakeup(false);
    return msg.length();
  }

  // try to read a message - not longer than timeout millis
  uint8_t read (Message& msg, uint32_t timeout) {
    uint8_t num = 0;
    uint32_t time=0;
    do {
      num = read(msg);
      if( num == 0 ) {
        _delay_ms(50); // wait 50ms
        time += 50;
      }
    }
    while( num == 0 && time < timeout );
    return num;
  }

  // simple send the message
  bool write (const Message& msg, uint8_t burst) {
    memcpy(buffer.buffer(),msg.buffer(),msg.length());
    buffer.length(msg.length());
    buffer.encode();
    return sndData(buffer.buffer(),buffer.length(),burst);
  }

  uint8_t sndData(uint8_t *buf, uint8_t size, uint8_t burst) {
    timeout.waitTimeout();
    this->wakeup();
    disable();
    setState(SENDING);
    uint8_t result = HWRADIO::sndData(buf,size,burst);
    unsetState(SENDING);
    enable();
    return result;
  }

};

template <class SPIType ,uint8_t GDO0, uint8_t PWRPIN=0xff, int SENDDELAY=100>
class CC1101Radio : public Radio<SPIType,GDO0, PWRPIN,SENDDELAY,CC1101<SPIType,PWRPIN>> {};

template <class SPIType ,uint8_t GDO0, uint8_t PWRPIN=0xff, int SENDDELAY=100>
class Si4431Radio : public Radio<SPIType,GDO0, PWRPIN,SENDDELAY,Si4431<SPIType,PWRPIN,GDO0>> {};

template <class SPIType, uint8_t GDO0, uint8_t PWRPIN = 0xff, int SENDDELAY = 100>
class RFM69Radio : public Radio<SPIType, GDO0, PWRPIN, SENDDELAY, RFM69<SPIType, PWRPIN, GDO0>> {};

}

#endif
