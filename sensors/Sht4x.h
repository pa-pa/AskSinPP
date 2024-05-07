//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2024-05-04 psi-4ward Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_SHT4x_h__
#define __SENSORS_SHT4x_h__

#include <Sensors.h>
#include <Wire.h>
#include <SensirionI2cSht4x.h>

namespace as {

// https://github.com/Sensirion/arduino-i2c-sht4x
template <uint8_t ADDRESS=0x44>
class Sht4x : public Temperature, public Humidity {
  SensirionI2cSht4x _sht4x;
  char     errorMessage[256];
public:
  Sht4x () {}

  void init () {
    uint16_t error;
    uint32_t serialNumber;
    DPRINTLN(F("SHT4x: Starting Serial Monitor ..."));
    _sht4x.begin(Wire, ADDRESS);

    DPRINTLN(F("SHT4x: Trying to get serial number.."));
    error = _sht4x.serialNumber(serialNumber);
    DPRINTLN(F("SHT4x: Getting serial number finished .."));
    if (error) {
      DPRINTLN(F("SHT4x sensor NOT found"));
      DPRINTLN(F("SHT4x: Error trying to execute serialNumber(): "));
      errorToString(error, errorMessage, 256);
      DPRINTLN(errorMessage);
      _present = false;
    } else {
      DPRINTLN(F("SHT4x sensor OK"));
      DPRINT(F("SHT4x Serial Number: "));
      DPRINTLN(serialNumber);
      _present = true;
    }
  }

  bool measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      float    temp(NAN), hum(NAN);
      uint16_t error;
      // read SHT4x measurement values
      error = _sht4x.measureHighPrecision(temp, hum);
     // check for errors
      if (error) {
        DPRINTLN(F("SHT4x: Error trying to execute measureHighPrecision(): "));
        errorToString(error, errorMessage, 256);
        DPRINTLN(errorMessage);
      } else {
        _temperature           = (int16_t)(round(temp * 10.0));
        _humidity              = (uint16_t)(round(hum));
        return true;
      }
    }
    return false;
  }
};

}
#endif
