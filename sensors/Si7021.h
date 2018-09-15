//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2018-09-13 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_SI7021_h__
#define __SENSORS_SI7021_h__

#include <Sensors.h>
#include <Wire.h>
#include <Si7021.h>
namespace as {

//https://github.com/jayjayuk/Si7021-Humidity-And-Temperature-Sensor-Library
class Si7021 : public Temperature, public Humidity {
  SI7021 _si7021;
public:
  Si7021 () {}
  void init () {
    _si7021.begin();
    DPRINT(F("SI7021 SENSOR "));
    if (_si7021.getDeviceID() > 0 && _si7021.getDeviceID() != 255) {
      _present = true;
       DPRINTLN(F("OK"));
    } else {
      _present = false;
      DPRINTLN(F("ERR"));
    }
  }
    
  bool measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      _temperature = _si7021.readTemp() * 10;
      _humidity = _si7021.readHumidity();
      return true;
    }
    return false;
  }
};

}

#endif
