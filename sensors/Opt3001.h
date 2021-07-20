//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// OPT3001 adaption by trilu2000
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_OPT3001_h__
#define __SENSORS_OPT3001_h__

#include <Sensors.h>
#include <Wire.h>

namespace as {

  typedef union {
    struct {
      uint8_t FaultCount : 2;                   // rw
      uint8_t MaskExponent : 1;                 // rw
      uint8_t Polarity : 1;                     // rw
      uint8_t Latch : 1;                        // rw
      uint8_t FlagLow : 1;                      // ro
      uint8_t FlagHigh : 1;                     // ro
      uint8_t ConversionReady : 1;              // ro
      uint8_t OverflowFlag : 1;                 // ro
      uint8_t ModeOfConversionOperation : 2;    // rw
      uint8_t ConvertionTime : 1;               // rw
      uint8_t RangeNumber : 4;                  // rw
    };
    uint16_t raw;
  } OPT3001_CONFIG;

  typedef enum {
    RESULT = 0x00,
    CONFIG = 0x01,
    LOW_LIMIT = 0x02,
    HIGH_LIMIT = 0x03,
    MANUFACTURER_ID = 0x7E,
    DEVICE_ID = 0x7F,
  } OPT3001_COMMANDS;

  typedef enum {
    NO_ERROR = 0,
    TIMEOUT = -100,
    I2C_DATA_TOO_LONG = -10,
    I2C_NACK_ON_ADDRESS = -20,
    I2C_NACK_ON_DATA = -30,
    I2C_UNKNOW_ERROR = -40
  } OPT3001_ERROR;

  typedef union {
    uint16_t raw;
    struct {
      uint16_t Result : 12;
      uint8_t Exponent : 4;
    };
  } OPT3001_RR;


  typedef struct {
    uint8_t len;
    const uint8_t* tbl;
  } LookupMaster;

  const uint8_t lkpTbl1[] PROGMEM = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 86, 89, 92, 95, 99, 103, 107, 112, 117, 123, 129, 136, 143, 151, 159, 168, 177, 187, 197, 207, 222, 237, 252, };
  const uint8_t lkpTbl2[] PROGMEM = { 27, 29, 31, 33, 36, 38, 41, 44, 47, 51, 54, 58, 62, 66, 71, 75, 80, 85, 90, 96, 101, 107, 113, 119, 126, 132, 139, 146, 153, 161, 169, 177, 185, 195, 206, 218, 231, 245, };
  const uint8_t lkpTbl3[] PROGMEM = { 26, 28, 29, 31, 33, 35, 37, 40, 42, 44, 47, 50, 53, 55, 58, 62, 65, 68, 71, 75, 79, 82, 86, 90, 94, 98, 102, 107, 111, 115, 120, 125, 130, 134, 139, 145, 150, 156, 161, 167, 173, 180, 186, 193, 199, 206, 213, 221, 228, 235, 242, 249, };
  const uint8_t lkpTbl4[] PROGMEM = { 26, 26, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 36, 36, 37, 38, 39, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47, 47, 48, 49, 50, 51, 51, 52, 53, 54, 55, 55, 56, 57, 58, 59, 60, 61, 63, 66, 70, 76, 85, 97, 112, 132, 157, 187, 227, };
  const uint8_t lkpTbl5[] PROGMEM = { 28, 34, 42, 52, 64, 80, 100, 130, 180, 250, };
  const uint8_t lkpTbl6[] PROGMEM = { 37, 55, 83, };

  const LookupMaster luM[6] = { { 97, lkpTbl1}, { 38, lkpTbl2}, { 52, lkpTbl3}, { 56, lkpTbl4}, { 10, lkpTbl5}, { 3, lkpTbl6}, };


template <byte ADDRESS=0x44>
class Opt3001 : public Brightness {
  OPT3001_ERROR writeData(OPT3001_COMMANDS command) {
    Wire.beginTransmission(ADDRESS);
    Wire.write(command);
    return (OPT3001_ERROR)(-10 * Wire.endTransmission());
  }

  OPT3001_ERROR writeConfig(OPT3001_CONFIG config) {
    Wire.beginTransmission(ADDRESS);
    Wire.write(CONFIG);
    Wire.write(config.raw >> 8);
    Wire.write(config.raw & 0x00FF);
    return (OPT3001_ERROR)(-10 * Wire.endTransmission());
  }

  OPT3001_ERROR readData(uint16_t* data) {
    uint8_t	buf[2];
    Wire.requestFrom(ADDRESS, (uint8_t)2);

    uint8_t counter = 0;
    while (Wire.available() < 2) {
      counter++;
      delay(10);
      if (counter > 250) return OPT3001_ERROR::TIMEOUT;
    }

    Wire.readBytes(buf, 2);
    *data = (buf[0] << 8) | buf[1];

    return OPT3001_ERROR::NO_ERROR;
  }

  OPT3001_ERROR readDeviceID(uint16_t* data) {
    OPT3001_ERROR error = writeData(DEVICE_ID);
    if (error == NO_ERROR) error = readData(data);
    return error;
  }

  OPT3001_ERROR readRegister(OPT3001_COMMANDS command, uint16_t* data) {
    OPT3001_ERROR error = writeData(command);
    if (error == NO_ERROR) error = readData(data);
    return error;
  }


public:
  Opt3001() {}

  void init () {
    // init the sensor
    Wire.begin();
    uint16_t dev_id = 0;
    OPT3001_ERROR error = readRegister(DEVICE_ID, &dev_id);
    if (dev_id != 12289) return;
    DPRINTLN(F("opt3001 sensor found"));
    _present = true;

    // configure the sensor
    OPT3001_CONFIG conf = { 0 };
    conf.RangeNumber = B1100;               // automatic full range mode
    conf.ConvertionTime = B1;               // 100 ms or 800 ms
    //conf.Latch = B1;                        
    conf.ModeOfConversionOperation = B11;   // always on
    error = writeConfig(conf);
    if (error) { DPRINT(F("config error opt3001: ")); DPRINTLN(error); }

    // read config
    /*error = readRegister(CONFIG, &conf.raw);
    DPRINT("FaultCount: "); DPRINTLN(conf.FaultCount);
    DPRINT("MaskExponent: "); DPRINTLN(conf.MaskExponent);
    DPRINT("Polarity: "); DPRINTLN(conf.Polarity);
    DPRINT("Latch: "); DPRINTLN(conf.Latch);
    DPRINT("FlagLow: "); DPRINTLN(conf.FlagLow);
    DPRINT("FlagHigh: "); DPRINTLN(conf.FlagHigh);
    DPRINT("ConversionReady: "); DPRINTLN(conf.ConversionReady);
    DPRINT("OverflowFlag: "); DPRINTLN(conf.OverflowFlag);
    DPRINT("ModeOfConversionOperation: "); DPRINTLN(conf.ModeOfConversionOperation);
    DPRINT("ConvertionTime: "); DPRINTLN(conf.ConvertionTime);
    DPRINT("RangeNumber: "); DPRINTLN(conf.RangeNumber);*/
  }

  void measure (__attribute__((unused)) bool async=false) {
    OPT3001_RR rr;
    uint32_t lux;

    OPT3001_ERROR error = readRegister(RESULT, &rr.raw);
    if (error) {
      DPRINT(F("read error opt3001: ")); DPRINTLN(error);
      return;
    }
    uint16_t lsb = 1 << rr.Exponent;
    lux = lsb * rr.Result;
    //DPRINT(lux); DPRINTLN(F(" LUX100")); 
    //DPRINT("res: "); DPRINT(rr.Result); DPRINT(", exp: "); DPRINT(rr.Exponent); DPRINT(", ");
    _brightness = lux;
  }

  uint8_t bright2mdir() {
    return bright2mdir(_brightness);
  }
  uint8_t bright2mdir(uint32_t value) {
    uint32_t tVal, tDiv = 1;
    uint8_t tMdir = 0;

    for (uint8_t i = 0; i < 6; i++) {
      tVal = value / tDiv;
      tDiv *= 10;
      for (uint8_t j = 0; j < luM[i].len; j++) {
        if (tVal <= pgm_read_byte(&luM[i].tbl[j]) ) goto DONEALL;
        tMdir++;
      }
    }

  DONEALL:
    DPRINT(F("LUX100: ")); DPRINT(value); DPRINT(F(", MDIR: ")); DPRINTLN(tMdir);
    return tMdir;
  }
};

}

#endif
