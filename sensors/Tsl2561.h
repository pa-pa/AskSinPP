//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_TSL2561_h__
#define __SENSORS_TSL2561_h__

#include <Sensors.h>
#include <Wire.h>
#include <Adafruit_TSL2561_U.h>

namespace as {

// https://github.com/adafruit/Adafruit_TSL2561
template <int ADDRESS=TSL2561_ADDR_LOW>
class Tsl2561 : public Brightness {
  ::Adafruit_TSL2561_Unified   _tsl;
  bool                         _x16;
public:
  Tsl2561 () : _tsl(ADDRESS), _x16(true) {}
  void init () {
    if( (_present = _tsl.begin()) == true ) {
      _tsl.setGain(TSL2561_GAIN_16X);
      _tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);
      DPRINT("TSL2561 found at 0x");DHEXLN((uint8_t)ADDRESS);
    }
    else {
      DPRINTLN("No TSL2561 present");
    }
  }
  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      uint16_t b = 0;
      _tsl.getLuminosity(&b, NULL);
      if( b > 63000 && _x16 == true ) {
        _x16 = false;
        _tsl.setGain(TSL2561_GAIN_1X);
        _tsl.getLuminosity(&b, NULL);
      }
      else if ( b < 500 && _x16 == false ) {
        _x16 = true;
        _tsl.setGain(TSL2561_GAIN_16X);
        _tsl.getLuminosity(&b, NULL);
      }
      DPRINT("Brightness: ");DDECLN(b);
      _brightness = b;
    }
  }
};

}

#endif
