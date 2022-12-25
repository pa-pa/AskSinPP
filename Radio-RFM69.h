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
//- AskSin RFM69 functions -----------------------------------------------------------------------------------------------
//- based on
//- -----------------------------------------------------------------------------------------------------------------------

/*
  TODO:
  - mit USE_WOR ist der rssi Wert immer 0
  - mit USE_WOR Modul wacht gelegentlich zu oft auf.
  - USE_OTA_BOOTLOADER_FREQUENCY
  - write / readBurst um mehrere Daten auf einmal zu schreiben / lesen

*/

#ifndef _RFM69_H
#define _RFM69_H

namespace as
{

  // RFM69 registers

#define RFM69_REG_FIFO 0x00
#define RFM69_REG_VERSION 0x10
#define RFM69_REG_OPMODE 0x01
#define RFM69_REG_DATAMODUL 0x02
#define RFM69_REG_BITRATEMSB 0x03
#define RFM69_REG_BITRATELSB 0x04
#define RFM69_REG_FDEVMSB 0x05
#define RFM69_REG_FDEVLSB 0x06
#define RFM69_REG_FRFMSB 0x07
#define RFM69_REG_FRFMID 0x08
#define RFM69_REG_FRFLSB 0x09
#define RFM69_REG_OSC1 0x0A
#define RFM69_REG_LISTEN1 0x0D
#define RFM69_REG_LISTEN2 0x0E
#define RFM69_REG_LISTEN3 0x0F
#define RFM69_REG_PALEVEL 0x11
#define RFM69_REG_RXBW 0x19
#define RFM_REG_RSSI_CONFIG 0x23
#define RFM_REG_RSSI_VALUE 0x24
#define RFM69_REG_DIOMAPPING1 0x25
#define RFM69_REG_DIOMAPPING2 0x26
#define RFM69_REG_IRQFLAGS2 0x28
#define RFM69_REG_RSSITHRESH 0x29
#define RFM69_REG_RXTIMEOUT1 0x2A
#define RFM69_REG_RXTIMEOUT2 0x2B
#define RFM69_REG_PREAMBLEMSB 0x2C
#define RFM69_REG_PREAMBLELSB 0x2D
#define RFM69_REG_SYNCCONFIG 0x2E
#define RFM69_REG_SYNCVALUE1 0x2F
#define RFM69_REG_SYNCVALUE2 0x30
#define RFM69_REG_SYNCVALUE3 0x31
#define RFM69_REG_SYNCVALUE4 0x32
#define RFM69_REG_SYNCVALUE5 0x33
#define RFM69_REG_PACKETCONFIG1 0x37
#define RFM69_REG_PAYLOADLENGTH 0x38
#define RFM69_REG_FIFOTHRESH 0x3C
#define RFM69_REG_PACKETCONFIG2 0x3D
#define RFM69_REG_TESTDAGC 0x6F

#define RFM69_REG_FIFO 0x00
#define RFM69_REG_VERSION 0x10
#define RFM69_REG_OPMODE 0x01
#define RFM69_REG_DATAMODUL 0x02
#define RFM69_REG_BITRATEMSB 0x03
#define RFM69_REG_BITRATELSB 0x04
#define RFM69_REG_FDEVMSB 0x05
#define RFM69_REG_FDEVLSB 0x06
#define RFM69_REG_FRFMSB 0x07
#define RFM69_REG_FRFMID 0x08
#define RFM69_REG_FRFLSB 0x09
#define RFM69_REG_OSC1 0x0A
#define RFM69_REG_LISTEN1 0x0D
#define RFM69_REG_LISTEN2 0x0E
#define RFM69_REG_LISTEN3 0x0F
#define RFM69_REG_PALEVEL 0x11
#define RFM69_REG_RXBW 0x19
#define RFM_REG_RSSI_CONFIG 0x23
#define RFM_REG_RSSI_VALUE 0x24
#define RFM69_REG_DIOMAPPING1 0x25
#define RFM69_REG_DIOMAPPING2 0x26
#define RFM69_REG_IRQFLAGS2 0x28
#define RFM69_REG_RSSITHRESH 0x29
#define RFM69_REG_RXTIMEOUT1 0x2A
#define RFM69_REG_RXTIMEOUT2 0x2B
#define RFM69_REG_PREAMBLEMSB 0x2C
#define RFM69_REG_PREAMBLELSB 0x2D
#define RFM69_REG_SYNCCONFIG 0x2E
#define RFM69_REG_SYNCVALUE1 0x2F
#define RFM69_REG_SYNCVALUE2 0x30
#define RFM69_REG_SYNCVALUE3 0x31
#define RFM69_REG_SYNCVALUE4 0x32
#define RFM69_REG_SYNCVALUE5 0x33
#define RFM69_REG_PACKETCONFIG1 0x37
#define RFM69_REG_PAYLOADLENGTH 0x38
#define RFM69_REG_FIFOTHRESH 0x3C
#define RFM69_REG_PACKETCONFIG2 0x3D
#define RFM69_REG_TESTDAGC 0x6F

#define RFM69_IRQFLAGS2_FIFOOVERRUN 0x10
#define RFM69_OPMODE_SLEEP 0x00
#define RFM69_HM_PAYLOAD_LENGTH 30

#define RFM69_OPMODE_STANDBY 0x04
#define RFM69_OPMODE_RECEIVER 0x10
#define RFM69_OPMODE_TRANSMITTER 0x0C

  class RadioTools
  {
  public:
    // source: Thx to Uli from https://forum.fhem.de/index.php?topic=49300.msg409698#msg409698
    static void xOr_PN9(uint8_t *p_buf, uint8_t p_len)
    {
      uint8_t bit_five = 0;
      uint8_t bit_zero = 0;
      uint8_t carry_new = 0;

      uint8_t key = 0xFF; // initialize value
      uint8_t carry = 1;  // initialize value

      for (uint8_t i = 0; i < p_len; i++)
      {
        p_buf[i] ^= key; // xor

        // PN9 Key generation to get next Key for xOr
        for (uint8_t j = 0; j < 8; j++)
        {
          bit_five = (key & (1 << 5)) >> 5;
          bit_zero = (key & (1 << 0)) >> 0;
          carry_new = bit_five ^ bit_zero;
          key >>= 1;
          key |= (carry << 7);
          carry = carry_new;
        }
      }
    }

    // source: Thx to Uli from https://forum.fhem.de/index.php?topic=49300.msg409698#msg409698
    static uint16_t calcCRC16hm(uint8_t *p_data, uint8_t p_length)
    {
      uint16_t checksum = 0xffff;
      for (int i = 0; i < p_length; i++)
      {
        uint8_t bte = p_data[i];
        for (int j = 0; j < 8; j++)
        {
          if (((checksum & 0x8000) >> 8) ^ (bte & 0x80))
          {
            checksum = (checksum << 1) ^ 0x8005;
          }
          else
          {
            checksum = (checksum << 1);
          }
          bte <<= 1;
        }
      }
      return checksum;
    }

    /// @brief paramKonvertiert die ankommenden Daten in Rawdaten für das RF Modul. Inc. Whitening und Crc.
    /// @param p_data Daten
    /// @param p_rawData Wird mit den raw Daten gefüllt. Muss mindestens 3 Byte länger sein
    /// @param p_size Länge der Daten ohne crc
    /// @return Länge + 3 ( Längenbyte + 2 crc Byte)
    static uint8_t convertToRawData(const uint8_t *p_data, uint8_t *p_rawData, const uint8_t p_size)
    {
      uint8_t size = p_size + 3;
      p_rawData[0] = p_size;
      for (uint8_t i = 0; i < p_size; i++)
      {
        if (i + 1 < RFM69_HM_PAYLOAD_LENGTH)
        {
          p_rawData[i + 1] = p_data[i];
        }
        else
        {
          DPRINTLN(F("Packet too long"));
        }
      }
      uint16_t crc = calcCRC16hm(p_rawData, p_size + 1); // size +1 crc über payload + längenbyte
      DPRINTLN("CCR");
      DHEX(crc);
      DPRINTLN("");
      p_rawData[p_size + 1] = (crc >> 8) & 0xff;
      p_rawData[p_size + 2] = crc & 0xff;
      xOr_PN9(p_rawData, size);
      return size;
    }
  };

  template <class SPIType, uint8_t PWRPIN, uint8_t IRQPIN>
  class RFM69
  {
  protected:
    SPIType spi;
    uint8_t rss; // signal strength

#ifdef USE_OTA_BOOTLOADER_FREQUENCY
    DPRINTLN("ERROR: USE_OTA_BOOTLOADER_FREQUENCY is not implemented for RFM69!");
#endif

  public:
    RFM69() : rss(0)
#ifdef USE_OTA_BOOTLOADER_FREQUENCY
              ,
              DPRINTLN("ERROR: USE_OTA_BOOTLOADER_FREQUENCY is not implemented for RFM69!");
#endif
    {
    }

    /// @brief Gibt zurück, ob bei einem Interupt auf die steigende oder fallende Flanke getriggert werden soll.
    /// @param return RISING, es muss auf die steigende Flanke getriggert werden, da der Interrupt Pin bis zum auslesen des FIFO auf high bleibt.
    uint8_t interruptMode()
    {
      return RISING;
    };

    void setIdle()
    {
      DPRINTLN(__func__);
      DPRINTLN("RFM69 enter powerdown");
      writeReg(RFM69_REG_OPMODE, 0b00000100, 4, 2); // standby
#ifdef USE_WOR
      writeCmd(RFM69_REG_OSC1, 0b10000000);
      uint8_t timeout = 0;
      uint8_t osc1 = readReg(RFM69_REG_OSC1);
      while (!(osc1 & 0b01000000))
      {
        if (timeout >= 10)
        {
          DPRINTLN("Timoeout!");
          break;
        }
        timeout++;
        _delay_ms(10);
        osc1 = readReg(RFM69_REG_OSC1);
      }

      writeCmd(RFM69_REG_OPMODE, 0b00000100);
      uint8_t opMode = readReg(RFM69_REG_OPMODE);
      opMode |= (1 << 6);
      writeCmd(RFM69_REG_OPMODE, opMode);
#endif
    }

    /// @brief Ist das RFM69 ausgeschaltet, wird es eingeschaltet und init() aufgerufen
    /// Wenn WOR aktiv ist wird der "Listen Mode" deaktiviert.
    /// Receivermode wird aktiviert.
    /// @param p_flush Wenn true wird flushrx() aufgerufen.
    void wakeup(bool p_flush)
    {
      DPRINTLN(__func__);
      DPRINTLN("RFM69 wakeup");
      if (PWRPIN < 0xff)
      {
        init();
      }

      spi.ping();
      if (p_flush == true)
      {
        flushrx();
      }
#ifdef USE_WOR
      uint8_t opmode = readReg(RFM69_REG_OPMODE);
      opmode |= (1 << 5);
      opmode &= ~(1 << 6);
      writeCmd(RFM69_REG_OPMODE, opmode);
#endif
      // set RX Mode
      DPRINTLN(F("Set RX Mode"));
      writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_RECEIVER, 4, 2);
    }

    /// @brief Das RFM69 kann nur über den Reset Pin oder durch aus und einschalten der Spannungsversorgung resettet werden. Dies könnte hier implementiert werden.
    /// Wenn WOR aktiv ist wird der "Listen Mode" deaktiviert.
    /// @returns immer 1
    uint8_t reset()
    {
      DPRINTLN(__func__);
      DPRINTLN("RFM69 reset");
      uint8_t ret = 1;
#ifdef USE_WOR
      uint8_t opmode = readReg(RFM69_REG_OPMODE);
      opmode |= (1 << 5);
      opmode &= ~(1 << 6);
      writeCmd(RFM69_REG_OPMODE, opmode);
#endif
      return ret;
    }

    /// @brief Liest einen Wert aus dem Register
    /// @param p_reg Register Adresse
    /// @returns Register Wert
    uint8_t readReg(uint8_t p_reg)
    {
      return spi.readReg(p_reg, 0);
    }

    void writeBurstReg(uint8_t p_reg, uint8_t *p_buf, uint8_t p_len)
    {
      // TODO
    }

    void readBurst(uint8_t *p_buf, uint8_t p_regAddr, uint8_t p_len)
    {
      // TODO
    }

    /// @brief Schreibt einen Wert in das Register.
    /// Im Gegensatz zu writeReg wird immer der komplette Wert geschrieben und hinterher nicht geprüft.
    /// @param p_reg Register Adresse
    /// @param p_value Register Wert
    void writeCmd(uint8_t p_reg, uint8_t p_value)
    {
      uint8_t reg = p_reg | (1 << 7);
      spi.writeReg(reg, p_value);
      _delay_ms(10);
    }

    /// @brief Schreibt einen Wert in das Register.
    /// Das Register wird ausgelesen. Danch werden die nötigen Bits übernommen und zurückgeschrieben. Zuletzt wird das Register ausgelesen und mit dem Sollwert verglichen.
    /// @param p_reg Register Adresse
    /// @param p_value Register Wert
    /// @param p_msb Ab diesem Bit wird eine Änderung berücksichtigt
    /// @param p_lsb Bis zu diesem Bit wird eine Änderung berücksichtigt
    /// @param p_retries Anzahl der Versuche, falls das Schreiben fehlschlägt.
    /// @return true bei Erfolg
    bool writeReg(uint8_t p_reg, uint8_t p_value, uint8_t p_msb = 7, uint8_t p_lsb = 0, uint8_t p_retries = 3)
    {
      uint8_t reg = p_reg | (1 << 7);
      uint8_t oldValue = readReg(p_reg);
      uint8_t retries = p_retries;
      if ((p_msb > 7) || (p_lsb > 7) || (p_lsb > p_msb))
      {
        DPRINT(F("writeReg Error "));
        return (false);
      }
      uint8_t mask = ~((0b11111111 << (p_msb + 1)) | (0b11111111 >> (8 - p_lsb)));
      uint8_t currValue = readReg(p_reg);
      uint8_t valueToWrite = (currValue & ~mask) | (p_value & mask);
      spi.writeReg(reg, valueToWrite);
      _delay_ms(10);
      uint8_t valRead = readReg(p_reg);
      bool result = valRead == valueToWrite;
      bool ok = result;
      while (!ok)
      {
        spi.writeReg(reg, valueToWrite);
        _delay_ms(10);
        valRead = readReg(p_reg);
        result = valRead == valueToWrite;
        ok = result;
        if (!ok)
        {
          DPRINT(F("NOT OK. try: "));
          DPRINTLN(retries);
          ok = retries-- <= 1;
        }
      }
      if (!result)
      {
        DPRINT(F("Error at "));
        DHEX(p_reg);
        DPRINT(F(" expected: "));
        DHEX(valueToWrite);
        DPRINT(F(" read: "));
        DHEX(valRead);
        DPRINT(F(" old: "));
        DHEXLN(oldValue);
        return false;
      }
      return true;
    }

    bool init()
    {
      bool initOK = true;
      DPRINTLN(__func__);
      if (PWRPIN < 0xff)
      {
        pinMode(PWRPIN, OUTPUT);
        digitalWrite(PWRPIN, LOW);
        _delay_ms(2);
      }
      spi.init(); // init the hardware to get access to the RF modul

      reset();

      uint8_t version = spi.readReg(RFM69_REG_VERSION, 0);
      if (version == 0x24)
      {
        DPRINT(F("RFM69 Version OK "));
      }
      else
      {
        DPRINT(F("RFM69 Version NOT OK "));
        while (true)
        {
        }
      }
      DPRINT(F("RFM69 Version - "));
      DHEXLN(version);
      writeReg(RFM69_REG_OPMODE, 0b10000100);
      writeReg(RFM69_REG_DATAMODUL, 0b00000010);
      writeReg(RFM69_REG_BITRATEMSB, 0x0C);                       // Bitrate 10 kBaud
      writeReg(RFM69_REG_BITRATELSB, 0x80);                       //
      writeReg(RFM69_REG_FDEVMSB, 0x01);                          // Deviation 20 kHz
      writeReg(RFM69_REG_FDEVLSB, 0x48);                          //
      writeCmd(RFM69_REG_FRFMSB, 0xD9);                           // FRF = (868.299866 * 524288) / 32 = 14226225
      writeCmd(RFM69_REG_FRFMID, 0x13);                           //
      writeReg(RFM69_REG_FRFLSB, 0x31);                           //
      writeReg(RFM69_REG_PALEVEL, 0b01111111);                    // Outputpower
      writeReg(RFM69_REG_RXBW, 0x4A);                             // cutoff frequency 4%, RxBwMant 20, RxBwExp 2
      writeReg(RFM69_REG_DIOMAPPING1, 0b01000001);                // DioMapping
      writeReg(RFM69_REG_DIOMAPPING2, 0b01000111);                //
      writeCmd(RFM69_REG_IRQFLAGS2, RFM69_IRQFLAGS2_FIFOOVERRUN); // clear flags and fifo
      writeReg(RFM69_REG_RSSITHRESH, 170);                        // dBm = (-Sensitivity / 2)
      writeReg(RFM69_REG_PREAMBLEMSB, 0x00);                      // Size of the preamble
      writeReg(RFM69_REG_PREAMBLELSB, 0x04);                      //
      writeReg(RFM69_REG_SYNCCONFIG, 0x98);                       // sync on, fill FIFO if SyncAddress interrupt occurs, syncWord size 4 , 0 SyncTol
      writeReg(RFM69_REG_SYNCVALUE1, 0xE9);                       // sync Word
      writeReg(RFM69_REG_SYNCVALUE2, 0xCA);                       //
      writeReg(RFM69_REG_SYNCVALUE3, 0xE9);                       //
      writeReg(RFM69_REG_SYNCVALUE4, 0xCA);                       //
      writeReg(RFM69_REG_PACKETCONFIG1, 0x08);                    // packet fixed length, no whitening, no crc, no AddressFiltering
      writeReg(RFM69_REG_PAYLOADLENGTH, 30);                      // Payloadlength
      writeReg(RFM69_REG_FIFOTHRESH, 0x8F);                       // txStart FifoNotEmpty
      writeReg(RFM69_REG_PACKETCONFIG2, 0x00);                    // AutoRxRestart off
      writeReg(RFM69_REG_TESTDAGC, 0x30);                         //
      writeReg(RFM69_REG_LISTEN1, 0b10010100);                    //
      writeReg(RFM69_REG_LISTEN2, 86);                            //
      writeReg(RFM69_REG_LISTEN3, 25);                            //
      writeReg(RFM69_REG_RXTIMEOUT1, 1);                          // wenn 1*16*Tbit (8 Byte) nach kein Rssi Interrupt erkannt wurde -> Timeout
      writeReg(RFM69_REG_RXTIMEOUT2, 146);                        // wenn 146*16*Tbit (292 Byte) nach Rssi Interrupt kein PayloadReady interrupt erkannt wurde -> Timeout

      // Sleep
      writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_SLEEP, 4, 2);

      DPRINTLN(F(" - ready"));
      return initOK;
    }

    bool initReg(uint8_t p_regAddr, uint8_t p_val, uint8_t p_retries = 3)
    {
      DPRINTLN(__func__);
      DPRINTLN("RFM69 initReg");
      bool initResult = writeReg(p_regAddr, p_val, 7, 0, p_retries);
      return initResult;
    }

    void tuneFreq(__attribute__((unused)) uint8_t freq2, __attribute__((unused)) uint8_t freq1, __attribute__((unused)) uint8_t freq0)
    {
      // RFM69 does not have problems with wrong oscillator capacitors on cheap modules
    }

    uint8_t rssi() const
    {
      return rss;
    }

    void flushrx()
    {
      DPRINTLN(__func__);
      // clearIRQFlags
      writeCmd(RFM69_REG_IRQFLAGS2, RFM69_IRQFLAGS2_FIFOOVERRUN);
    }

    bool detectBurst()
    {
      // TODO
      DPRINTLN(__func__);
      DPRINTLN("RFM69 detect Burst");
      return true;
    }

    void pollRSSI()
    {
      DPRINTLN(__func__);
      writeCmd(RFM_REG_RSSI_CONFIG, 0b00000001);
      _delay_ms(100);
      uint8_t rssiConf = readReg(RFM_REG_RSSI_CONFIG);
      if (rssiConf & (1 << 1))
      {
        DPRINTLN("rssi not ready");
      }

      uint8_t rssi = readReg(RFM_REG_RSSI_VALUE);
      DPRINT("RFM69 pollrssi: ");
      DPRINTLN(rssi);
      calculateRSSI(rssi);
    }

  protected:
    void calculateRSSI(uint8_t p_rsshex)
    {
      DPRINTLN(__func__);
      DPRINT("RFM69 calculateRSSI: ");
      rss = p_rsshex / 2;
      DPRINTLN(rss);
    }

    uint8_t sndData(uint8_t *p_buf, uint8_t p_size, uint8_t p_burst)
    {
      DPRINTLN(__func__);
      writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_STANDBY, 4, 2);
      writeCmd(RFM69_REG_IRQFLAGS2, RFM69_IRQFLAGS2_FIFOOVERRUN); // flush fifo

      // Zu lang?
      if (p_size > 64)
      {
        DPRINTLN(F("Packet too lang"));
        // set rx Mode
        DPRINTLN(F("Set rx Mode"));
        writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_RECEIVER, 4, 2);
        return false;
      }

      uint8_t data[RFM69_HM_PAYLOAD_LENGTH] = {0};
      uint8_t rawSize = RadioTools::convertToRawData(p_buf, data, p_size);
      uint8_t payloadLengthOld = readReg(RFM69_REG_PAYLOADLENGTH);
      uint8_t fifoThreshOld = readReg(RFM69_REG_FIFOTHRESH);
      writeReg(RFM69_REG_PAYLOADLENGTH, rawSize);                 // Läge, da FixSize nötig
      uint8_t fifothr = (rawSize - 1 <= 127) ? rawSize - 1 : 127; // kann nie passieren. Maxsize is 64
      writeReg(RFM69_REG_FIFOTHRESH, fifothr);                    // erst senden wenn alle Daten übertragen

      writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_TRANSMITTER, 4, 2);
      if (p_burst)
      {                 // BURST-bit set?
        _delay_ms(350); // according to ELV, devices get activated every 300ms, so send burst for 360ms
      }
      // write packet to FIFO
      for (uint8_t i = 0; i < rawSize; i++)
      {
        writeCmd(RFM69_REG_FIFO, data[i]);
      }
      // erfolgreich gesendet?
      uint8_t irqFlags2 = readReg(RFM69_REG_IRQFLAGS2);
      uint8_t count = 0;
      while (bitRead(irqFlags2, 3) == 0)
      {
        _delay_ms(2);
        irqFlags2 = readReg(RFM69_REG_IRQFLAGS2);
        if (count >= 35)
        {
          DPRINT(F("Error Send Timeout "));
          break;
        }
        count++;
      }
      // Register wiederherstellen
      writeReg(RFM69_REG_PAYLOADLENGTH, payloadLengthOld);
      writeReg(RFM69_REG_FIFOTHRESH, fifoThreshOld);
      writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_RECEIVER, 4, 2);
      return true;
    }

    uint8_t rcvData(uint8_t *p_buf, uint8_t p_size)
    {
      DPRINTLN(__func__);

#ifdef USE_WOR
      uint8_t opmode = readReg(RFM69_REG_OPMODE);
      if (opmode & (1 << 6)) // ListenOn ?
      {
        opmode |= (1 << 5);
        opmode &= ~(1 << 6);
        writeCmd(RFM69_REG_OPMODE, opmode);
      }
#endif
      pollRSSI();
      writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_SLEEP, 4, 2);

      uint8_t data[RFM69_HM_PAYLOAD_LENGTH] = {0};
      if (p_size > 0)
      {
        for (uint8_t i = 0; i < RFM69_HM_PAYLOAD_LENGTH; i++)
        {
          data[i] = readReg(RFM69_REG_FIFO);
          if (!(bitRead(readReg(0x28), 6)))
          {
            break;
          }
        }
      }
      else
      {
        DPRINT(F("No Data?"));
      }
      RadioTools::xOr_PN9(data, RFM69_HM_PAYLOAD_LENGTH);
      uint8_t realLengh = data[0];
      if (realLengh > RFM69_HM_PAYLOAD_LENGTH)
      {
        DPRINTLN("MSG lengh ERROR");
        writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_RECEIVER, 4, 2);
        return 0;
      }
      uint16_t crc = RadioTools::calcCRC16hm(data, realLengh + 1); // size +1 crc über payload + längenbyte
      uint8_t crc1 = (crc >> 8) & 0xff;
      uint8_t crc2 = crc & 0xff;
      if (data[realLengh + 1] != crc1 || data[realLengh + 2] != crc2)
      {
        DPRINTLN("CRC ERROR");
        writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_RECEIVER, 4, 2);
        return 0;
      }

      for (uint8_t i = 0; i < p_size; i++)
      {
        if (i < realLengh)
        {
          p_buf[i] = data[(i + 1)];
        }
        else
        {
          p_buf[i] = 0;
        }
      }
      flushrx();
      writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_RECEIVER, 4, 2);
      return realLengh; // return number of byte in buffer
    }
  };
}

#endif
