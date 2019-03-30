/***************************************************
 This is a library for the Adafruit VEML6070 UV Sensor Breakout
 
 Designed specifically to work with the VEML6070 sensor from Adafruit
 ----> https://www.adafruit.com/products/2899
 
 These sensors use I2C to communicate, 2 pins are required to
 interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!
 
 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, all text above must be included in any redistribution
 ****************************************************/
//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2018-07-04 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_VEML6070_h__
#define __SENSORS_VEML6070_h__

#define VEML6070_ADDR_H 0x39
#define VEML6070_ADDR_L 0x38

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Sensors.h>
#include <Wire.h>

typedef enum veml6070_integrationtime {
    VEML6070_HALF_T,
    VEML6070_1_T,
    VEML6070_2_T,
    VEML6070_4_T,
} veml6070_integrationtime_t;

namespace as {

template <uint8_t INTEGRATION_TIME=VEML6070_1_T>
class Veml6070 : public Sensor {
  uint16_t  _uvvalue;
  uint8_t   _uvindex;
public:
  Veml6070 ()  {}
    
  uint16_t readUV() {
        if (Wire.requestFrom(VEML6070_ADDR_H, 1) != 1) return -1;
        uint16_t uvi = Wire.read();
        uvi <<= 8;
        if (Wire.requestFrom(VEML6070_ADDR_L, 1) != 1) return -1;
        uvi |= Wire.read();
        return uvi;
  }
  
  void init () {
      Wire.begin();
      Wire.beginTransmission(VEML6070_ADDR_L);
      Wire.write((INTEGRATION_TIME << 2) | 0x02);
      Wire.endTransmission();
      delay(500);
      _present = (readUV() < 65535);
  }
  
  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      _uvvalue = readUV();
      if (INTEGRATION_TIME == VEML6070_HALF_T) {
        DPRINTLN(F("UV Index calc. with VEML6070_HALF_T not supported"));
        return;
      }
      uint8_t _integration_time_factor = (INTEGRATION_TIME == VEML6070_4_T) ? 4 : INTEGRATION_TIME;
      _uvindex = (_uvvalue < (12 * (187 * _integration_time_factor))) ? (_uvvalue + 1) / (187 * _integration_time_factor) : 11;
    }
  }
  uint16_t UVValue ()  { return _uvvalue; }
  uint8_t UVIndex ()   { return _uvindex; }
};

}

#endif
