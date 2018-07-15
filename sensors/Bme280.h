//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_BME280_h__
#define __SENSORS_BME280_h__

#include <Sensors.h>
#include <Wire.h>
#include <BME280I2C.h>

namespace as {

// https://github.com/finitespace/BME280
class Bme280 : public Temperature, public Pressure, public Humidity {
  BME280I2C _bme;
public:
  Bme280 () {}
  void init () {
    _present = _bme.begin();
  }
  bool measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      float t, p, h;
      _bme.read(p,t,h);
      _temperature = t * 10;
      _pressure = p;
      _humidity = h;
      return true;
    }
    return false;
  }
};

}

#endif
