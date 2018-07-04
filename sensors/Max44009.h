//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2018-07-04 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// https://github.com/jp112sdl/Arduino/tree/master/libraries/Max44009

#ifndef __SENSORS_MAX44009_h__
#define __SENSORS_MAX44009_h__

#include <Sensors.h>
#include <Wire.h>
#include <Max44009.h>

namespace as {

template <byte ADDRESS=0x4a, uint8_t CDR=0x01, uint8_t TIM=0x03>
class MAX44009 : public Brightness {
    ::Max44009 _max44009;
public:
  MAX44009 () : _max44009(ADDRESS) {}
  void init () {
      _max44009.getLux();
      uint8_t err = _max44009.getError();
      if (err == 0) {
        _present = true;
        _max44009.setContinuousMode();
        _max44009.setManualMode(CDR, TIM);
        DPRINTLN(F("MAX44009 Sensor OK"));
      } else {
          DPRINT(F("MAX44009 Sensor Error "));DDECLN(err);
      }
  }
  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      _brightness = _max44009.getLux() / 100;
      uint8_t err = _max44009.getError();
      if (err != 0) {
        DPRINT("MAX44009 Sensor Error ");DDECLN(err);
      }
    }
  }
};

}

#endif
