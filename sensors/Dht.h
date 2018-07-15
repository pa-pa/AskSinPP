//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_DHT11_h__
#define __SENSORS_DHT11_h__

#include <Sensors.h>
#include <DHT.h>

namespace as {

// https://github.com/adafruit/DHT-sensor-library
template <int DATAPIN,int TYPE=DHT11,uint8_t MAXMEASURE=1>
class Dht : public Temperature, public Humidity {
  DHT _dht;
public:
  Dht () : _dht(DATAPIN,TYPE) {}

  void init () {
    _dht.begin();
    _present = true;
  }
  bool measure (__attribute__((unused)) bool async=false) {
    bool success = false;
    if( present() == true ) {
      uint8_t measure=MAXMEASURE;
      while( success == false && measure > 0 ) {
        --measure;
        float t = _dht.readTemperature(false,true);
        float h = _dht.readHumidity();
        if( isnan(t) == false && isnan(h) == false ) {
          _temperature = t * 10;
          _humidity = h;
          success = true;
        }
        else if( measure > 0 ) {
          _delay_ms(500);
        }
      }
    }
    return success;
  }
};

}

#endif
