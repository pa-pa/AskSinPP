//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-03-31 stan23 Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- -----------------------------------------------------------------------------------------------------------------------
// AskSin SX1262 functions -----------------------------------------------------------------------------------------------
// 2023.09.14 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef _SX1262_H
#define _SX1262_H

namespace as {

// radio commands
#define SX1262_GET_STATUS                 0xC0
#define SX1262_WRITE_REGISTER             0x0D
#define SX1262_READ_REGISTER              0x1D
#define SX1262_WRITE_BUFFER               0x0E
#define SX1262_READ_BUFFER                0x1E
#define SX1262_SET_SLEEP                  0x84
#define SX1262_SET_STANDBY                0x80
#define SX1262_SET_FS                     0xC1
#define SX1262_SET_TX                     0x83
#define SX1262_SET_RX                     0x82
#define SX1262_SET_RXDUTYCYCLE            0x94
#define SX1262_SET_CAD                    0xC5
#define SX1262_SET_TXCONTINUOUSWAVE       0xD1
#define SX1262_SET_TXCONTINUOUSPREAMBLE   0xD2
#define SX1262_SET_PACKETTYPE             0x8A
#define SX1262_GET_PACKETTYPE             0x11
#define SX1262_SET_RFFREQUENCY            0x86
#define SX1262_SET_TXPARAMS               0x8E
#define SX1262_SET_PACONFIG               0x95
#define SX1262_SET_CADPARAMS              0x88
#define SX1262_SET_BUFFERBASEADDRESS      0x8F
#define SX1262_SET_MODULATIONPARAMS       0x8B
#define SX1262_SET_PACKETPARAMS           0x8C
#define SX1262_GET_RXBUFFERSTATUS         0x13
#define SX1262_GET_PACKETSTATUS           0x14
#define SX1262_GET_RSSIINST               0x15
#define SX1262_GET_STATS                  0x10
#define SX1262_RESET_STATS                0x00
#define SX1262_CFG_DIOIRQ                 0x08
#define SX1262_GET_IRQSTATUS              0x12
#define SX1262_CLR_IRQSTATUS              0x02
#define SX1262_CALIBRATE                  0x89
#define SX1262_CALIBRATEIMAGE             0x98
#define SX1262_SET_REGULATORMODE          0x96
#define SX1262_GET_ERROR                  0x17
#define SX1262_SET_TCXOMODE               0x97
#define SX1262_SET_TXFALLBACKMODE         0x93
#define SX1262_SET_RFSWITCHMODE           0x9D
#define SX1262_SET_STOPRXTIMERONPREAMBLE  0x9F
#define SX1262_SET_LORASYMBTIMEOUT        0xA0

#define SX1262_STDBY_RC                   0x00    // Using the STDBY_RC standby mode allow to reduce the energy consumption
#define SX1262_STDBY_XOSC                 0x01    // STDBY_XOSC should be used for time critical applications

#define SX1262_MODE_SLEEP                 0x00    // in sleep mode
#define SX1262_MODE_STDBY_RC              0x01    // in standby mode with RC oscillator
#define SX1262_MODE_STDBY_XOSC            0x02    // in standby mode with XOSC oscillator
#define SX1262_MODE_FS                    0x03    // in frequency synthesis mode
#define SX1262_MODE_TX                    0x04    // in transmit mode
#define SX1262_MODE_RX                    0x05    // in receive mode
#define SX1262_MODE_RX_DC                 0x06    // in receive duty cycle mode
#define SX1262_MODE_CAD                   0x07    // in channel activity detection mode

#define SX1262_PACKET_TYPE_GFSK           0x00
#define SX1262_PACKET_TYPE_LORA           0x01
#define SX1262_PACKET_TYPE_NONE           0x0F


#if defined STM32WL
// #include <SubGhz.h> must be done on top of the main sketch 

class SubGhzSPI {
public:
  SubGhzSPI() {}

  void init() {
    SubGhz.setResetActive(false);                   // start the rf modul hardware
    SubGhz.SPI.begin();                             // start the spi interface
  }

  void select() {
    SubGhz.SPI.beginTransaction(SubGhz.spi_settings);
    SubGhz.setNssActive(true);
    while (SubGhz.isBusy());
  }

  void deselect() {
    SubGhz.setNssActive(false);
    SubGhz.SPI.endTransaction();
  }

  /*void shutdown() {
    SubGhz.SPI.end();                               // shutdown spi interface
    SubGhz.setNssActive(false);                     // deselect rf module
    SubGhz.setResetActive(true);                    // shutdown the rf module
  }


  void ping() {
    //SubGhz.SPI.beginTransaction(SubGhz.spi_settings);
    select();                                     // wake up the communication module
    SubGhz.SPI.transfer(0);
    deselect();
    //SubGhz.SPI.endTransaction();
  }

  void waitMiso() {
  }

  uint8_t send(uint8_t data) {
    // needs a check why we have it and whats the difference to strobe
    DPRINTLN("send");
    SubGhz.SPI.beginTransaction(SubGhz.spi_settings);
    uint8_t ret = SubGhz.SPI.transfer(data);
    SubGhz.SPI.endTransaction();
    return ret;
  }

  uint8_t strobe(uint8_t cmd) {
    //SubGhz.SPI.beginTransaction(SubGhz.spi_settings);
    select();
    uint8_t ret = SubGhz.SPI.transfer(cmd);
    deselect();
    //SubGhz.SPI.endTransaction();
    return ret;
  }*/

  void readBurst(uint8_t regAddr, uint8_t* buf, uint8_t len) {
    select();
    SubGhz.SPI.transfer(regAddr);
    for (uint8_t i = 0; i < len; i++) {
      buf[i] = SubGhz.SPI.transfer(0x00);           // read result byte by byte
    }
    deselect();                                     // deselect  radio module
  }
  uint8_t readBurst(uint8_t regAddr) {
    uint8_t ret;
    readBurst(regAddr, &ret, 1);
    return ret;
  }

  void writeBurst(uint8_t command, uint8_t* buf, uint8_t len) {
    select();                                       // select radio module, wait till ready and start SPI interface
    SubGhz.SPI.transfer(command);
    for (uint16_t i = 0; i < len; i++)
      SubGhz.SPI.transfer(buf[i]);                  // write content to the register
    //DPRINT(F("writeBurst 0x")); DHEX(command); DPRINT(F(":")); DHEXLN(buf, len);
    deselect();                                     // deselect  radio module
    _delay_us(100);
  }
  void writeBurst(uint8_t command, uint8_t value) {
    writeBurst(command, &value, 1);
  }

  void readBuffer(uint8_t offset, uint8_t* buf, uint8_t len) {
    select();                                       // select radio module, wait till ready and start SPI interface
    SubGhz.SPI.transfer(SX1262_READ_BUFFER);
    SubGhz.SPI.transfer(offset);
    SubGhz.SPI.transfer(0);
    for (uint8_t i = 0; i < len; i++)
      buf[i] = SubGhz.SPI.transfer(0);              // read buffer
    //DPRINT(F("readBuffer 0x")); DHEX(offset); DPRINT(": "); DHEXLN(buf, len);
    deselect();                                     // deselect  radio module
  }
  void writeBuffer(uint8_t offset, uint8_t* buf, uint8_t len) {
    select();                                       // select radio module, wait till ready and start SPI interface
    SubGhz.SPI.transfer(SX1262_WRITE_BUFFER);
    SubGhz.SPI.transfer(offset);
    //SubGhz.SPI.transfer(0);
    for (uint8_t i = 0; i < len; i++)
      SubGhz.SPI.transfer(buf[i]);              // write buffer
    //DPRINT(F("writeBuffer 0x")); DHEX(offset); DPRINT(": "); DHEXLN(buf, len);
    deselect();                                     // deselect  radio module
  }

  void readRegister(uint16_t regAddr, uint8_t* buf, uint8_t len, uint8_t regType = SX1262_READ_REGISTER) {
    select();

    SubGhz.SPI.transfer(regType);                   // address register write
    SubGhz.SPI.transfer((regAddr & 0xFF00) >> 8);   // upper byte of address
    SubGhz.SPI.transfer(regAddr & 0x00FF);          // lower byte
    SubGhz.SPI.transfer(0x00);                      // read status

    for (uint16_t i = 0; i < len; i++)
      buf[i] = SubGhz.SPI.transfer(0x00);           // read result byte by byte
    //DPRINT(F("readRegister 0x")); DHEX(regAddr); DPRINT(F(":")); DHEXLN(buf, len);
    deselect();                                     // deselect  radio module
  }
  uint8_t readRegister(uint8_t regAddr) {
    uint8_t data;
    readRegister(regAddr, &data, 1);
    return data;
  }

  void writeRegister(uint16_t regAddr, uint8_t* buf, uint8_t len, uint8_t regType = SX1262_WRITE_REGISTER) {
    select();                                       // select radio module, wait till ready

    SubGhz.SPI.transfer(regType);                   // address register write
    SubGhz.SPI.transfer((regAddr & 0xFF00) >> 8);   // upper byte of address
    SubGhz.SPI.transfer(regAddr & 0x00FF);          // lower byte

    for (uint16_t i = 0; i < len; i++)
      SubGhz.SPI.transfer(buf[i]);                  // write content to the register
    //DPRINT(F("writeRegister 0x")); DHEX(regAddr); DPRINT(F(":")); DHEXLN(buf, len);
    deselect();                                     // deselect  radio module
  }
  void writeRegister(uint16_t regAddr, uint8_t val) {
    writeRegister(regAddr, &val, 1);
  }

};

template <class SPIType>
class STM32WLx {
protected:
  /** calculate CRC-16 on buf with size len */
  uint16_t calcCrcWorker(uint8_t crcData, uint16_t crcReg) {
    for (uint8_t i = 0; i < 8; i++) {
      if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
        crcReg = (crcReg << 1) ^ 0x8005;
      else
        crcReg = (crcReg << 1);
      crcData <<= 1;
    }
    return crcReg;
  }

  uint16_t calculateCrc(uint8_t* buf, uint8_t len) {
    uint16_t crc = 0xFFFF;
    crc = calcCrcWorker(len, crc);
    for (uint8_t i = 0; i < len; i++) {
      crc = calcCrcWorker(buf[i], crc);
    }
    return crc;
  }

  void whitenBuffer(uint8_t* buf, uint8_t len, uint8_t offset = 0) {
    const uint8_t pn9[] = {
      0xFF, 0xE1, 0x1D, 0x9A, 0xED, 0x85, 0x33, 0x24, 0xEA, 0x7A,
      0xD2, 0x39, 0x70, 0x97, 0x57, 0x0A, 0x54, 0x7D, 0x2D, 0xD8,
      0x6D, 0x0D, 0xBA, 0x8F, 0x67, 0x59, 0xC7, 0xA2, 0xBF, 0x34,
      0xCA, 0x18, 0x30, 0x53, 0x93, 0xDF, 0x92, 0xEC, 0xA7, 0x15
    };

    if (len + offset < sizeof(pn9)) {
      for (uint8_t i = 0; i < len; i++) {
        buf[i] = buf[i] ^ pn9[i + offset];
      }
    }
    else {
      DPRINTLN(F("ERR: packet too large to be whitened!"));
    }
  }

public:
  SPIType spi;
  uint8_t rss;   // signal strength

public:
  STM32WLx() : rss(0) {}

  bool init(callback_function_t callback) {
    //DPRINT(F("SX1262 init "));
    bool initOK = true;
    
    spi.init();
    DPRINT('1');
    reset();
    DPRINT('2');

    // - register settings -----------------------------------------------------------------
    //Frequency: 868.299866 MHz
    //Deviation: 20 kHz
    //Datarate : 10 kBaud
    //Preamble : 4 bytes
    //SyncWords : 0xE9 0xCA 0xE9 0xCA (4 bytes)

    // Sync Word Programming - Byte 0 0x06C0, Byte 1 0x06C1, ... 
    // SyncWords: 0xE9 0xCA 0xE9 0xCA (4 bytes)
    uint8_t syncWord[8] = { 0xE9, 0xCA, 0xE9, 0xCA, 0, 0, 0, 0 };
    spi.writeRegister(0x06C0, syncWord, 4);

    // Set to standby mode to start chip configure
    spi.writeBurst(SX1262_SET_STANDBY, SX1262_STDBY_XOSC);

    // Define the protocol(LoRa® or FSK) with the command SetPacketType(...)
    spi.writeBurst(SX1262_SET_PACKETTYPE, SX1262_PACKET_TYPE_GFSK);

    // Define the RF frequency with the command SetRfFrequency(...)
    // rfFreq = Frequency * 2^25 / Fxtal :: rfFreq = 868300000 * 33554432 / 32000000 :: rfFreq = 910.478.400 or 0x3644CC40
    // test   867.299866: 909.429.824, 3634 CC40
    // asksin 868.299866: 910.478.400, 3644 CC40
    uint8_t freq[4] = { 0x36, 0x44, 0xCC, 0x40 };
    spi.writeBurst(SX1262_SET_RFFREQUENCY, freq, 4);

    // Define the Power Amplifier configuration with the command SetPaConfig(...)
    // datasheet, ppage 76
    uint8_t paConf[4] = {0x03, 0x05, 0x00, 0x01};
    spi.writeBurst(SX1262_SET_PACONFIG, paConf, 4);

    // Define output power and ramping time with the command SetTxParams(...)
    // datasheet, ppage 84
    uint8_t txParam[2] = { 0x16, 0x00 };
    spi.writeBurst(SX1262_SET_TXPARAMS, txParam, 2);


    // Define the modulation parameter according to the chosen protocol with the command SetModulationParams(...)
    // byte 1-3 Bitrate: br = 32 * Fxtal / bit rate :: br = 32 * 32000000 / 10000; :: br = 102471 or 0x019047
    // byte 4 Pulse shape: off
    // byte 5 Bandwith: see datasheet, ox19 RX_BW_312000 (312 kHz DSB)
    // byte 6-8 Deviation: fdev = (Frequency Deviation * 2^25) / Fxtal :: fdev = 20000 * 33554432 / 32000000 :: freqDev = 19968 or 0x4E00
    uint8_t mod[8] = { 0x01, 0x90, 0x47, 0x00, 0x19, 0x00, 0x4E, 0x00 };
    spi.writeBurst(SX1262_SET_MODULATIONPARAMS, mod, 8);

    // Define the frame format to be used with the command SetPacketParams(...)
    // byte 1-2 PreambleLength TX: size in bit
    // byte 3 PreambleDetectorLength: see datasheet, 0x06 detect 24 bit, 0x07 detect 32 bits
    // byte 4 SyncWordLength: in bit (going from 0 to 8 bytes)
    // byte 5 AddrComp: 0x00 address filtering disable
    // byte 6 PacketType: 0x00 on fixed size, otherwise packet size byte is not shown in buffer
    // byte 7 PayloadLength: size of payload 30 byte 0x1e
    // byte 8 CRCType: 0x01 CRC_OFF (No CRC) - must be done manually, as whitening is on a different approach
    // byte 9 Whitening: 0x00 whitening disabled, as CC1101 has a different standard
    uint8_t par[9] = { 0x00, 0x20, 0x06, 0x20, 0x00, 0x00, 0x1e, 0x01, 0x00 };
    spi.writeBurst(SX1262_SET_PACKETPARAMS, par, 9);

    // TX specific settings
    // TODO

    // Define where the data will be stored inside the data buffer in Rx with the command SetBufferBaseAddress(...)
    // byte 1: TX base address, byte 2: RX base address - both are set to 0 
    uint8_t base[2] = { 0x00, 0x00 };
    spi.writeBurst(SX1262_SET_BUFFERBASEADDRESS, base, 2);

    // Image Calibration for Specific Frequency Bands
    // byte 1-2: see datasheet, 863 - 870 MHz 0xD7 0xDB
    uint8_t cal[2] = { 0xD7, 0xD8 };
    spi.writeBurst(SX1262_CALIBRATEIMAGE, cal, 2);

    // Configure DIO and irq: use the command SetDioIrqParams(...) 
    // byte 1-2 Irq Mask(15:0); byte 3-4 DIO1Mask(15:0); byte 5-6 DIO2Mask(15:0); byte 7-8 DIO3Mask(15:0)
    // bit 0: TxDone, bit 1: RxDone, bit 2: PreambleDetected, bit 3: SyncWordValid, bit 6: CrcErr, bit 9: Rx or Tx timeout
    uint8_t mask[8] = { 0x0, 0x8, 0x0, 0x8, 0x00, 0x00, 0x00, 0x00 };
    spi.writeBurst(SX1262_CFG_DIOIRQ, mask, 8);

    // switch in RX mode
    uint8_t rxTO[3] = { 0,0,0 };
    spi.writeBurst(SX1262_SET_RX, rxTO, 3); 
    DPRINT('3');

    // start receive mode
    SubGhz.attachInterrupt(callback);
    DPRINTLN(F(" - ready"));

    // some readings of the chip
    return initOK;
  }
  uint8_t reset() {
    DPRINT('R');
    // TODO: return value is never used, neither for CC1101 nor Si4431
    return 0;
  }
  void tuneFreq(__attribute__((unused)) uint8_t freq2, __attribute__((unused)) uint8_t freq1, __attribute__((unused)) uint8_t freq0) {
    // SX1262 does not have problems with wrong oscillator capacitors on cheap modules
  }

  void enableInterrupt() {
    SubGhz.clearPendingInterrupt();
    SubGhz.enableInterrupt();
  }
  void disableInterrupt() {
    SubGhz.disableInterrupt();
  }
  uint8_t interruptMode() {
    return 0; 
  }

  void setIdle () {
    DPRINTLN("setIdleWLx");

#ifdef USE_WOR
    DPRINTLN("ERROR: WOR is not implemented for STM32WLx!");
#else
    // enter power down state
#endif
  }
  void wakeup (bool flush) {
    DPRINTLN("wakeupWLx");

  }
  bool detectBurst() {
    DPRINTLN("detectBurstWLx");
    // TODO: how to detect a burst?
    return 0;
  }

  uint8_t rssi () const {
    return rss;
  }
  void pollRSSI() {
    DPRINTLN("pollRSSIWLx");
    //calculateRSSI(readReg(SI4431_REG_RSSI, 0));         // read RSSI from STATUS register
  }

  /* not needed as we do not use fsk modem packet handling
  void flushRx () {
    DPRINTLN("flushRxWLx");
  }*/

  uint8_t sndData(uint8_t *buf, uint8_t size, __attribute__ ((unused)) uint8_t burst) {
    disableInterrupt();
    spi.writeBurst(SX1262_SET_STANDBY, 1);                // go to standby mode, to read buffer and restart rx

    // packethandling of SX1262 does not work as whitening algorythm is different
    // therefor we calculate and add crc and whiten the string in software 
    //DPRINT("  buf: ");DHEX(buf, size);DPRINTLN("");
    uint16_t crc;
    uint8_t packetBuffer[40];

    crc = calculateCrc(buf, size);
    //DPRINT("  calculated CRC: "); DHEX(crc); DPRINTLN("");

    // build packet: first byte is the length
    packetBuffer[0] = size;
    // then the encoded payload (len-1 bytes)
    memcpy(&packetBuffer[1], buf, size);
    // then 2 bytes of CRC
    packetBuffer[size+1] = crc >> 8;
    packetBuffer[size+2] = crc & 0xFF;
    //DPRINT("packet:    "); DHEX(packetBuffer, size+3); DPRINTLN("");

    whitenBuffer(packetBuffer, size+3);
    //DPRINT("packet wh: "); DHEX(packetBuffer, size+3); DPRINTLN("");

    //uint8_t irqstat[3];
    //spi.readBurst(SX1262_GET_IRQSTATUS, irqstat, 3);    // read the irq status
    //DPRINT("IRQ stat: "); DHEXLN(irqstat, 3);

    //uint8_t txParam[2] = { 0x0e,0x03 };                 
    //spi.writeBurst(SX1262_SET_TXPARAMS, txParam, 2);

    //SetStandby
    //SetPacketType
    //SetRfFrequency
    //SetPaConfig
    //SetTxParams
    //SetBufferBaseAddress
    spi.writeBuffer(0, packetBuffer, size+3);           // write prepared string into the tx buffer
    //SetModulationParams
    //SetPacketParams
    //uint8_t xbuf[100];
    //spi.readBuffer(0, xbuf, 100);
    //DHEXLN(xbuf, 100);

    uint8_t par[9] = { 0x00, 0x20, 0x06, 0x20, 0x00, 0x00, (size+3), 0x01, 0x00 };
    spi.writeBurst(SX1262_SET_PACKETPARAMS, par, 9);


    //SetTx - Timeout duration = Timeout * 15.625 µs - Timeout = Timeout duration / 15.625 µs
    // a 30 byte string needs ~40ms to be transmitted, 40ms = 40000us / 15.625 µs = 0x0A00
    uint8_t txTO[3] = { 0x00,0xa0,0x00 };                 // TX timeout mode
    spi.writeBurst(SX1262_SET_TX, txTO, 3);

    //loop till tx has finished or timed out, 06: Command TX done
    uint8_t i = 20; uint8_t cp_stat = 0;
    while ((cp_stat != 6) && (--i != 0)) {
      cp_stat = ((spi.readBurst(SX1262_GET_STATUS) & 0b00001110) >> 1);
      //DPRINT("cp_stat: "); DPRINTLN(cp_stat);
      _delay_ms(5);
    }


    spi.writeBurst(SX1262_SET_STANDBY, 1);                // go to standby mode, to restart in tx






    // clear interrupts


    // no TX packet handling, therefore preamble & syncwords have to be sent manually

    if (burst) {
      //DPRINTLN("Send Burst");

      //set TX on
    }


    // set mode to transmit


    

    //set nIRQ to trigger RX received
    setRXmode();                                          // enable Rx

    return true;
  }
  uint8_t rcvData(uint8_t* buf, uint8_t size) {
    // fsk modem packet control is not working because of different whitening algorythm
    // interrupt is raised while syncword was detected, therefor we need to give the receive function 
    // some time till the complete string is transmitted and available in the buffer

    delay(30);                                            // give some time for the length byte

    //uint32_t time = micros();
    spi.writeBurst(SX1262_SET_STANDBY, 1);                // go to standby mode, to read buffer and restart rx
    uint8_t irqstat[3];
    spi.readBurst(SX1262_GET_IRQSTATUS, irqstat, 3);      // read the irq status
    //DPRINT("IRQ stat: "); DHEXLN(irqstat, 3);

    //uint8_t rxstat[3];                                  // RX status: not needed as in chip packet handling does not work for cc1101 whitening 
    //spi.readBurst(rxstat, SX1262_GET_RXBUFFERSTATUS, 3);
    //DPRINT("RX stat:  "); DHEXLN(rxstat, 3);

    uint8_t rssistat[2];                                  // get RSSI - Signal power in dBm = –RssiInst/2 (dBm)
    spi.readBurst(SX1262_GET_RSSIINST, rssistat, 2);
    rss = rssistat[1] / 2;
    //DPRINT(F("RXrssi:")); DPRINTLN(rss);

    spi.writeBurst(SX1262_CLR_IRQSTATUS, &irqstat[1], 2); // clear irq status, irqstat[0] is device status
    if (!irqstat[2] & 8) {
      DPRINT(F("rxIRQ:")); DHEXLN(irqstat, 3);
      setRXmode();
      return 0;
    }

    spi.readBuffer(0, buf, 1);                            // read the 1st byte from buffer
    uint8_t len = buf[0];
    whitenBuffer(&len, 1);                                // whiten the len byte
    spi.readBuffer(1, buf, size);                         // read the buffer, max 28 byte for an asksin string
    //DPRINT("raw: "); DHEXLN(buf, size);
    whitenBuffer(buf, size, 1);                           // whiten the buffer
    //DPRINT("wht: "); DHEX(len); DPRINT(' ');  DHEXLN(buf, len + 2);

    // check if we have a valid string by len
    if ((len < 9) && (len > 26)) {
      DPRINT(F("rxLen:")); DHEXLN(len);
      setRXmode();
      return 0;
    }

    // LEN CNT STE FRM  SND       RCV       CNL CNT  CRC
    // 0B  C5  86  40   02 BF 01  00 00 00  01  C4   EF E3      48 A5 AA 06 F2 27 3A 70 01 A2 7E 84 59 

    // check if we have a valid string by crc
    uint16_t clc_crc = calculateCrc(buf, len );           // calculate the checksum
    uint16_t buf_crc = (buf[len] << 8 | buf[len + 1]);    //DHEXLN(buf_crc);
    if (clc_crc != buf_crc) {
      DPRINTLN(F("rxCRC failed ")); //DHEX(len); DPRINT(' ');  DHEXLN(buf, len + 2);
      //setRXmode();
      //return 0;
    }
    //DPRINTLN(micros()  -time); ~0,5 ms

    setRXmode();
    return len;                                           // return number of byte in buffer
  }
  void setRXmode() {
    // switch to RX mode
    enableInterrupt();                                    // clear pending and enable interrupt

    uint8_t rxTO[3] = { 0xff,0xff,0xff };                 // RX Continuous mode, as the packet detection is not working anyhow
    spi.writeBurst(SX1262_SET_RX, rxTO, 3);
  }

};



#endif

}
#endif
