//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2018-11-14 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_VL53L0X_h__
#define __SENSORS_VL53L0X_h__

#include <Sensors.h>
#include <Wire.h>
#include <VL53L0X.h>

namespace as {

enum Precision {
   HIGH_ACCURACY,
   HIGH_SPEED
};

enum RangeMode {
    SHORT_RANGE,
    LONG_RANGE
};

//https://github.com/pololu/vl53l0x-arduino
template <uint8_t ADDR=0x29, uint16_t TIMEOUT=500, uint8_t RANGEMODE=RangeMode::SHORT_RANGE, uint8_t PRECISION= Precision::HIGH_ACCURACY>
class Vl53l0x : public Sensor {
  VL53L0X  _vl53l0x;
  uint16_t _distance_mm;
public:
  Vl53l0x () {}
  void init() {
    Wire.begin();
    _vl53l0x.setAddress(ADDR);

    _present = _vl53l0x.init();
    
    _vl53l0x.setTimeout(TIMEOUT);

   	DPRINTLN(F("VL53L0X SENSOR "));
    //DPRINT(F("- RANGEMODE = "));
    if (RANGEMODE == LONG_RANGE) {
      _vl53l0x.setSignalRateLimit(0.1);
      _vl53l0x.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
      _vl53l0x.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
      //DPRINTLN(F("LONG"));
    } else {
      //DPRINTLN(F("SHORT"));
    }

    //DPRINT(F("- PRECISION = "));
    if (PRECISION == HIGH_SPEED) {
      _vl53l0x.setMeasurementTimingBudget(20000);
      //DPRINTLN(F("SPEED"));
    } 
    
    if (PRECISION == HIGH_ACCURACY) {
      _vl53l0x.setMeasurementTimingBudget(200000);
      //DPRINTLN(F("ACCURACY"));
    }

   	if (_present) DPRINTLN(F("- OK")); else DPRINTLN(F("- ERROR"));
  }
    
  bool measure(__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      _distance_mm = _vl53l0x.readRangeSingleMillimeters(); 
      DPRINT("VL53L0X measure distance = ");DDECLN(_distance_mm);
      bool timeOut = _vl53l0x.timeoutOccurred();
      if (timeOut) DPRINTLN(F("VL53L0X measure timeout occured"));
      return !(timeOut);
    }
    return false;
  }
  
  uint16_t DistanceMM ()  { return _distance_mm; }
};

}

#endif
