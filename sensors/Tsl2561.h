//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_TSL2561_h__
#define __SENSORS_TSL2561_h__

#include <Sensors.h>
#include <Wire.h>
#include <TSL2561.h>

namespace as {

// https://github.com/adafruit/TSL2561-Arduino-Library
template <int ADDRESS=TSL2561_ADDR_LOW>
class Tsl2561 : public Brightness {
  ::TSL2561   _tsl;
  bool        _x16;
public:
  Tsl2561 () : _tsl(ADDRESS), _x16(true) {}
  void init () {
    if( (_present = _tsl.begin()) == true ) {
      _tsl.setGain(TSL2561_GAIN_16X);
      _tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);
    }
  }
  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      uint16_t b = _tsl.getLuminosity(TSL2561_VISIBLE);
      if( b > 63000 && _x16 == true ) {
        _x16 = false;
        _tsl.setGain(TSL2561_GAIN_0X);
        b = _tsl.getLuminosity(TSL2561_VISIBLE);
      }
      else if ( b < 500 && _x16 == false ) {
        _x16 = true;
        _tsl.setGain(TSL2561_GAIN_16X);
        b = _tsl.getLuminosity(TSL2561_VISIBLE);
      }
      _brightness = b;
    }
  }
};

}

#endif
