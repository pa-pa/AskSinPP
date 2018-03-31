
#ifndef __SENSORS_DHT11_h__
#define __SENSORS_DHT11_h__

#include <Sensors.h>
#include <DHT.h>

namespace as {
    
    // https://github.com/adafruit/DHT-sensor-library
    template <int DATAPIN,int TYPE=DHT11, uint8_t max_retries=0>
    class Dht : public Temperature, public Humidity {
        DHT _dht;
        
    public:
        Dht () : _dht(DATAPIN,TYPE) {}
        
        void init () {
            _dht.begin();
            _present = true;
        }
        bool measure (__attribute__((unused)) bool async=false) {
            if( present() == true ) {
                uint8_t retries = 0;
                float t = _dht.readTemperature(false);
                float h = _dht.readHumidity();
                
                if ((isnan(t) || isnan(h)) && max_retries == 0) {
                    return false;
                }
                
                while ((isnan(t) || isnan(h)) && retries < max_retries)
                {
                    retries++;
                    DPRINT(F("DHT measure failure. Retry #"));DDEC(retries);DPRINT(F(" of "));DDEC(max_retries);DPRINTLN(F(", Trying again..."));
                    t = _dht.readTemperature(false);
                    h = _dht.readHumidity();
                    delay(500);
                }
                
                _temperature = t * 10;
                _humidity = h;
                return true;
            }
        }
    };
}

#endif
