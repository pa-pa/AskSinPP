//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-11-02 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2020-01-03 manawyrm
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_SHT21_h__
#define __SENSORS_SHT21_h__

#include <Sensors.h>
#include <Wire.h>
#include <SHT21.h>

namespace as {

//https://github.com/manawyrm/SHT21
template <uint8_t ADDRESS=0x40>
class Sht21 : public Temperature, public Humidity {
  SHT21 _sht21;
public:
  Sht21 () {}

  void init () {
    _present = _sht21.begin(ADDRESS);
    DPRINT(F("SHT21 "));
    if (_present) {
      DPRINTLN(F("OK"));
    } else {
      DPRINTLN(F("ERROR"));
    }
  }

  bool measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
        if (_sht21.startMeasurement()) 
        {
            _temperature = _sht21.readTemperature();
            _humidity = _sht21.readHumidity();
            return true;
        }
    }
    return false;
  }
};

}

#endif
