//
// Main Parts are from
//  AUTHOR: Rob dot Tillaart at gmail dot com
// VERSION: 0.1.9
// PURPOSE: library for MAX44009 lux sensor Arduino
// HISTORY: See Max440099.cpp
//
// Released to the public domain
//
//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2018-07-04 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_MAX44009_h__
#define __SENSORS_MAX44009_h__

#include <Sensors.h>
#include <Wire.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// REGISTERS
#define MAX44009_CONFIGURATION      0x02
#define MAX44009_LUX_READING_HIGH   0x03
#define MAX44009_LUX_READING_LOW    0x04

// CONFIGURATION MASKS
#define MAX44009_CFG_CONTINUOUS     0x80
#define MAX44009_CFG_MANUAL         0x40
#define MAX44009_CFG_CDR            0x08
#define MAX44009_CFG_TIMER          0x07

namespace as {

template <byte ADDRESS=0x4a, uint8_t CDR=0x01, uint8_t TIM=0x03>
class MAX44009 : public Brightness {
public:
  void init () {
      Wire.begin();
      getLux();
      uint8_t err = getError();
      if (err == 0) {
        _present = true;
        setContinuousMode();
        setManualMode(CDR, TIM);
        DPRINTLN(F("MAX44009 Sensor OK"));
      } else {
          DPRINT(F("MAX44009 Sensor Error "));DDECLN(err);
      }
  }
  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      _brightness = getLux() / 100;
      uint8_t err = getError();
      if (err != 0) {
        //DPRINT("MAX44009 Sensor Error ");DDECLN(err);
      }
    }
  }
private:
    uint8_t _error;
    uint8_t _data;
    uint32_t getLux(void) {
        uint8_t dhi = read(MAX44009_LUX_READING_HIGH);
        uint8_t dlo = read(MAX44009_LUX_READING_LOW);
        uint8_t e = dhi >> 4;
        uint32_t m = ((dhi & 0x0F) << 4) + (dlo & 0x0F);
        m <<= e;
        uint32_t val = m * 45;
        return val;
    }
    uint8_t getError() {
        uint8_t e = _error;
        _error = 0;
        return e;
    }
    void setConfiguration(const uint8_t value) {
        write(MAX44009_CONFIGURATION, value);
    }
    
    uint8_t getConfiguration() {
        return read(MAX44009_CONFIGURATION);
    }
    
    void setAutomaticMode() {
        uint8_t config = read(MAX44009_CONFIGURATION);
        config &= ~MAX44009_CFG_CONTINUOUS; // off
        config &= ~MAX44009_CFG_MANUAL;     // off
        write(MAX44009_CONFIGURATION, config);
    }
    
    void setContinuousMode() {
        uint8_t config = read(MAX44009_CONFIGURATION);
        config |= MAX44009_CFG_CONTINUOUS; // on
        config &= ~MAX44009_CFG_MANUAL;    // off
        write(MAX44009_CONFIGURATION, config);
    }
    
    void setManualMode(uint8_t _CDR, uint8_t _TIM) {
        if (_CDR != 0) _CDR = 1;
        if (_TIM > 7) _TIM = 7;
        uint8_t config = read(MAX44009_CONFIGURATION);
        config &= ~MAX44009_CFG_CONTINUOUS; // off
        config |= MAX44009_CFG_MANUAL;      // on
        config &= 0xF0; // clear CDR & TIM bits
        config |= _CDR << 3 | TIM;
        write(MAX44009_CONFIGURATION, config);
    }
    
    uint8_t read(uint8_t reg) {
        Wire.beginTransmission(ADDRESS);
        Wire.write(reg);
        _error = Wire.endTransmission();
        if (_error != 0) {
            return _data; // last value
        }
        if (Wire.requestFrom(ADDRESS, (uint8_t) 1) != 1) {
            _error = 10;
            return _data; // last value
        }
#if (ARDUINO <  100)
        _data = Wire.receive();
#else
        _data = Wire.read();
#endif
        return _data;
    }
    
    void write(uint8_t reg, uint8_t value) {
        Wire.beginTransmission(ADDRESS);
        Wire.write(reg);
        Wire.write(value);
        _error = Wire.endTransmission();
    }
};

}

#endif
