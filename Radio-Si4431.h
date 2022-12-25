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
//- AskSin Si4431 functions -----------------------------------------------------------------------------------------------
//- based on analysis of HM-Sec-SCo
//- -----------------------------------------------------------------------------------------------------------------------

/*
  TODO
  - implement WoR (if possible with Si4431)

*/

#ifndef _SI4431_H
#define _SI4431_H

namespace as
{

// Si4431 registers
#define SI4431_REG_DEVICE_TYPE 0x00
#define SI4431_REG_DEVICE_VERSION 0x01
#define SI4431_REG_DEVICE_STATUS 0x02
#define SI4431_REG_INTERRUPT_STATUS_1 0x03
#define SI4431_REG_INTERRUPT_STATUS_2 0x04
#define SI4431_REG_INTERRUPT_ENABLE_1 0x05
#define SI4431_REG_INTERRUPT_ENABLE_2 0x06
#define SI4431_REG_OP_FUNC_CONTROL_1 0x07
#define SI4431_REG_OP_FUNC_CONTROL_2 0x08
#define SI4431_REG_XOSC_LOAD_CAPACITANCE 0x09
#define SI4431_REG_MCU_OUTPUT_CLOCK 0x0A
#define SI4431_REG_GPIO0_CONFIG 0x0B
#define SI4431_REG_GPIO1_CONFIG 0x0C
#define SI4431_REG_GPIO2_CONFIG 0x0D
#define SI4431_REG_IO_PORT_CONFIG 0x0E
#define SI4431_REG_ADC_CONFIG 0x0F
#define SI4431_REG_ADC_SENSOR_AMP_OFFSET 0x10
#define SI4431_REG_ADC_VALUE 0x11
#define SI4431_REG_TEMP_SENSOR_CONTROL 0x12
#define SI4431_REG_TEMP_VALUE_OFFSET 0x13
#define SI4431_REG_WAKEUP_TIMER_PERIOD_1 0x14
#define SI4431_REG_WAKEUP_TIMER_PERIOD_2 0x15
#define SI4431_REG_WAKEUP_TIMER_PERIOD_3 0x16
#define SI4431_REG_WAKEUP_TIMER_VALUE_1 0x17
#define SI4431_REG_WAKEUP_TIMER_VALUE_2 0x18
#define SI4431_REG_LOW_DC_MODE_DURATION 0x19
#define SI4431_REG_LOW_BATT_DET_THRESHOLD 0x1A
#define SI4431_REG_BATT_VOLTAGE_LEVEL 0x1B
#define SI4431_REG_IF_FILTER_BANDWIDTH 0x1C
#define SI4431_REG_AFC_LOOP_GEARSHIFT_OVERRIDE 0x1D
#define SI4431_REG_AFC_TIMING_CONTROL 0x1E
#define SI4431_REG_CLOCK_REC_GEARSHIFT_OVERRIDE 0x1F
#define SI4431_REG_CLOCK_REC_OVERSAMP_RATIO 0x20
#define SI4431_REG_CLOCK_REC_OFFSET_2 0x21
#define SI4431_REG_CLOCK_REC_OFFSET_1 0x22
#define SI4431_REG_CLOCK_REC_OFFSET_0 0x23
#define SI4431_REG_CLOCK_REC_TIMING_LOOP_GAIN_1 0x24
#define SI4431_REG_CLOCK_REC_TIMING_LOOP_GAIN_0 0x25
#define SI4431_REG_RSSI 0x26
#define SI4431_REG_RSSI_CLEAR_CHANNEL_THRESHOLD 0x27
#define SI4431_REG_ANTENNA_DIVERSITY_1 0x28
#define SI4431_REG_ANTENNA_DIVERSITY_2 0x29
#define SI4431_REG_AFC_LIMITER 0x2A
#define SI4431_REG_AFC_CORRECTION 0x2B
#define SI4431_REG_OOK_COUNTER_1 0x2C
#define SI4431_REG_OOK_COUNTER_2 0x2D
#define SI4431_REG_SLICER_PEAK_HOLD 0x2E
#define SI4431_REG_DATA_ACCESS_CONTROL 0x30
#define SI4431_REG_EZMAC_STATUS 0x31
#define SI4431_REG_HEADER_CONTROL_1 0x32
#define SI4431_REG_HEADER_CONTROL_2 0x33
#define SI4431_REG_PREAMBLE_LENGTH 0x34
#define SI4431_REG_PREAMBLE_DET_CONTROL 0x35
#define SI4431_REG_SYNC_WORD_3 0x36
#define SI4431_REG_SYNC_WORD_2 0x37
#define SI4431_REG_SYNC_WORD_1 0x38
#define SI4431_REG_SYNC_WORD_0 0x39
#define SI4431_REG_TRANSMIT_HEADER_3 0x3A
#define SI4431_REG_TRANSMIT_HEADER_2 0x3B
#define SI4431_REG_TRANSMIT_HEADER_1 0x3C
#define SI4431_REG_TRANSMIT_HEADER_0 0x3D
#define SI4431_REG_TRANSMIT_PACKET_LENGTH 0x3E
#define SI4431_REG_CHECK_HEADER_3 0x3F
#define SI4431_REG_CHECK_HEADER_2 0x40
#define SI4431_REG_CHECK_HEADER_1 0x41
#define SI4431_REG_CHECK_HEADER_0 0x42
#define SI4431_REG_HEADER_ENABLE_3 0x43
#define SI4431_REG_HEADER_ENABLE_2 0x44
#define SI4431_REG_HEADER_ENABLE_1 0x45
#define SI4431_REG_HEADER_ENABLE_0 0x46
#define SI4431_REG_RECEIVED_HEADER_3 0x47
#define SI4431_REG_RECEIVED_HEADER_2 0x48
#define SI4431_REG_RECEIVED_HEADER_1 0x49
#define SI4431_REG_RECEIVED_HEADER_0 0x4A
#define SI4431_REG_RECEIVED_PACKET_LENGTH 0x4B
#define SI4431_REG_ADC8_CONTROL 0x4F
#define SI4431_REG_CHANNEL_FILTER_COEFF 0x60
#define SI4431_REG_XOSC_CONTROL_TEST 0x62
#define SI4431_REG_AGC_OVERRIDE_1 0x69
#define SI4431_REG_TX_POWER 0x6D
#define SI4431_REG_TX_DATA_RATE_1 0x6E
#define SI4431_REG_TX_DATA_RATE_0 0x6F
#define SI4431_REG_MODULATION_MODE_CONTROL_1 0x70
#define SI4431_REG_MODULATION_MODE_CONTROL_2 0x71
#define SI4431_REG_FREQUENCY_DEVIATION 0x72
#define SI4431_REG_FREQUENCY_OFFSET_1 0x73
#define SI4431_REG_FREQUENCY_OFFSET_2 0x74
#define SI4431_REG_FREQUENCY_BAND_SELECT 0x75
#define SI4431_REG_NOM_CARRIER_FREQUENCY_1 0x76
#define SI4431_REG_NOM_CARRIER_FREQUENCY_0 0x77
#define SI4431_REG_FREQUENCY_HOPPING_CHANNEL_SEL 0x79
#define SI4431_REG_FREQUENCY_HOPPING_STEP_SIZE 0x7A
#define SI4431_REG_TX_FIFO_CONTROL_1 0x7C
#define SI4431_REG_TX_FIFO_CONTROL_2 0x7D
#define SI4431_REG_RX_FIFO_CONTROL 0x7E
#define SI4431_REG_FIFO_ACCESS 0x7F

// Si4431 register bitfields

// SI4431_INTERRUPT_STATUS_1
// SI4431_INTERRUPT_ENABLE_1
#define SI4431_IRQ1_FIFO_LEVEL_ERROR 0b10000000      //  7     7     Tx/Rx FIFO overflow or underflow
#define SI4431_IRQ1_TX_FIFO_ALMOST_FULL 0b01000000   //  6     6     Tx FIFO almost full
#define SI4431_IRQ1_TX_FIFO_ALMOST_EMPTY 0b00100000  //  5     5     Tx FIFO almost empty
#define SI4431_IRQ1_RX_FIFO_ALMOST_FULL 0b00010000   //  4     4     Rx FIFO almost full
#define SI4431_IRQ1_EXTERNAL 0b00001000              //  3     3     external interrupt occurred on GPIOx
#define SI4431_IRQ1_PACKET_SENT 0b00000100           //  2     2     packet transmission done
#define SI4431_IRQ1_VALID_PACKET_RECEIVED 0b00000010 //  1     1     valid packet has been received
#define SI4431_IRQ1_CRC_ERROR 0b00000001             //  0     0     CRC failed

// SI4431_INTERRUPT_STATUS_2
// SI4431_INTERRUPT_ENABLE_2
#define SI4431_IRQ2_SYNC_WORD_DETECTED 0b10000000        //  7     7     sync word has been detected
#define SI4431_IRQ2_VALID_PREAMBLE_DETECTED 0b01000000   //  6     6     valid preamble has been detected
#define SI4431_IRQ2_INVALID_PREAMBLE_DETECTED 0b00100000 //  5     5     invalid preamble has been detected
#define SI4431_IRQ2_RSSI 0b00010000                      //  4     4     RSSI exceeded programmed threshold
#define SI4431_IRQ2_WAKEUP_TIMER 0b00001000              //  3     3     wake-up timer expired
#define SI4431_IRQ2_LOW_BATTERY 0b00000100               //  2     2     low battery detected
#define SI4431_IRQ2_CHIP_READY 0b00000010                //  1     1     chip ready event detected
#define SI4431_IRQ2_POWER_ON_RESET 0b00000001            //  0     0     power-on-reset detected

// SI4431_REG_OP_FUNC_CONTROL_1
#define SI4431_OFC1_SWRES 0b10000000   //  7     7     SOFTWARE_RESET            reset all registers to default values
#define SI4431_OFC1_ENLBD 0b01000000   //  6     6     ENABLE_LOW_BATTERY_DETECT enable low battery detection
#define SI4431_OFC1_ENWT 0b00100000    //  5     5     ENABLE_WAKEUP_TIMER       enable wakeup timer
#define SI4431_OFC1_X32KSEL 0b00000000 //  4     4     32_KHZ_RC                 32.768 kHz source: RC oscillator (default)
#define SI4431_OFC1_TXON 0b00001000    //  3     3     TX_ON                     Tx on in manual transmit mode
#define SI4431_OFC1_RXON 0b00000100    //  2     2     RX_ON                     Rx on in manual receive mode
#define SI4431_OFC1_PLLON 0b00000010   //  1     1     PLL_ON                    PLL on (tune mode)
#define SI4431_OFC1_XTALON 0b00000001  //  0     0     XTAL_ON                   on (ready mode)
#define SI4431_OFC1_NONE 0             //              no bit set

// SI4431_REG_OP_FUNC_CONTROL_2
#define SI4431_OFC2_RXMPK 0b00010000   //  4     4     RX_MULTIPACKET_ON
#define SI4431_OFC2_AUTOTX 0b00001000  //  3     3     AUTO_TX_ON
#define SI4431_OFC2_ENLDM 0b00000100   //  2     2     LOW_DUTY_CYCLE_ON
#define SI4431_OFC2_FFCLRRX 0b00000010 //  1     1     RX_FIFO_RESET
#define SI4431_OFC2_FFCLRTX 0b00000001 //  0     0     TX_FIFO_RESET
#define SI4431_OFC2_NONE 0             //              no bit set

// SPI protocol definitions
#define WRITE_REG 0x80 // write access

  template <class SPIType, uint8_t PWRPIN, uint8_t IRQPIN>
  class Si4431
  {
  protected:
    SPIType spi;
    uint8_t rss; // signal strength

  public:
    Si4431() : rss(0)
    {
    }

    inline void writeReg(uint8_t regAddr, uint8_t val)
    {
      spi.writeReg(regAddr | WRITE_REG, val);
    }
    inline void writeBurst(uint8_t regAddr, const uint8_t *buf, uint8_t len)
    {
      spi.writeBurst(regAddr | WRITE_REG, buf, len);
    }
    inline uint8_t readReg(uint8_t regAddr)
    {
      return spi.readReg(regAddr);
    }
    inline void readBurst(uint8_t *buf, uint8_t regAddr, uint8_t len)
    {
      spi.readBurst(buf, regAddr, len);
    }

    uint8_t interruptMode()
    {
      return FALLING;
    };

    void setIdle()
    {
      // DPRINTLN("Si4431 enter powerdown");

#ifdef USE_WOR
      DPRINTLN("ERROR: WOR is not implemented for Si4431!");
#else
      // enter power down state
      (void)readReg(SI4431_REG_INTERRUPT_STATUS_1);
      (void)readReg(SI4431_REG_INTERRUPT_STATUS_2);
      writeReg(SI4431_REG_OP_FUNC_CONTROL_1, SI4431_OFC1_NONE);
      pinMode(MISO, INPUT);
#endif

      if (PWRPIN < 0xff)
      {
        spi.shutdown();
        digitalWrite(PWRPIN, HIGH);
      }
    }

    void wakeup(bool flush)
    {
      // DPRINTLN("Si4431 wakeup");
      pinMode(MISO, INPUT_PULLUP);
      if (PWRPIN < 0xff)
      {
        init();
      }

      writeReg(SI4431_REG_OP_FUNC_CONTROL_1, SI4431_OFC1_RXON | SI4431_OFC1_XTALON);
      if (flush == true)
      {
        flushRx();
      }
    }

    uint8_t reset()
    {
      // DPRINTLN("Si4431 reset");
      // read & clear any IRQ
      (void)readReg(SI4431_REG_INTERRUPT_STATUS_1);
      (void)readReg(SI4431_REG_INTERRUPT_STATUS_2);

      // write SWRES | XTON
      writeReg(SI4431_REG_OP_FUNC_CONTROL_1, SI4431_OFC1_SWRES | SI4431_OFC1_XTALON);

      // wait for IRQ
      DPRINT("wait for Si4431 being reset");
      while (digitalRead(IRQPIN))
        DPRINT(".");
      DPRINTLN("");
      // TODO: use correct function and pin definition
      // TODO: add timeout

      for (uint8_t i = 0; i < 200; i++)
      {
        if (readReg(SI4431_REG_INTERRUPT_STATUS_2) & SI4431_IRQ2_CHIP_READY)
        {
          break;
        }
        _delay_us(100);
      }

      (void)readReg(SI4431_REG_INTERRUPT_STATUS_2);

      // enable only XTAL IRQ (chip ready)
      // writeReg(SI4431_REG_INTERRUPT_ENABLE_2, SI4431_IRQ2_CHIP_READY);
      writeReg(SI4431_REG_INTERRUPT_ENABLE_1, SI4431_IRQ1_VALID_PACKET_RECEIVED);
      writeReg(SI4431_REG_INTERRUPT_ENABLE_2, 0);
      // writeReg(SI4431_REG_INTERRUPT_ENABLE_2, SI4431_IRQ2_SYNC_WORD_DETECTED);

      // TODO: return value is never used, neither for CC1101 nor Si4431
      return 0;
    }

    bool init()
    {
      DPRINTLN("Si4431 init");
      if (PWRPIN < 0xff)
      {
        pinMode(PWRPIN, OUTPUT);
        digitalWrite(PWRPIN, LOW);
        _delay_ms(2);
      }
      spi.init();

      reset();

      // define init settings for Si4431
      // recorded from stock HM-Sec-SCo
      static const uint8_t initVal[] PROGMEM = {
          /*register                                  value       reset   explanation of delta to reset value   */
          SI4431_REG_OP_FUNC_CONTROL_1, 0x01,     //  0x01
          SI4431_REG_XOSC_LOAD_CAPACITANCE, 0x6B, //  0x7F
          // stock HM-Sec-SCo
          // SI4431_REG_GPIO0_CONFIG,                  0x1F,   //  0x00    GPIO tied to GND
          //  stan23 debugging
          // SI4431_REG_GPIO0_CONFIG,                  0x40,   //  0x00    GPIO0 = POR
          // SI4431_REG_GPIO0_CONFIG,                  0x0A,   //  0x00    GPIO0 = IO
          // SI4431_REG_GPIO0_CONFIG,                  0x12,   //  0x00    GPIO0 = TX state
          // SI4431_REG_GPIO0_CONFIG,                  0x0F,   //  0x00    GPIO0 = RX/TX data clk
          // SI4431_REG_GPIO0_CONFIG,                  0x15,   //  0x00    GPIO0 = RX state
          // SI4431_REG_GPIO0_CONFIG,                  0x19,   //  0x00    GPIO0 = valid preamble detected
          // SI4431_REG_GPIO0_CONFIG,                  0x1A,   //  0x00    GPIO0 = invalid preamble detected
          SI4431_REG_GPIO0_CONFIG, 0x1B,                 //  0x00    GPIO0 = sync word detected
          SI4431_REG_GPIO1_CONFIG, 0x1F,                 //  0x00    GPIO tied to GND
          SI4431_REG_GPIO2_CONFIG, 0x1F,                 //  0x00    GPIO tied to GND
          SI4431_REG_FREQUENCY_BAND_SELECT, 0x73,        //  0x75    900..920 MHz band
          SI4431_REG_NOM_CARRIER_FREQUENCY_1, 0x67,      //  0xBB
          SI4431_REG_NOM_CARRIER_FREQUENCY_0, 0xC0,      //  0x80
          SI4431_REG_HEADER_CONTROL_2, 0x0E,             //  0x22    no header, sync word 3+2+1+0
          SI4431_REG_SYNC_WORD_3, 0xE9,                  //  0x2D
          SI4431_REG_SYNC_WORD_2, 0xCA,                  //  0xD2
          SI4431_REG_SYNC_WORD_1, 0xE9,                  //  0x00
          SI4431_REG_SYNC_WORD_0, 0xCA,                  //  0x00
          SI4431_REG_TX_POWER, 0x1F,                     //  0x18    max output power
          SI4431_REG_RX_FIFO_CONTROL, 0x03,              //  0x37    IRQ at >3 byte in RX FIFO
          SI4431_REG_TX_FIFO_CONTROL_2, 0x07,            //  0x40    IRQ at <8 byte in TX FIFO
          SI4431_REG_TX_DATA_RATE_1, 0x51,               //  0x0A    10 kbps
          SI4431_REG_TX_DATA_RATE_0, 0xEC,               //  0x3D
          SI4431_REG_MODULATION_MODE_CONTROL_1, 0x2C,    //  0x0C    low data rate
          SI4431_REG_FREQUENCY_DEVIATION, 0x1E,          //  0x20
          SI4431_REG_MODULATION_MODE_CONTROL_2, 0x22,    //  0x00    FIFO mode, FSK
          SI4431_REG_IF_FILTER_BANDWIDTH, 0x1E,          //  0x01    620.7 kHz
          SI4431_REG_CLOCK_REC_OVERSAMP_RATIO, 0xC8,     //  0x64    25x oversampling
          SI4431_REG_CLOCK_REC_OFFSET_2, 0x00,           //  0x01
          SI4431_REG_CLOCK_REC_OFFSET_1, 0xA3,           //  0x47
          SI4431_REG_CLOCK_REC_OFFSET_0, 0xD7,           //  0xA7
          SI4431_REG_CLOCK_REC_TIMING_LOOP_GAIN_1, 0x00, //  0x02
          SI4431_REG_CLOCK_REC_TIMING_LOOP_GAIN_0, 0xAE, //  0x8F
          SI4431_REG_AFC_LOOP_GEARSHIFT_OVERRIDE, 0x40,  //  0x44
          SI4431_REG_AFC_TIMING_CONTROL, 0x0A,           //  0x0A
          SI4431_REG_AFC_LIMITER, 0x28,                  //  0x00
          SI4431_REG_CLOCK_REC_GEARSHIFT_OVERRIDE, 0x03, //  0x03
          SI4431_REG_AGC_OVERRIDE_1, 0x60,               //  0x20

          // manually added
          SI4431_REG_DATA_ACCESS_CONTROL, 0x80, //  0x8D    no TxPkt, no CRC as it does not fit to CC1101
          SI4431_REG_HEADER_CONTROL_1, 0x00,    //  0x0C    no header check
      };

      bool initOK = true;
      for (uint8_t i = 0; i < sizeof(initVal); i += 2)
      { // write init values to Si4431
        bool initres = initReg(pgm_read_byte(&initVal[i]), pgm_read_byte(&initVal[i + 1]));
        // if any initReg fails, initOK has to be false
        if (initres == false)
          initOK = false;
      }

      DPRINT(F("Si4431 Type: "));
      DHEX(readReg(SI4431_REG_DEVICE_TYPE));
      DPRINT(F(", Version: "));
      DHEX(readReg(SI4431_REG_DEVICE_VERSION));
      DPRINT(F(", Status: "));
      DHEXLN(readReg(SI4431_REG_DEVICE_STATUS));

      DPRINTLN(F(" - ready"));

      writeReg(SI4431_REG_OP_FUNC_CONTROL_1, SI4431_OFC1_RXON | SI4431_OFC1_XTALON);

      return initOK;
    }

    bool initReg(uint8_t regAddr, uint8_t val, uint8_t retries = 3)
    {
      writeReg(regAddr, val);
      uint8_t val_read = readReg(regAddr);
      bool initResult = true;
      if (val_read != val)
      {
        if (retries > 0)
        {
          initResult = initReg(regAddr, val, --retries);
          _delay_ms(1);
        }
        else
        {
          DPRINT(F("Error at "));
          DHEX(regAddr);
          DPRINT(F(" expected: "));
          DHEX(val);
          DPRINT(F(" read: "));
          DHEXLN(val_read);
          return false;
        }
      }
      return initResult;
    }

    void tuneFreq(__attribute__((unused)) uint8_t freq2, __attribute__((unused)) uint8_t freq1, __attribute__((unused)) uint8_t freq0)
    {
      // Si4431 does not have problems with wrong oscillator capacitors on cheap modules
    }

    uint8_t rssi() const
    {
      return rss;
    }

    void flushRx()
    {
      // DPRINTLN("Si4431 flushRx");
      //  set and clear bit FIFO Clear RX
      writeReg(SI4431_REG_OP_FUNC_CONTROL_2, SI4431_OFC2_FFCLRRX);
      writeReg(SI4431_REG_OP_FUNC_CONTROL_2, SI4431_OFC2_NONE);
    }

    bool detectBurst()
    {
      // DPRINTLN("Si4431 detectBurst");
      // TODO: how to detect a burst?
      // Si4431 datasheet does not mention any "burst"
      return 0;
    }

    void pollRSSI()
    {
      // DPRINTLN("Si4431 pollRSSI");
      calculateRSSI(readReg(SI4431_REG_RSSI, 0)); // read RSSI from STATUS register
    }

  protected:
    // CC1101 CRC-16 implementation with init value 0xFFFF
    // https://www.ti.com/lit/pdf/swra111

#define CRC16_POLY 0x8005

    uint16_t calcCrcWorker(uint8_t crcData, uint16_t crcReg)
    {
      for (uint8_t i = 0; i < 8; i++)
      {
        if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
          crcReg = (crcReg << 1) ^ CRC16_POLY;
        else
          crcReg = (crcReg << 1);
        crcData <<= 1;
      }
      return crcReg;
    }

    /** calculate CRC-16 on buf with size len, and using len as the first byte */
    uint16_t calculateCrc(uint8_t *buf, uint8_t len)
    {
      uint16_t crc = 0xFFFF;
      crc = calcCrcWorker(len, crc);
      for (uint8_t i = 0; i < len; i++)
      {
        crc = calcCrcWorker(buf[i], crc);
      }
      return crc;
    }

    void whitenBuffer(uint8_t *buf, uint8_t len, uint8_t offset = 0)
    {
      const uint8_t pn9[] = {
          0xFF, 0xE1, 0x1D, 0x9A, 0xED, 0x85, 0x33, 0x24, 0xEA, 0x7A,
          0xD2, 0x39, 0x70, 0x97, 0x57, 0x0A, 0x54, 0x7D, 0x2D, 0xD8,
          0x6D, 0x0D, 0xBA, 0x8F, 0x67, 0x59, 0xC7, 0xA2, 0xBF, 0x34,
          0xCA, 0x18, 0x30, 0x53, 0x93, 0xDF, 0x92, 0xEC, 0xA7, 0x15};

      if (len + offset < sizeof(pn9))
      {
        for (uint8_t i = 0; i < len; i++)
        {
          buf[i] = buf[i] ^ pn9[i + offset];
        }
      }
      else
      {
        DPRINTLN("ERR: packet too large to be whitened!");
      }
    }

    void calculateRSSI(uint8_t rsshex)
    {
      rss = -1 * ((((int16_t)rsshex - ((int16_t)rsshex >= 128 ? 256 : 0)) / 2) - 74);
    }

    uint8_t sndData(uint8_t *buf, uint8_t size, __attribute__((unused)) uint8_t burst)
    {
      // sndData() is called while nIRQ is disabled in Radio.h, so the GPIO can be safely polled for status changes
      // DPRINTLN("Si4431 sndData -----------------------------------");

      // DPRINT("  buf: ");DHEX(buf, size);DPRINTLN("");
      uint16_t crc;
      uint8_t packetBuffer[40];

      static const uint8_t preambleShort[4] = {0xAA, 0xAA, 0xAA, 0xAA};
      static const uint8_t preambleLong[32] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                               0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                               0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                               0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
      static const uint8_t syncword[4] = {0xE9, 0xCA, 0xE9, 0xCA};

      crc = calculateCrc(buf, size);
      // DPRINT("  calculated CRC: ");DHEX(crc);DPRINTLN("");

      // build packet: first byte is the length
      packetBuffer[0] = size;
      // then the encoded payload (len-1 bytes)
      memcpy(&packetBuffer[1], buf, size);
      // then 2 bytes of CRC
      packetBuffer[size + 1] = crc >> 8;
      packetBuffer[size + 2] = crc & 0xFF;
      // DPRINT("packet:   ");DHEX(packetBuffer, size+3);DPRINTLN("");

      whitenBuffer(packetBuffer, size + 3);
      // DPRINT("packet wh: ");DHEX(packetBuffer, size+3);DPRINTLN("");

      // set and clear bit FIFO Clear TX
      writeReg(SI4431_REG_OP_FUNC_CONTROL_2, SI4431_OFC2_FFCLRTX);
      writeReg(SI4431_REG_OP_FUNC_CONTROL_2, SI4431_OFC2_NONE);

      // clear interrupts
      (void)readReg(SI4431_REG_INTERRUPT_STATUS_1);
      (void)readReg(SI4431_REG_INTERRUPT_STATUS_2);

      // TODO: is the transmit size necessary if packet mode is not used?
      writeReg(SI4431_REG_TRANSMIT_PACKET_LENGTH, size + 1);

      // no TX packet handling, therefore preamble & syncwords have to be sent manually

      if (burst)
      {
        // DPRINTLN("Send Burst");

        // use nIRQ pin for FIFO_ALMOST_EMPTY state
        writeReg(SI4431_REG_INTERRUPT_ENABLE_1, SI4431_IRQ1_TX_FIFO_ALMOST_EMPTY);

        // fill the FIFO with 64bytes of burstPacket
        writeBurst(SI4431_REG_FIFO_ACCESS, preambleLong, sizeof(preambleLong));
        writeBurst(SI4431_REG_FIFO_ACCESS, preambleLong, sizeof(preambleLong));

        // set TX on
        writeReg(SI4431_REG_OP_FUNC_CONTROL_1, SI4431_OFC1_XTALON | SI4431_OFC1_TXON);

        // now check the nIRQ pin until the FIFO is almost empty - or timeout after max. 55ms
        for (uint16_t i = 0; i < 5500; i++)
        {
          if (digitalRead(IRQPIN) == LOW)
          {
            break;
          }
          _delay_us(10);
        }

        for (uint8_t t = 0; t < 12; t++)
        {
          (void)readReg(SI4431_REG_INTERRUPT_STATUS_1);
          (void)readReg(SI4431_REG_INTERRUPT_STATUS_2);
          writeBurst(SI4431_REG_FIFO_ACCESS, preambleLong, sizeof(preambleLong));
          for (uint16_t i = 0; i < 3000; i++)
          {
            if (digitalRead(IRQPIN) == LOW)
            {
              break;
            }
            _delay_us(10);
          }
        }
      }

      writeBurst(SI4431_REG_FIFO_ACCESS, preambleShort, sizeof(preambleShort));
      writeBurst(SI4431_REG_FIFO_ACCESS, syncword, sizeof(syncword));
      writeBurst(SI4431_REG_FIFO_ACCESS, packetBuffer, size + 3);

      // set mode to transmit
      writeReg(SI4431_REG_OP_FUNC_CONTROL_1, SI4431_OFC1_TXON | SI4431_OFC1_XTALON);

      // wait until packet is sent
      writeReg(SI4431_REG_INTERRUPT_ENABLE_1, SI4431_IRQ1_PACKET_SENT);
      for (uint16_t i = 0; i < 2000; i++)
      {
        if (digitalRead(IRQPIN) == LOW)
        {
          break;
        }
        _delay_us(10);
      }

      // TODO: why is 0x40 used? max expected packet size?
      writeReg(SI4431_REG_TRANSMIT_PACKET_LENGTH, 0x40);

      // enable Rx
      writeReg(SI4431_REG_OP_FUNC_CONTROL_1, SI4431_OFC1_RXON | SI4431_OFC1_XTALON);
      // set nIRQ to trigger RX received
      writeReg(SI4431_REG_INTERRUPT_ENABLE_1, SI4431_IRQ1_VALID_PACKET_RECEIVED);
      return true;
    }

    uint8_t rcvData(uint8_t *buf, uint8_t size)
    {
      // DPRINTLN("Si4431 rcvData -----------------------------------");

      // disable receiver
      writeReg(SI4431_REG_OP_FUNC_CONTROL_1, SI4431_OFC1_XTALON);
      // clear interrupts
      (void)readReg(SI4431_REG_INTERRUPT_STATUS_1);
      (void)readReg(SI4431_REG_INTERRUPT_STATUS_2);

      uint8_t packetBytes = 0;
      uint8_t rxBytes = 0;

      // TODO: handle RX FIFO overflow, signaled by IRQ registers
      // read packet length and whiten it
      packetBytes = readReg(SI4431_REG_FIFO_ACCESS) ^ 0xFF;
      // check that packet fits into the buffer
      if (packetBytes <= size)
      {
        // read packetSize bytes + 2 for CRC
        readBurst(buf, SI4431_REG_FIFO_ACCESS, packetBytes + 2);
        // len is already whitened, so use offset of 1
        whitenBuffer(buf, packetBytes + 2, 1);
        uint16_t crcCalc = calculateCrc(buf, packetBytes);
        if (crcCalc == (buf[packetBytes] << 8 | buf[packetBytes + 1]))
        {
          rxBytes = packetBytes;
        }
        else
        {
          DPRINTLN("  CRC failed!");
        }
        calculateRSSI(readReg(SI4431_REG_RSSI));
      }
      else
      {
        DPRINT(F("Packet too big: "));
        DDECLN(packetBytes);
      }

      // clear interrupts
      (void)readReg(SI4431_REG_INTERRUPT_STATUS_1);
      (void)readReg(SI4431_REG_INTERRUPT_STATUS_2);

      // DPRINT("-> ");
      // DHEXLN(buf+1,rxBytes);
      flushRx();

      // enable Rx
      writeReg(SI4431_REG_OP_FUNC_CONTROL_1, SI4431_OFC1_RXON | SI4431_OFC1_XTALON);

      return rxBytes; // return number of byte in buffer
    }
  };

}

#endif
