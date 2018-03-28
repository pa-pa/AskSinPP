
#ifndef __SENSORS_DHT11_h__
#define __SENSORS_DHT11_h__

#include <Sensors.h>
#include <DHT.h>

namespace as {

// https://github.com/adafruit/DHT-sensor-library
template <int DATAPIN,int TYPE=DHT11>
class Dht : public Temperature, public Humidity {
  DHT _dht;

public:
  Dht () : _dht(DATAPIN,TYPE) {}

  void init () {
    _dht.begin();
    _present = true;
  }
  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
        float t = _dht.readTemperature(false);
        float h = _dht.readHumidity();
        while (isnan(t) || isnan(h))
        {
            DPRINTLN("DHT measure failure. Trying again...");
            t = _dht.readTemperature(false);
            h = _dht.readHumidity();
            delay(500);
        }
        _temperature = t * 10;
        _humidity = h;
    }
  }
};
}

#endif
