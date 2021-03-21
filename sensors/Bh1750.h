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
template <byte ADDRESS=0x23,::BH1750::Mode MODE=::BH1750::Mode::ONE_TIME_HIGH_RES_MODE>
class Bh1750 : public Brightness {
  ::BH1750   _bh;
private:
  uint8_t numRetry;

  #ifdef BH1750_AUTOCALIBRATE
  bool waitForMeasurementReady() {
    for (int i=0; i<360; i++) {
      if (_bh.measurementReady())
        return true;
      delay(1);
    }
    return false;
  }

  float doMeasurement()
  {
    waitForMeasurementReady();
    float lux = _bh.readLightLevel();

    if (lux < 0)
    {
      if (numRetry == 10)
        return 0.0;

       numRetry++;
      doMeasurement();
    }

    numRetry = 0;
    return lux;
  }
  #endif

public:
  Bh1750 () : _bh(ADDRESS) {}
  void init () {
      _present = _bh.begin(MODE);
      numRetry = 0;
  }

  void measure (__attribute__((unused)) bool async=false) {
    if( present() ) {
      #ifdef BH1750_AUTOCALIBRATE
      float lux = doMeasurement();
      if (lux > 40000.0 )
        _bh.setMTreg(32);
      else if (lux > 10.0)
        _bh.setMTreg(69);
      else if (lux <= 10.0)
        _bh.setMTreg(138);

      _brightness = (uint32_t)doMeasurement();
      #else
      _brightness = (uint32_t)_bh.readLightLevel();
      #endif
    }
  }
};

}

#endif
