//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-01-07 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef SENSORS_H_
#define SENSORS_H_

namespace as {

class BrightnessFake {
public:
  BrightnessFake () {}
  void init () {}
  bool present () { return false; }
  uint16_t brightness () { return 0; }
};

#ifdef _TSL2561_H_

template <int ADDRESS=TSL2561_ADDR_LOW>
class BrightnessTSL2561 {
  TSL2561 tsl;
  bool    present;
public:
  BrightnessTSL2561 () : tsl(ADDRESS), present(false) {}
  void init () {
    if( (present = tsl.begin()) == true ) {
      tsl.setGain(TSL2561_GAIN_0X);
      tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);
    }
  }

  uint16_t brightness () {
    if( present == false ) {
      return 0;
    }
    uint16_t bright = tsl.getLuminosity(TSL2561_VISIBLE);
    // TODO which gain on very high values ???
    return bright;
  }
};

#endif

}

#endif
