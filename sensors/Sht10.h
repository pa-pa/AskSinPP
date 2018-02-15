
#ifndef __SENSORS_SHT10_h__
#define __SENSORS_SHT10_h__

#include <Sensors.h>
#include <Sensirion.h>

namespace as {

// https://github.com/spease/Sensirion.git
template <int DATAPIN,int CLOCKPIN>
class Sht10 : public Temperature, public Humidity {
  Sensirion _sht10;

  void stopWire () {
    if( DATAPIN == A4 && CLOCKPIN==A5 ) {
#if defined(TwoWire_h) || defined(_WIRE_H_)
      Wire.end();
#endif
    }
  }
  void startWire () {
    if( DATAPIN == A4 && CLOCKPIN==A5 ) {
#if defined(TwoWire_h) || defined(_WIRE_H_)
      Wire.begin();
#endif
    }
  }

public:
  Sht10 () : _sht10(DATAPIN,CLOCKPIN) {}

  void init () {
    stopWire();
    _present = (_sht10.writeSR(LOW_RES) == 0);
    startWire();
  }
  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      uint16_t tdata,hdata;
      stopWire();
      if ( _sht10.measTemp(&tdata)== 0) {
        if( _sht10.measHumi(&hdata)== 0 ) {
          float t = _sht10.calcTemp(tdata);
          _temperature = t * 10;
          _humidity = _sht10.calcHumi(hdata,t);
        }
      }
      startWire();
    }
  }
};


}

#endif
