//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_BH1750_h__
#define __SENSORS_BH1750_h__

#include <Sensors.h>
#include <Wire.h>
#include <BH1750.h>

namespace as {

// https://github.com/claws/BH1750
template <byte ADDRESS=0x23,::BH1750::Mode MODE=::BH1750::Mode::CONTINUOUS_HIGH_RES_MODE>
class Bh1750 : public Brightness {
  ::BH1750   _bh;
public:
  Bh1750 () : _bh(ADDRESS) {}
  void init () {
      _present = _bh.begin(MODE);
  }
  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      _brightness = _bh.readLightLevel();
    }
  }
};

}

#endif
