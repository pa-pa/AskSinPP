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

BME280I2C::Settings settings(
   BME280::OSR_X1, // Temperature Oversampling Rate (tempOSR): OSR Enum, default = OSR_X1
   BME280::OSR_X1, // Humidity Oversampling Rate (humOSR): OSR Enum, default = OSR_X1
   BME280::OSR_X1, // Pressure Oversampling Rate (presOSR): OSR Enum, default = OSR_X1
   BME280::Mode_Forced, // Mode (mode): Mode Enum, default = Mode_Forced
   BME280::StandbyTime_1000ms, // Standby Time (standbyTime): StandbyTime Enum, default = StandbyTime_1000ms
   BME280::Filter_Off, // Filter (filter): Filter Enum, default = Filter_16
   BME280::SpiEnable_False // SPI Enable: SpiEnable Enum, default = false
);

// https://github.com/finitespace/BME280
class Bme280 : public Temperature, public Pressure, public Humidity {
  BME280I2C _bme;
public:
  Bme280 () {}
  void init () {
    Wire.begin();
    _present = _bme.begin();
    _bme.setSettings(settings);
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
