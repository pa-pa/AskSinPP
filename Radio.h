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
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef _CC_H
#define _CC_H

#include "Message.h"
#include "AlarmClock.h"

#ifdef ARDUINO_ARCH_AVR
  #include <util/delay.h>
  #define LIBCALL_ENABLEINTERRUPT
  #include <EnableInterrupt.h>
  typedef uint8_t BitOrder;
#endif

namespace as {

// CC1101 config register                         // Reset  Description
#define CC1101_IOCFG2           0x00                    // (0x29) GDO2 Output Pin Configuration
#define CC1101_IOCFG1           0x01                    // (0x2E) GDO1 Output Pin Configuration
#define CC1101_IOCFG0           0x02                    // (0x3F) GDO0 Output Pin Configuration
#define CC1101_FIFOTHR          0x03                    // (0x07) RX FIFO and TX FIFO Thresholds
#define CC1101_SYNC1            0x04                    // (0xD3) Sync Word, High Byte
#define CC1101_SYNC0            0x05                    // (0x91) Sync Word, Low Byte
#define CC1101_PKTLEN           0x06                    // (0xFF) Packet Length
#define CC1101_PKTCTRL1         0x07                    // (0x04) Packet Automation Control
#define CC1101_PKTCTRL0         0x08                    // (0x45) Packet Automation Control
#define CC1101_ADDR             0x09                    // (0x00) Device Address
#define CC1101_CHANNR           0x0A                    // (0x00) Channel Number
#define CC1101_FSCTRL1          0x0B                    // (0x0F) Frequency Synthesizer Control
#define CC1101_FSCTRL0          0x0C                    // (0x00) Frequency Synthesizer Control
#define CC1101_FREQ2            0x0D                    // (0x1E) Frequency Control Word, High Byte
#define CC1101_FREQ1            0x0E                    // (0xC4) Frequency Control Word, Middle Byte
#define CC1101_FREQ0            0x0F                    // (0xEC) Frequency Control Word, Low Byte
#define CC1101_MDMCFG4          0x10                    // (0x8C) Modem Configuration
#define CC1101_MDMCFG3          0x11                    // (0x22) Modem Configuration
#define CC1101_MDMCFG2          0x12                    // (0x02) Modem Configuration
#define CC1101_MDMCFG1          0x13                    // (0x22) Modem Configuration
#define CC1101_MDMCFG0          0x14                    // (0xF8) Modem Configuration
#define CC1101_DEVIATN          0x15                    // (0x47) Modem Deviation Setting
#define CC1101_MCSM2            0x16                    // (0x07) Main Radio Control State Machine Configuration
#define CC1101_MCSM1            0x17                    // (0x30) Main Radio Control State Machine Configuration
#define CC1101_MCSM0            0x18                    // (0x04) Main Radio Control State Machine Configuration
#define CC1101_FOCCFG           0x19                    // (0x36) Frequency Offset Compensation Configuration
#define CC1101_BSCFG            0x1A                    // (0x6C) Bit Synchronization Configuration
#define CC1101_AGCCTRL2         0x1B                    // (0x03) AGC Control
#define CC1101_AGCCTRL1         0x1C                    // (0x40) AGC Control
#define CC1101_AGCCTRL0         0x1D                    // (0x91) AGC Control
#define CC1101_WOREVT1          0x1E                    // (0x87) High Byte Event0 Timeout
#define CC1101_WOREVT0          0x1F                    // (0x6B) Low Byte Event0 Timeout
#define CC1101_WORCTRL          0x20                    // (0xF8) Wake On Radio Control
#define CC1101_FREND1           0x21                    // (0x56) Front End RX Configuration
#define CC1101_FREND0           0x22                    // (0x10) Front End RX Configuration
#define CC1101_FSCAL3           0x23                    // (0xA9) Frequency Synthesizer Calibration
#define CC1101_FSCAL2           0x24                    // (0x0A) Frequency Synthesizer Calibration
#define CC1101_FSCAL1           0x25                    // (0x20) Frequency Synthesizer Calibration
#define CC1101_FSCAL0           0x26                    // (0x0D) Frequency Synthesizer Calibration
#define CC1101_RCCTRL1          0x27                    // (0x41) RC Oscillator Configuration
#define CC1101_RCCTRL2          0x28                    // (0x00) RC Oscillator Configuration
#define CC1101_FSTEST           0x29                    // (0x59) Frequency Synthesizer Calibration Control
#define CC1101_PTEST            0x2A                    // (0x7F) Production Test
#define CC1101_AGCTEST          0x2B                    // (0x3F) AGC Test
#define CC1101_TEST2            0x2C                    // (0x88) Various Test Settings
#define CC1101_TEST1            0x2D                    // (0x31) Various Test Settings
#define CC1101_TEST0            0x2E                    // (0x0B) Various Test Settings

#define CC1101_PARTNUM          0x30                    // (0x00) Readonly: Chip ID
#define CC1101_VERSION          0x31                    // (0x04) Readonly: Chip ID
#define CC1101_FREQEST          0x32                    // (0x00) Readonly: Frequency Offset Estimate from Demodulator
#define CC1101_LQI              0x33                    // (0x00) Readonly: Demodulator Estimate for Link Quality
#define CC1101_RSSI             0x34                    // (0x00) Readonly: Received Signal Strength Indication
#define CC1101_MARCSTATE        0x35                    // (0x00) Readonly: Main Radio Control State Machine State
#define CC1101_WORTIME1         0x36                    // (0x00) Readonly: High Byte of WOR Time
#define CC1101_WORTIME0         0x37                    // (0x00) Readonly: Low Byte of WOR Time
#define CC1101_PKTSTATUS        0x38                    // (0x00) Readonly: Current GDOx Status and Packet Status
#define CC1101_VCO_VC_DAC       0x39                    // (0x00) Readonly: Current Setting from PLL Calibration Module
#define CC1101_TXBYTES          0x3A                    // (0x00) Readonly: Underflow and Number of Bytes
#define CC1101_RXBYTES          0x3B                    // (0x00) Readonly: Overflow and Number of Bytes
#define CC1101_RCCTRL1_STATUS   0x3C                    // (0x00) Readonly: Last RC Oscillator Calibration Result
#define CC1101_RCCTRL0_STATUS   0x3D                    // (0x00) Readonly: Last RC Oscillator Calibration Result

#define CC1101_PATABLE          0x3E                    // PATABLE address
#define CC1101_TXFIFO           0x3F                    // TX FIFO address
#define CC1101_RXFIFO           0x3F                    // RX FIFO address

#define CC1101_PA_TABLE0        0x40                    // (0x00) PA table, entry 0
#define CC1101_PA_TABLE1        0x41                    // (0x00) PA table, entry 1
#define CC1101_PA_TABLE2        0x42                    // (0x00) PA table, entry 2
#define CC1101_PA_TABLE3        0x43                    // (0x00) PA table, entry 3
#define CC1101_PA_TABLE4        0x44                    // (0x00) PA table, entry 4
#define CC1101_PA_TABLE5        0x45                    // (0x00) PA table, entry 5
#define CC1101_PA_TABLE6        0x46                    // (0x00) PA table, entry 6
#define CC1101_PA_TABLE7        0x47                    // (0x00) PA table, entry 7

// some register definitions for TRX868 communication
#define READ_SINGLE              0x80                   // type of transfers
#define READ_BURST               0xC0
#define WRITE_BURST              0x40

#define CC1101_CONFIG            0x80                   // type of register
#define CC1101_STATUS            0xC0

#define CC1101_SRES              0x30                   // reset CC1101 chip
#define CC1101_SFSTXON           0x31                   // enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). if in RX (with CCA): Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
#define CC1101_SXOFF             0x32                   // turn off crystal oscillator
#define CC1101_SCAL              0x33                   // calibrate frequency synthesizer and turn it off. SCAL can be strobed from IDLE mode without setting manual calibration mode (MCSM0.FS_AUTOCAL=0)
#define CC1101_SRX               0x34                   // enable RX. perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1
#define CC1101_STX               0x35                   // in IDLE state: enable TX. perform calibration first if MCSM0.FS_AUTOCAL=1. if in RX state and CCA is enabled: only go to TX if channel is clear
#define CC1101_SIDLE             0x36                   // exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable
#define CC1101_SWOR              0x38                   // start automatic RX polling sequence (Wake-on-Radio) as described in Section 19.5 if WORCTRL.RC_PD=0
#define CC1101_SPWD              0x39                   // enter power down mode when CSn goes high
#define CC1101_SFRX              0x3A                   // flush the RX FIFO buffer. only issue SFRX in IDLE or RXFIFO_OVERFLOW states
#define CC1101_SFTX              0x3B                   // flush the TX FIFO buffer. only issue SFTX in IDLE or TXFIFO_UNDERFLOW states
#define CC1101_SWORRST           0x3C                   // reset real time clock to Event1 value
#define CC1101_SNOP              0x3D                   // no operation. may be used to get access to the chip status byte

#define MARCSTATE_SLEEP          0x00
#define MARCSTATE_IDLE           0x01
#define MARCSTATE_XOFF           0x02
#define MARCSTATE_VCOON_MC       0x03
#define MARCSTATE_REGON_MC       0x04
#define MARCSTATE_MANCAL         0x05
#define MARCSTATE_VCOON          0x06
#define MARCSTATE_REGON          0x07
#define MARCSTATE_STARTCAL       0x08
#define MARCSTATE_BWBOOST        0x09
#define MARCSTATE_FS_LOCK        0x0A
#define MARCSTATE_IFADCON        0x0B
#define MARCSTATE_ENDCAL         0x0C
#define MARCSTATE_RX             0x0D
#define MARCSTATE_RX_END         0x0E
#define MARCSTATE_RX_RST         0x0F
#define MARCSTATE_TXRX_SWITCH    0x10
#define MARCSTATE_RXFIFO_OFLOW   0x11
#define MARCSTATE_FSTXON         0x12
#define MARCSTATE_TX             0x13
#define MARCSTATE_TX_END         0x14
#define MARCSTATE_RXTX_SWITCH    0x15
#define MARCSTATE_TXFIFO_UFLOW   0x16

#define PA_LowPower              0x03                       // PATABLE values
#define PA_Normal                0x50                       // PATABLE values
#define PA_MaxPower              0xC0



#ifdef ARDUINO_ARCH_AVR

template <uint8_t CS,uint8_t MOSI,uint8_t MISO,uint8_t SCLK>
class AvrSPI {

  uint8_t send (uint8_t data) {
    SPDR = data;                  // send byte
    while (!(SPSR & _BV(SPIF)));  // wait until transfer finished
    return SPDR;
  }

  void waitMiso () {
    while(digitalRead(MISO));
  }

public:
  void init () {
    pinMode(CS,OUTPUT);
    pinMode(MOSI,OUTPUT);
    pinMode(MISO,INPUT);
    pinMode(SCLK,OUTPUT);
    // SPI enable, master, speed = CLK/4
    SPCR = _BV(SPE) | _BV(MSTR);
  }

  void select () {
    digitalWrite(CS,LOW);
  }

  void deselect () {
    digitalWrite(CS,HIGH);
  }

  void ping () {
    select();                                     // wake up the communication module
    waitMiso();
    deselect();
  }

  uint8_t strobe(uint8_t cmd) {
    select();                                     // select CC1101
    waitMiso();                                     // wait until MISO goes low
    uint8_t ret = send(cmd);                  // send strobe command
    deselect();                                   // deselect CC1101
    return ret;
  }

  void readBurst(uint8_t * buf, uint8_t regAddr, uint8_t len) {
    select();                                     // select CC1101
    waitMiso();                                     // wait until MISO goes low
    send(regAddr | READ_BURST);                         // send register address
    for(uint8_t i=0 ; i<len ; i++) {
      buf[i] = send(0x00);                            // read result byte by byte
      //dbg << i << ":" << buf[i] << '\n';
    }
    deselect();                                   // deselect CC1101
  }

  void writeBurst(uint8_t regAddr, uint8_t* buf, uint8_t len) {
    select();                                     // select CC1101
    waitMiso();                                     // wait until MISO goes low
    send(regAddr | WRITE_BURST);                          // send register address
    for(uint8_t i=0 ; i<len ; i++)
      send(buf[i]);                  // send value
    deselect();                                   // deselect CC1101
  }

  uint8_t readReg(uint8_t regAddr, uint8_t regType) {
    select();                                     // select CC1101
    waitMiso();                                     // wait until MISO goes low
    send(regAddr | regType);                            // send register address
    uint8_t val = send(0x00);                           // read result
    deselect();                                   // deselect CC1101
    return val;
  }

  void writeReg(uint8_t regAddr, uint8_t val) {
    select();                                     // select CC1101
    waitMiso();                                     // wait until MISO goes low
    send(regAddr);                                // send register address
    send(val);                                  // send value
    deselect();                                   // deselect CC1101
  }

};

#endif


#ifdef SPI_MODE0

template <uint8_t CS,uint32_t CLOCK=2000000, BitOrder BITORDER=MSBFIRST, uint8_t MODE=SPI_MODE0>
class LibSPI {

public:
  LibSPI () {}
  void init () {
    pinMode(CS,OUTPUT);
    SPI.begin();
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

  uint8_t strobe(uint8_t cmd) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // select CC1101
    uint8_t ret = SPI.transfer(cmd);
    deselect();                                   // deselect CC1101
    SPI.endTransaction();
    return ret;
  }

  void readBurst(uint8_t * buf, uint8_t regAddr, uint8_t len) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // select CC1101
    SPI.transfer(regAddr | READ_BURST);                         // send register address
    for(uint8_t i=0 ; i<len ; i++) {
      buf[i] = SPI.transfer(0x00);                            // read result byte by byte
      //dbg << i << ":" << buf[i] << '\n';
    }
    deselect();                                   // deselect CC1101
    SPI.endTransaction();
  }

  void writeBurst(uint8_t regAddr, uint8_t* buf, uint8_t len) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // select CC1101
    SPI.transfer(regAddr | WRITE_BURST);                          // send register address
    for(uint8_t i=0 ; i<len ; i++)
      SPI.transfer(buf[i]);                  // send value
    deselect();                                   // deselect CC1101
    SPI.endTransaction();
  }

  uint8_t readReg(uint8_t regAddr, uint8_t regType) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // select CC1101
    SPI.transfer(regAddr | regType);                            // send register address
    uint8_t val = SPI.transfer(0x00);                           // read result
    deselect();                                   // deselect CC1101
    SPI.endTransaction();
    return val;
  }

  void writeReg(uint8_t regAddr, uint8_t val) {
    SPI.beginTransaction(SPISettings(CLOCK,BITORDER,MODE));
    select();                                     // select CC1101
    SPI.transfer(regAddr);                                // send register address
    SPI.transfer(val);                                  // send value
    deselect();                                   // deselect CC1101
    SPI.endTransaction();
  }

};
#endif


static void* instance;

template <class SPIType ,uint8_t GDO0>
class Radio {

  static void isr () {
    ((Radio<SPIType,GDO0>*)instance)->handleInt();
  }

  class MinSendTimeout : public Alarm {
    volatile bool wait;
  public:
    MinSendTimeout () : Alarm(0), wait(false) { async(true); }
    virtual ~MinSendTimeout () {}
    void waitTimeout () {
      // wait until time out over
      while( wait==true ) _delay_us(1);
      set(millis2ticks(100));
      // signal new wait cycle
      wait = true;
      // add to system clock
      sysclock.add(*this);
    }

    void setTimeout () {
      // cancel possible old timeout
      sysclock.cancel(*this);
      // set to 100ms
      set(millis2ticks(100));
      // signal new wait cycle
      wait = true;
      // add to system clock
      sysclock.add(*this);
    }

    virtual void trigger(__attribute__ ((unused)) AlarmClock& clock) {
      // signal wait cycle over
      wait = false;
    }
  } timeout;

private:
  SPIType spi;

  uint8_t rss;                                      // signal strength
  uint8_t lqi;                                      // link quality
  volatile uint8_t intread;
  volatile bool idle;
  Message buffer;

public:   //---------------------------------------------------------------------------------------------------------
  void setIdle () {
    if( idle == false ) {
      uint8_t cnt = 0xff;
      while(cnt-- && (spi.strobe(CC1101_SIDLE) & 0x70) != 0) {
        _delay_us(10);
      }
      spi.strobe(CC1101_SFRX);
      spi.strobe(CC1101_SPWD);                            // enter power down state
      idle = true;
    }
  }

  void wakeup () {
    if( idle == true ) {
      spi.ping();
      flushrx();
      idle = false;
    }
  }

  Radio () : rss(0), lqi(0), intread(0), idle(false) {}

  void init () {
    // ensure ISR if off before we start to init CC1101
    // OTA boot loader may leave it on
    disable();
    instance = this;
    DPRINT(F("CC init"));
    spi.init();                                     // init the hardware to get access to the RF modul
    pinMode(GDO0,INPUT);

    DPRINT(F("1"));
    spi.deselect();                                   // some deselect and selects to init the TRX868modul
    _delay_us(5);
    spi.select();
    _delay_us(10);
    spi.deselect();
    _delay_us(41);

    spi.strobe(CC1101_SRES);                                // send reset
    _delay_ms(10);

    // define init settings for TRX868
    static const uint8_t initVal[] PROGMEM = {
      CC1101_IOCFG2,    0x2E, //                        // non inverted GDO2, high impedance tri state
  //    CC1101_IOCFG1,    0x2E, // (default)                  // low output drive strength, non inverted GD=1, high impedance tri state
      CC1101_IOCFG0,    0x06, // packet CRC ok                // disable temperature sensor, non inverted GDO0,
      CC1101_FIFOTHR,   0x0D,                         // 0 ADC retention, 0 close in RX, TX FIFO = 9 / RX FIFO = 56 byte
      CC1101_SYNC1,     0xE9,                         // Sync word
      CC1101_SYNC0,     0xCA,
      CC1101_PKTLEN,    0x3D,                         // packet length has to be set to 61
      CC1101_PKTCTRL1,  0x0C,                         // PQT = 0, CRC auto flush = 1, append status = 1, no address check
      CC1101_PKTCTRL0,  0x45,
      CC1101_FSCTRL1,   0x06,                         // frequency synthesizer control

      // 868.299866 MHz
      CC1101_FREQ2,     0x21,
      CC1101_FREQ1,     0x65,
      CC1101_FREQ0,     0x6A,

      // 868.2895508
      //CC1101_FREQ2,     0x21,
      //CC1101_FREQ1,     0x65,
      //CC1101_FREQ0,     0x50,

      CC1101_MDMCFG4,  0xC8,
      CC1101_MDMCFG3,  0x93,
      CC1101_MDMCFG2,  0x03,
      CC1101_MDMCFG1,  0x22,
      CC1101_DEVIATN,  0x34,                          // 19.042969 kHz
      CC1101_MCSM2,    0x01,
      CC1101_MCSM1,    0x33,
      CC1101_MCSM0,    0x18,
      CC1101_FOCCFG,   0x16,
      CC1101_AGCCTRL2, 0x43,
      //CC1101_WOREVT1, 0x28,                         // tEVENT0 = 50 ms, RX timeout = 390 us
      //7CC1101_WOREVT0, 0xA0,
      //CC1101_WORCTRL, 0xFB,                         //EVENT1 = 3, WOR_RES = 0
      CC1101_FREND1,  0x56,
      CC1101_FSCAL1,  0x00,
      CC1101_FSCAL0,  0x11,
      CC1101_FSTEST,  0x59,
      CC1101_TEST2,   0x81,
      CC1101_TEST1,   0x35,
      CC1101_PATABLE, 0xC3,
    };
    for (uint8_t i=0; i<sizeof(initVal); i+=2) {                    // write init value to TRX868
      spi.writeReg(pgm_read_byte(&initVal[i]), pgm_read_byte(&initVal[i+1]));
    }
    DPRINT(F("2"));
    spi.strobe(CC1101_SCAL);                                // calibrate frequency synthesizer and turn it off
    while (spi.readReg(CC1101_MARCSTATE, CC1101_STATUS) != 1) {               // waits until module gets ready
      _delay_us(1);
      DPRINT(F("."));
    }
    DPRINT(F("3"));
    spi.writeReg(CC1101_PATABLE, PA_MaxPower);                        // configure PATABLE
    flushrx();
    spi.strobe(CC1101_SWORRST);                               // reset real time clock
    DPRINTLN(F(" - ready"));
  }

  void handleInt () {
    // DPRINTLN("*");
    intread = 1;
  }

  uint8_t getGDO0 () {
    return digitalRead(GDO0);
  }

  void enable () {
    enableInterrupt(GDO0, isr, FALLING);
  }
  void disable () {
    disableInterrupt(GDO0);
  }

  // read the message form the internal buffer, if any
  uint8_t read (Message& msg) {
    if( intread == 0 )
      return 0;

    uint8_t len = rcvData(buffer.buffer(),buffer.buffersize());
    if( len > 0 ) {
      buffer.length(len);
      // decode the message
      buffer.decode();
      // copy buffer to message
      memcpy(msg.buffer(),buffer.buffer(),len);
    }
    else {
      intread = 0;
    }

    msg.length(len);
    // reset buffer
    buffer.clear();
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

  bool readAck (const Message& msg) {
    if( intread == 0 )
      return false;

    bool ack=false;
    uint8_t len = rcvData(buffer.buffer(),buffer.buffersize());
    if( len > 0 ) {
      buffer.length(len);
      // decode the message
      buffer.decode();
      ack = buffer.isAck() &&
           (buffer.from() == msg.to()) &&
           (buffer.to() == msg.from()) &&
           (buffer.count() == msg.count());
      // reset buffer
      buffer.clear();
    }
    else {
      intread = 0;
    }
    return ack;
  }

  uint8_t rssi () const {
    return rss;
  }

protected:
  uint8_t sndData(uint8_t *buf, uint8_t size, uint8_t burst) {
    timeout.waitTimeout();
    wakeup();

    uint8_t i=200;
    do {
      spi.strobe(CC1101_STX);
      _delay_us(10);
      if( --i == 0 ) {
        // can not enter TX state - reset fifo
        spi.strobe(CC1101_SFTX  );
        spi.strobe(CC1101_SIDLE );
        spi.strobe(CC1101_SNOP );
        // back to RX mode
        do { spi.strobe(CC1101_SRX);
        } while (spi.readReg(CC1101_MARCSTATE, CC1101_STATUS) != MARCSTATE_RX);
        return false;
      }
    }
    while(spi.readReg(CC1101_MARCSTATE, CC1101_STATUS) != MARCSTATE_TX);

    _delay_ms(10);
    if (burst) {         // BURST-bit set?
      _delay_ms(350);    // according to ELV, devices get activated every 300ms, so send burst for 360ms
    }

    spi.writeReg(CC1101_TXFIFO, size);
    spi.writeBurst(CC1101_TXFIFO, buf, size);           // write in TX FIFO

    for(uint8_t i = 0; i < 200; i++) {  // after sending out all bytes the chip should go automatically in RX mode
      if( spi.readReg(CC1101_MARCSTATE, CC1101_STATUS) == MARCSTATE_RX)
        break;                                    //now in RX mode, good
      _delay_us(10);
    }

    return true;
  }

  uint8_t rcvData(uint8_t *buf, uint8_t size) {
    uint8_t packetBytes = 0;
    uint8_t rxBytes = 0;
    uint8_t fifoBytes = spi.readReg(CC1101_RXBYTES, CC1101_STATUS);             // how many bytes are in the buffer
    // DPRINT("RX FIFO: ");DHEXLN(fifoBytes);
    // overflow detected - flush the FIFO
    if( fifoBytes > 0 && (fifoBytes & 0x80) != 0x80 ) {
      packetBytes = spi.readReg(CC1101_RXFIFO, CC1101_CONFIG); // read packet length
      // DPRINT("Start Packet: ");DHEXLN(packetBytes);
      // check that packet fits into the buffer
      if (packetBytes <= size) {
        spi.readBurst(buf, CC1101_RXFIFO, packetBytes);          // read data packet
        rss = spi.readReg(CC1101_RXFIFO, CC1101_CONFIG);         // read RSSI
        if (rss >= 128) rss = 255 - rss;
        rss /= 2; rss += 72;
        uint8_t val = spi.readReg(CC1101_RXFIFO, CC1101_CONFIG); // read LQI and CRC_OK
        lqi = val & 0x7F;
        if( (val & 0x80) == 0x80 ) { // check crc_ok
          // DPRINTLN("CRC OK");
          rxBytes = packetBytes;
        }
        else {
          DPRINTLN("CRC Failed");
        }
        do { spi.strobe(CC1101_SRX); }
        while (spi.readReg(CC1101_MARCSTATE, CC1101_STATUS) != MARCSTATE_RX);
      }
      else {
        DPRINT("Packet too big: ");DDECLN(packetBytes);
        flushrx();
      }
    }
  //  DPRINT("-> ");
  //  DHEX(buf,buf[0]);

    switch(spi.readReg(CC1101_MARCSTATE, CC1101_STATUS)) {
      case MARCSTATE_RXFIFO_OFLOW:
        spi.strobe(CC1101_SFRX);
      case MARCSTATE_IDLE:
        spi.strobe(CC1101_SIDLE);
        spi.strobe(CC1101_SNOP);
        spi.strobe(CC1101_SRX);
        break;
    }
    return rxBytes; // return number of byte in buffer
  }

  void flushrx () {
    spi.strobe(CC1101_SFRX);                                // flush Rx FIFO
    spi.strobe(CC1101_SIDLE);                               // enter IDLE state
    spi.strobe(CC1101_SNOP);
    spi.strobe(CC1101_SRX);                                 // back to RX state
  }

};

}

#endif
