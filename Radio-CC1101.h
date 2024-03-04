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

#ifndef _CC1101_H
#define _CC1101_H

namespace as {

// CC1101 config register                         // Reset  Description
#define CC1101_IOCFG2           0x00              // (0x29) GDO2 Output Pin Configuration
#define CC1101_IOCFG1           0x01              // (0x2E) GDO1 Output Pin Configuration
#define CC1101_IOCFG0           0x02              // (0x3F) GDO0 Output Pin Configuration
#define CC1101_FIFOTHR          0x03              // (0x07) RX FIFO and TX FIFO Thresholds
#define CC1101_SYNC1            0x04              // (0xD3) Sync Word, High Byte
#define CC1101_SYNC0            0x05              // (0x91) Sync Word, Low Byte
#define CC1101_PKTLEN           0x06              // (0xFF) Packet Length
#define CC1101_PKTCTRL1         0x07              // (0x04) Packet Automation Control
#define CC1101_PKTCTRL0         0x08              // (0x45) Packet Automation Control
#define CC1101_ADDR             0x09              // (0x00) Device Address
#define CC1101_CHANNR           0x0A              // (0x00) Channel Number
#define CC1101_FSCTRL1          0x0B              // (0x0F) Frequency Synthesizer Control
#define CC1101_FSCTRL0          0x0C              // (0x00) Frequency Synthesizer Control
#define CC1101_FREQ2            0x0D              // (0x1E) Frequency Control Word, High Byte
#define CC1101_FREQ1            0x0E              // (0xC4) Frequency Control Word, Middle Byte
#define CC1101_FREQ0            0x0F              // (0xEC) Frequency Control Word, Low Byte
#define CC1101_MDMCFG4          0x10              // (0x8C) Modem Configuration
#define CC1101_MDMCFG3          0x11              // (0x22) Modem Configuration
#define CC1101_MDMCFG2          0x12              // (0x02) Modem Configuration
#define CC1101_MDMCFG1          0x13              // (0x22) Modem Configuration
#define CC1101_MDMCFG0          0x14              // (0xF8) Modem Configuration
#define CC1101_DEVIATN          0x15              // (0x47) Modem Deviation Setting
#define CC1101_MCSM2            0x16              // (0x07) Main Radio Control State Machine Configuration
#define CC1101_MCSM1            0x17              // (0x30) Main Radio Control State Machine Configuration
#define CC1101_MCSM0            0x18              // (0x04) Main Radio Control State Machine Configuration
#define CC1101_FOCCFG           0x19              // (0x36) Frequency Offset Compensation Configuration
#define CC1101_BSCFG            0x1A              // (0x6C) Bit Synchronization Configuration
#define CC1101_AGCCTRL2         0x1B              // (0x03) AGC Control
#define CC1101_AGCCTRL1         0x1C              // (0x40) AGC Control
#define CC1101_AGCCTRL0         0x1D              // (0x91) AGC Control
#define CC1101_WOREVT1          0x1E              // (0x87) High Byte Event0 Timeout
#define CC1101_WOREVT0          0x1F              // (0x6B) Low Byte Event0 Timeout
#define CC1101_WORCTRL          0x20              // (0xF8) Wake On Radio Control
#define CC1101_FREND1           0x21              // (0x56) Front End RX Configuration
#define CC1101_FREND0           0x22              // (0x10) Front End RX Configuration
#define CC1101_FSCAL3           0x23              // (0xA9) Frequency Synthesizer Calibration
#define CC1101_FSCAL2           0x24              // (0x0A) Frequency Synthesizer Calibration
#define CC1101_FSCAL1           0x25              // (0x20) Frequency Synthesizer Calibration
#define CC1101_FSCAL0           0x26              // (0x0D) Frequency Synthesizer Calibration
#define CC1101_RCCTRL1          0x27              // (0x41) RC Oscillator Configuration
#define CC1101_RCCTRL2          0x28              // (0x00) RC Oscillator Configuration
#define CC1101_FSTEST           0x29              // (0x59) Frequency Synthesizer Calibration Control
#define CC1101_PTEST            0x2A              // (0x7F) Production Test
#define CC1101_AGCTEST          0x2B              // (0x3F) AGC Test
#define CC1101_TEST2            0x2C              // (0x88) Various Test Settings
#define CC1101_TEST1            0x2D              // (0x31) Various Test Settings
#define CC1101_TEST0            0x2E              // (0x0B) Various Test Settings

#define CC1101_PARTNUM          0x30              // (0x00) Readonly: Chip ID
#define CC1101_VERSION          0x31              // (0x04) Readonly: Chip ID
#define CC1101_FREQEST          0x32              // (0x00) Readonly: Frequency Offset Estimate from Demodulator
#define CC1101_LQI              0x33              // (0x00) Readonly: Demodulator Estimate for Link Quality
#define CC1101_RSSI             0x34              // (0x00) Readonly: Received Signal Strength Indication
#define CC1101_MARCSTATE        0x35              // (0x00) Readonly: Main Radio Control State Machine State
#define CC1101_WORTIME1         0x36              // (0x00) Readonly: High Byte of WOR Time
#define CC1101_WORTIME0         0x37              // (0x00) Readonly: Low Byte of WOR Time
#define CC1101_PKTSTATUS        0x38              // (0x00) Readonly: Current GDOx Status and Packet Status
#define CC1101_VCO_VC_DAC       0x39              // (0x00) Readonly: Current Setting from PLL Calibration Module
#define CC1101_TXBYTES          0x3A              // (0x00) Readonly: Underflow and Number of Bytes
#define CC1101_RXBYTES          0x3B              // (0x00) Readonly: Overflow and Number of Bytes
#define CC1101_RCCTRL1_STATUS   0x3C              // (0x00) Readonly: Last RC Oscillator Calibration Result
#define CC1101_RCCTRL0_STATUS   0x3D              // (0x00) Readonly: Last RC Oscillator Calibration Result

#define CC1101_PATABLE          0x3E              // PATABLE address
#define CC1101_TXFIFO           0x3F              // TX FIFO address
#define CC1101_RXFIFO           0x3F              // RX FIFO address

#define CC1101_PA_TABLE0        0x40              // (0x00) PA table, entry 0
#define CC1101_PA_TABLE1        0x41              // (0x00) PA table, entry 1
#define CC1101_PA_TABLE2        0x42              // (0x00) PA table, entry 2
#define CC1101_PA_TABLE3        0x43              // (0x00) PA table, entry 3
#define CC1101_PA_TABLE4        0x44              // (0x00) PA table, entry 4
#define CC1101_PA_TABLE5        0x45              // (0x00) PA table, entry 5
#define CC1101_PA_TABLE6        0x46              // (0x00) PA table, entry 6
#define CC1101_PA_TABLE7        0x47              // (0x00) PA table, entry 7

// some register definitions for TRX868 communication
#define READ_SINGLE              0x80             // type of transfers
#define READ_BURST               0xC0
#define WRITE_BURST              0x40

#define CC1101_CONFIG            0x80             // type of register
#define CC1101_STATUS            0xC0

#define CC1101_SRES              0x30             // reset CC1101 chip
#define CC1101_SFSTXON           0x31             // enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). if in RX (with CCA): Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
#define CC1101_SXOFF             0x32             // turn off crystal oscillator
#define CC1101_SCAL              0x33             // calibrate frequency synthesizer and turn it off. SCAL can be strobed from IDLE mode without setting manual calibration mode (MCSM0.FS_AUTOCAL=0)
#define CC1101_SRX               0x34             // enable RX. perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1
#define CC1101_STX               0x35             // in IDLE state: enable TX. perform calibration first if MCSM0.FS_AUTOCAL=1. if in RX state and CCA is enabled: only go to TX if channel is clear
#define CC1101_SIDLE             0x36             // exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable
#define CC1101_SWOR              0x38             // start automatic RX polling sequence (Wake-on-Radio) as described in Section 19.5 if WORCTRL.RC_PD=0
#define CC1101_SPWD              0x39             // enter power down mode when CSn goes high
#define CC1101_SFRX              0x3A             // flush the RX FIFO buffer. only issue SFRX in IDLE or RXFIFO_OVERFLOW states
#define CC1101_SFTX              0x3B             // flush the TX FIFO buffer. only issue SFTX in IDLE or TXFIFO_UNDERFLOW states
#define CC1101_SWORRST           0x3C             // reset real time clock to Event1 value
#define CC1101_SNOP              0x3D             // no operation. may be used to get access to the chip status byte

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

#define PA_LowPower              0x03             // PATABLE values
#define PA_Normal                0x50             // PATABLE values
#define PA_MaxPower              0xC0



template <class SPIType, uint8_t PWRPIN>
class CC1101  {
protected:
  SPIType spi;
  uint8_t rss;   // signal strength
#ifdef USE_OTA_BOOTLOADER_FREQUENCY
  uint8_t f1,f0; // storage for frequency values from boot loader
#endif

public:
  CC1101() : rss(0)
#ifdef USE_OTA_BOOTLOADER_FREQUENCY
  , f1(0x65), f0(0x6A) // set to defaults
#endif
  {}

  uint8_t interruptMode() {
    return 0; // FALLING
  };

  void setIdle () {
    //DPRINTLN("CC enter powerdown");
    uint8_t cnt = 0xff;
    while(cnt-- && (spi.strobe(CC1101_SIDLE) & 0x70) != 0) {
      _delay_us(10);
    }
    spi.strobe(CC1101_SFRX);

#ifdef USE_WOR
    // init
    spi.writeReg(CC1101_PKTCTRL1, 0x4C);    // preamble quality estimator threshold=2
    spi.writeReg(CC1101_MCSM2, 0x1c);       // RX_TIME_RSSI=1, RX_TIME_QUAL=1, RX_TIME=4
    //start
    spi.strobe(CC1101_SWORRST);
    spi.strobe(CC1101_SWOR);
#else
    spi.strobe(CC1101_SPWD);                // enter power down state
#endif

    if (PWRPIN < 0xff) {
      spi.shutdown();
      digitalWrite(PWRPIN, HIGH);
    }
  }

  void wakeup (bool flush) {
    if (PWRPIN < 0xff) {
      init();
    }

    spi.ping();
    if( flush==true ) {
      flushrx();
    }
#ifdef USE_WOR
    // ToDo: is this the right position?
    spi.writeReg(CC1101_PKTCTRL1, 0x0C);    // preamble quality estimator threshold=0
    spi.writeReg(CC1101_MCSM2, 0x07);       // RX_TIME_RSSI=0, RX_TIME_QUAL=0, RX_TIME=7
#endif
    spi.strobe(CC1101_SRX);
  }

  uint8_t reset() {

    // Strobe CSn low / high
    spi.select();

    // Automatic POR
    // If the chip has had sufficient time for the crystal oscillator to stabilize after the power-on-reset the SO pin
    // will go low immediately after taking CSn low. If CSn is taken low before reset is completed the
    // SO pin will first go high, indicating that the crystal oscillator is not stabilized, before going low
    spi.waitMiso();
    spi.deselect();

    // Hold CSn high for at least 40μs relative to pulling CSn low
    _delay_us(50);

    // Pull CSn low and wait for SO to go low (CHIP_RDYn).
    spi.select();
    spi.waitMiso();

    // Issue the SRES strobe on the SI line
    uint8_t ret = spi.send(CC1101_SRES);

    // When SO goes low again, reset is complete and the chip is in the IDLE state.
    spi.waitMiso();
    spi.deselect();

    return ret;
  }


  bool init () {
    if (PWRPIN < 0xff) {
      pinMode(PWRPIN, OUTPUT);
      digitalWrite(PWRPIN, LOW);
      _delay_ms(5);
    }
    spi.init();                 // init the hardware to get access to the RF modul

#ifdef USE_OTA_BOOTLOADER_FREQUENCY
    // before we reset the CC1101 - store frequency settings from bootloader
    f1 = spi.readReg(CC1101_FREQ1, CC1101_CONFIG);
    f0 = spi.readReg(CC1101_FREQ0, CC1101_CONFIG);
    DPRINT(F("Boot Loader Freq: 0x21"));DHEX(f1);DHEXLN(f0);
#endif
    reset();

#ifdef SIMPLE_CC1101_INIT
    static const uint8_t initVal[] PROGMEM = {
    /* register          value       reset   explanation of delta to reset value   */
    /* CC1101_IOCFG2 */  0x2E,   //  0x29    high impedance tri state (pin not used)
    /* CC1101_IOCFG1 */  0x2E,   //  0x2E    high impedance tri state (pin not used)
    /* CC1101_IOCFG0,*/  0x06,   //  0x3F    Asserts when sync word has been sent / received, and de-asserts at the end of the packet. In RX, the pin will also deassert when a packet is discarded due to address or maximum length filtering or when the radio enters RXFIFO_OVERFLOW state. In TX the pin will de-assert if the TX FIFO underflows.
    /* CC1101_FIFOTHR,*/ 0x0D,   //  0x07    TX FIFO = 9, RX FIFO = 56 byte
    /* CC1101_SYNC1, */  0xE9,   //  0xD3    Sync word MSB
    /* CC1101_SYNC0, */  0xCA,   //  0x91    Sync word LSB
    /* CC1101_PKTLEN,*/  0xFF,   //  0xFF
    /* CC1101_PKTCTRL1,*/0x0C,   //  0x04    CRC auto flush = 1, append status = 1,
    /* CC1101_PKTCTRL0,*/0x45,   //  0x45
    /* CC1101_ADDR, */   0x00,   //  0x00
    /* CC1101_CHANNR,*/  0x00,   //  0x00
    /* CC1101_FSCTRL1,*/ 0x06,   //  0x0F    frequency synthesizer control
    /* CC1101_FSCTRL0,*/ 0x00,   //  0x00
    /* CC1101_FREQ2, */  0x21,   //  0x1E
    /* CC1101_FREQ1, */  0x65,   //  0xC4
    /* CC1101_FREQ0, */  0x6A,   //  0xEC
    /* CC1101_MDMCFG4,*/ 0xC8,   //  0x8C    channel bandwidth
    /* CC1101_MDMCFG3,*/ 0x93,   //  0x22    symbol data rate
    /* CC1101_MDMCFG2,*/ 0x03,   //  0x02    30 of 32 bits of sync word need to match
    /* CC1101_MDMCFG1,*/ 0x22,   //  0x22
    /* CC1101_MDMCFG0,*/ 0xF8,   //  0xF8
    /* CC1101_DEVIATN,*/ 0x34,   //  0x47    deviation = 19.042969 kHz
    /* CC1101_MCSM2, */  0x07,   //  0x07
#ifdef USE_CCA
    /* CC1101_MCSM1,*/   0x33,   //  0x30    CCA, RX after TX
#else
    /* CC1101_MCSM1,*/   0x03,   //  0x30    always clear channel indication, RX after TX
#endif
    /* CC1101_MCSM0,*/   0x18,   //  0x04    auto cal when going from IDLE to RX/TX, XOSC stable count = 64
    /* CC1101_FOCCFG,*/  0x16,   //  0x36    don't freeze freq offset compensation
    /* CC1101_BSCFG,*/   0x6C,   //  0x6C
    /* CC1101_AGCCTRL2,*/0x43,   //  0x03    forbid highest gain setting for DVGA
    /* CC1101_AGCCTRL1,*/0x40,   //  0x40
    /* CC1101_AGCCTRL0,*/0x91,   //  0x91
    /* CC1101_WOREVT1,*/ 0x2f,   //  0x87    see next line
    /* CC1101_WOREVT0,*/ 0x65,   //  0x6B    t_Event0 = 350ms
    /* CC1101_WORCTRL,*/ 0x78,   //  0xF8    RC_PD=0
    /* CC1101_FREND1,*/  0x56,   //  0x56
    /* CC1101_FREND0,*/  0x10,   //  0x10
    /* CC1101_FSCAL3,*/  0xE9,   //  0xA9    charge pump calib stage
    /* CC1101_FSCAL2,*/  0x2A,   //  0x0A    high VCO
    /* CC1101_FSCAL1,*/  0x1F,   //  0x20    freq synthesizer calib result
    /* CC1101_FSCAL0,*/  0x11,   //  0x0D    freq synthesizer calib control
    //CC1101_RCCTRL1,   0x41,   //  0x41
    //CC1101_RCCTRL0,   0x00,   //  0x00
    //CC1101_FSTEST,    0x59,   //  0x59
    //CC1101_PTEST,     0x7f,   //  0x7f
    //CC1101_AGCTEST,   0x3f,   //  0x3f
    //CC1101_TEST2,     0x88,   //  0x88
    //CC1101_TEST1,     0x31,   //  0x31
    //CC1101_TEST0,     0x0b,   //  0x0b
    //  CC1101_PATABLE,   0x03,   //    NA
    };
    bool initOK = true;
    for (uint8_t i=0; i<sizeof(initVal); ++i) { // write init value to TRX868
      bool initres = initReg(i, pgm_read_byte(&initVal[i]));
      //if any initReg fails, initOK has to be false
      if (initres == false) initOK = false;
    }
#else
    // define init settings for CC1101
    static const uint8_t initVal[] PROGMEM = {
    /*register          value       reset   explanation of delta to reset value   */
      CC1101_IOCFG2,    0x2E,   //  0x29    high impedance tri state (pin not used)
    //CC1101_IOCFG2,    0x24,   //  0x29    Debug: WOR_EVT0

    //CC1101_IOCFG1,    0x2E,   //  0x2E    high impedance tri state (pin not used)
    //CC1101_IOCFG1,    0x25,   //  0x2E    Debug: WOR_EVT1
    //CC1101_IOCFG1,    0x0E,   //  0x2E    Debug: Carrier sense
    //CC1101_IOCFG1,    0x0F,   //  0x2E    Debug: CRC_OK

      CC1101_IOCFG0,    0x06,   //  0x3F    Asserts when sync word has been sent / received, and de-asserts at the end of the packet. In RX, the pin will also deassert when a packet is discarded due to address or maximum length filtering or when the radio enters RXFIFO_OVERFLOW state. In TX the pin will de-assert if the TX FIFO underflows.
      CC1101_FIFOTHR,   0x0D,   //  0x07    TX FIFO = 9, RX FIFO = 56 byte
      CC1101_SYNC1,     0xE9,   //  0xD3    Sync word MSB
      CC1101_SYNC0,     0xCA,   //  0x91    Sync word LSB
    //CC1101_PKTLEN,    0xFF,   //  0xFF
      CC1101_PKTCTRL1,  0x0C,   //  0x04    CRC auto flush = 1, append status = 1,
    //CC1101_PKTCTRL0,  0x45,   //  0x45
    //CC1101_ADDR,      0x00,   //  0x00
    //CC1101_CHANNR,    0x00,   //  0x00
      CC1101_FSCTRL1,   0x06,   //  0x0F    frequency synthesizer control
    //CC1101_FSCTRL0,   0x00,   //  0x00

#ifndef USE_OTA_BOOTLOADER_FREQUENCY
      // 868.299866 MHz - if other values are found in EEPROM, these are overwritten later
      CC1101_FREQ2,     0x21,   //  0x1E
      CC1101_FREQ1,     0x65,   //  0xC4
      CC1101_FREQ0,     0x6A,   //  0xEC
#endif

      CC1101_MDMCFG4,   0xC8,   //  0x8C    channel bandwidth
      CC1101_MDMCFG3,   0x93,   //  0x22    symbol data rate
      CC1101_MDMCFG2,   0x03,   //  0x02    30 of 32 bits of sync word need to match
    //CC1101_MDMCFG1,   0x22,   //  0x22
    //CC1101_MDMCFG0,   0xF8,   //  0xF8
      CC1101_DEVIATN,   0x34,   //  0x47    deviation = 19.042969 kHz
    //CC1101_MCSM2,     0x07,   //  0x07
#ifdef USE_CCA
      CC1101_MCSM1,     0x33,   //  0x30    CCA, RX after TX
#else
      CC1101_MCSM1,     0x03,   //  0x30    always clear channel indication, RX after TX
#endif
      CC1101_MCSM0,     0x18,   //  0x04    auto cal when going from IDLE to RX/TX, XOSC stable count = 64
      CC1101_FOCCFG,    0x16,   //  0x36    don't freeze freq offset compensation
    //CC1101_BSCFG,     0x6C,   //  0x6C
      CC1101_AGCCTRL2,  0x43,   //  0x03    forbid highest gain setting for DVGA
    //CC1101_AGCCTRL1,  0x40,   //  0x40
    //CC1101_AGCCTRL0,  0x91,   //  0x91
      CC1101_WOREVT1,   0x2f,   //  0x87    see next line
      CC1101_WOREVT0,   0x65,   //  0x6B    t_Event0 = 350ms
      CC1101_WORCTRL,   0x78,   //  0xF8    RC_PD=0
    //CC1101_FREND1,    0x56,   //  0x56
    //CC1101_FREND0,    0x10,   //  0x10
      CC1101_FSCAL3,    0xE9,   //  0xA9    charge pump calib stage
      CC1101_FSCAL2,    0x2A,   //  0x0A    high VCO
      CC1101_FSCAL1,    0x1F,   //  0x20    freq synthesizer calib result
      CC1101_FSCAL0,    0x11,   //  0x0D    freq synthesizer calib control
    //CC1101_RCCTRL1,   0x41,   //  0x41
    //CC1101_RCCTRL0,   0x00,   //  0x00
    //CC1101_FSTEST,    0x59,   //  0x59
    //CC1101_PTEST,     0x7f,   //  0x7f
    //CC1101_AGCTEST,   0x3f,   //  0x3f
    //CC1101_TEST2,     0x88,   //  0x88
    //CC1101_TEST1,     0x31,   //  0x31
    //CC1101_TEST0,     0x0b,   //  0x0b
      CC1101_PATABLE,   0x03,   //    NA
    };

    bool initOK = true;
    for (uint8_t i=0; i<sizeof(initVal); i+=2) { // write init value to TRX868
      bool initres = initReg(pgm_read_byte(&initVal[i]), pgm_read_byte(&initVal[i+1]));
      //if any initReg fails, initOK has to be false
      if (initres == false) initOK = false;
    }
#endif

#ifdef USE_OTA_BOOTLOADER_FREQUENCY
    // set frequency from bootloader again
    initReg(CC1101_FREQ2, 0x21);
    initReg(CC1101_FREQ1, f1);
    initReg(CC1101_FREQ0, f0);
#endif

    // Settings that ELV sets
    DPRINT(F("CC Version: ")); DHEXLN(spi.readReg(CC1101_VERSION, CC1101_STATUS));

    spi.strobe(CC1101_SCAL);                                // calibrate frequency synthesizer and turn it off

    _delay_ms(23);

    initReg(CC1101_PATABLE, PA_MaxPower);                   // configure PATABLE

    DPRINTLN(F(" - ready"));
    return initOK;
  }

#ifdef SIMPLE_CC1101_INIT
  bool initReg (uint8_t regAddr, uint8_t val) {
    spi.writeReg(regAddr, val);
    return true;
  }
#else
  bool initReg (uint8_t regAddr, uint8_t val, uint8_t retries=3) {
    spi.writeReg(regAddr, val);
    uint8_t val_read = spi.readReg(regAddr, CC1101_CONFIG);
    bool initResult = true;
    if( val_read != val ) {
      if( retries > 0 ) {
        initResult = initReg(regAddr, val, --retries);
        _delay_ms(1);
      }
      else {
        DPRINT(F("Error at ")); DHEX(regAddr);
        DPRINT(F(" expected: ")); DHEX(val); DPRINT(F(" read: ")); DHEXLN(val_read);
        return false;
      }
    }
    return initResult;
  }
#endif

  void tuneFreq(uint8_t freq2, uint8_t freq1, uint8_t freq0) {
    initReg(CC1101_FREQ2, freq2);
    initReg(CC1101_FREQ1, freq1);
    initReg(CC1101_FREQ0, freq0);
  }

  uint8_t rssi () const {
    return rss;
  }

  void flushrx () {
    spi.strobe(CC1101_SIDLE);
    spi.strobe(CC1101_SNOP);
    spi.strobe(CC1101_SFRX);
  }

  bool detectBurst () {
    uint8_t state = spi.readReg(CC1101_PKTSTATUS, CC1101_STATUS);
    // DHEXLN(state);
    return (state & 0x01<<6) == (0x01<<6);
  }

  void pollRSSI() {
    calculateRSSI(spi.readReg(CC1101_RSSI, CC1101_STATUS));         // read RSSI from STATUS register
  }

protected:

  void calculateRSSI(uint8_t rsshex) {
    rss = -1 * ((((int16_t)rsshex-((int16_t)rsshex >= 128 ? 256 : 0))/2)-74);
  }

  uint8_t sndData(uint8_t *buf, uint8_t size, uint8_t burst) {
    // Going from RX to TX does not work if there was a reception less than 0.5
    // sec ago. Due to CCA? Using IDLE helps to shorten this period(?)
    spi.strobe(CC1101_SIDLE);  // go to idle mode
    spi.strobe(CC1101_SFTX );  // flush TX buffer
    
    // switch to TX and wait till status has changed, will take approx 1ms
    spi.strobe(CC1101_STX);
    uint8_t i = 200;
    while ((spi.readReg(CC1101_MARCSTATE, CC1101_STATUS) != MARCSTATE_TX) && (--i != 0)) {
      _delay_us(100);
    }

    // if switch to TX mode timed out - reset fifo and back to RX mode
    if (i == 0) {
      spi.strobe(CC1101_SIDLE);                                             
      spi.strobe(CC1101_SFTX);
      spi.strobe(CC1101_SNOP);
      spi.strobe(CC1101_SRX);
      waitRX();
      return false;
    }

    // send burst signal for sleeping devices if necessary
    _delay_ms(10);
    if (burst) {               // BURST-bit set?
      _delay_ms(350);          // according to ELV, devices get activated every 300ms, so send burst for 360ms
    }

    // write bytes into cc1101 TX queue
    spi.writeReg(CC1101_TXFIFO, size);
    spi.writeBurst(CC1101_TXFIFO | WRITE_BURST, buf, size);             // write in TX FIFO

    // after sending out all bytes the chip should go automatically in RX mode, we wait for RX mode to ensure a proper handling
    return waitRX();
  }

  uint8_t waitRX() {
    uint8_t i = 200;
    while ((spi.readReg(CC1101_MARCSTATE, CC1101_STATUS) != MARCSTATE_RX) && (--i != 0)) {
      _delay_us(200);
    }
    return i ? true : false;
  }


  uint8_t rcvData(uint8_t *buf, uint8_t size) {
    //DPRINTLN(" rcvData");

    uint8_t rxBytes = 0;
    uint8_t fifoBytes = spi.readReg(CC1101_RXBYTES, CC1101_STATUS);     // how many bytes are in the buffer
    //DPRINT("  RX FIFO: ");DHEXLN(fifoBytes);
    
    // overflow detected - flush the FIFO
    if( fifoBytes > 0 && (fifoBytes & 0x80) != 0x80 ) {
      uint8_t packetBytes = spi.readReg(CC1101_RXFIFO, CC1101_CONFIG);  // read packet length
      //DPRINT("  Start Packet: ");DHEXLN(packetBytes);
      
      // check that packet fits into the buffer
      if (packetBytes <= size) {
        spi.readBurst(buf, CC1101_RXFIFO | READ_BURST, packetBytes);    // read data packet
        calculateRSSI(spi.readReg(CC1101_RXFIFO, CC1101_CONFIG));       // read RSSI from RXFIFO
        uint8_t val = spi.readReg(CC1101_RXFIFO, CC1101_CONFIG);        // read LQI and CRC_OK
        // lqi = val & 0x7F;
        if( (val & 0x80) == 0x80 ) {                                    // check crc_ok
          // DPRINTLN("CRC OK");
          rxBytes = packetBytes;
        }
        else {
          DPRINTLN(F("CRC Failed"));
        }
      }
      else {
        DPRINT(F("Packet too big: ")); DDECLN(packetBytes);
      }
    }
    //DPRINT(F("-> ")); DHEXLN(buf,rxBytes);

    // cc1101 is configured to enter idle mode after receiving a message; go back in receive mode
    flushrx();
    spi.strobe(CC1101_SRX);
    //DHEXLN(spi.readReg(CC1101_MARCSTATE, CC1101_STATUS));
    return rxBytes;                                                     // return number of byte in buffer
  }

  void recalibrate() {
    uint8_t fscal1val = spi.readReg(CC1101_FSCAL1, CC1101_CONFIG);
    if (fscal1val == 0x3F) {
      DPRINT(F("\nforce calibration - ")); DPRINTLN(millis()); DPRINT('\n');
      spi.strobe(CC1101_SIDLE);
      spi.strobe(CC1101_SCAL);
      spi.strobe(CC1101_SRX);
    }
  }

};

}

#endif
