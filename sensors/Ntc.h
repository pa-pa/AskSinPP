//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_NTC_h__
#define __SENSORS_NTC_h__

#include <Sensors.h>

namespace as {

template <int SENSEPIN,int R0=10000,int B=3435,int ACTIVATEPIN=0,int T0=25>
class Ntc : public Temperature {
  float _b;
  float _t0Abs;
  float _r0;
  
public:
  Ntc () : _t0Abs((float)T0 + 273.15), _r0(R0), _b(B) {}

  void init () {
  }

  bool measure (__attribute__((unused)) bool async=false) {
    int vo;

    if (ACTIVATEPIN != 0) {
      pinMode(ACTIVATEPIN, OUTPUT);
      digitalWrite(ACTIVATEPIN, HIGH);
    }

    vo = analogRead(SENSEPIN);

    if (ACTIVATEPIN != 0) {
      digitalWrite(ACTIVATEPIN, LOW);
    }

    float rNtc = _r0 * (1023.0 / (float)vo - 1.0);

    _temperature = (_t0Abs * _b / (_b + _t0Abs * log(rNtc / _r0))-273.15) * 10;

    return true;
  }
};
}

#endif
