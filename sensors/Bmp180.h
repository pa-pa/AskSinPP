
#ifndef __SENSORS_BMP180_h__
#define __SENSORS_BMP180_h__

#include <Sensors.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

namespace as {

// https://github.com/adafruit/Adafruit-BMP085-Library
class Bmp180 : public Temperature, public Pressure {
  Adafruit_BMP085 _bmp;
public:
  Bmp180 () {}
  void init () {
    _present = _bmp.begin();
  }
  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      _temperature = _bmp.readTemperature() * 10;
      _pressure = _bmp.readPressure() / 100;
    }
  }
};

}

#endif
