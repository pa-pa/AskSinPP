//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_SHT10_h__
#define __SENSORS_SHT10_h__

#include <Sensors.h>
#include <Sensirion.h>

namespace as {

enum Sht10Mode { LOWRES_INT=0x01, LOWRES_FLOAT=0x02, HIGHRES=0x03 };

// https://github.com/spease/Sensirion.git
template <uint8_t DATAPIN,uint8_t CLOCKPIN,Sht10Mode MODE=Sht10Mode::LOWRES_INT>
class Sht10 : public Temperature, public Humidity {
  Sensirion _sht10;

  void stopWire () {
#if defined(TwoWire_h) || defined(_WIRE_H_)
    if( DATAPIN == A4 && CLOCKPIN==A5 ) {
      Wire.end();
    }
#endif
  }

  void startWire () {
    digitalWrite(CLOCKPIN,HIGH);
#if defined(TwoWire_h) || defined(_WIRE_H_)
    if( DATAPIN == A4 && CLOCKPIN==A5 ) {
      Wire.begin();
    }
#endif
  }

  bool meas(uint16_t& tdata,uint16_t& hdata) {
    bool success = false;
    stopWire();
    success = (_sht10.measTemp(&tdata) == 0) && (_sht10.measHumi(&hdata) == 0);
    startWire();
    return success;
  }

public:

  Sht10 () : _sht10(DATAPIN,CLOCKPIN) {}

  void init () {
    stopWire();
    uint8_t sr = MODE == Sht10Mode::HIGHRES ? 0 : LOW_RES;
    _present = (_sht10.writeSR(sr) == 0);
    startWire();
  }

  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      uint16_t tdata,hdata;
      if( meas(tdata,hdata) == true ) {
        if( MODE != Sht10Mode::LOWRES_INT ) {
          float t = _sht10.calcTemp(tdata);
          _temperature = t * 10;
          _humidity = _sht10.calcHumi(hdata,t);
        }
        else {
          // LOW_RES: temp = -40.1 + 0.04 * (float) tdata;
          //          temp = ((4*tdata) - 4010) / 100
          int32_t tt = ((4L * tdata) - 4010);
          // DDEC(_temperature);DPRINT(" <> ");DDECLN(tt/10);
          // humi = C1 + C2l * rawData + C3l * rawData * rawData;
          // humi = (temp - 25.0) * (T1 + T2l * rawData) + humi;
          // LOW_RES: humi = -2.0468 + 0.5872 * hdata + -4.0845E-4 * hdata * hdata;
          //          humi = (temp - 25.0) * (0.01 + 0.00128 * hdata) + humi;
          //          humi = -20468 + 5872 * hdata + -4.0845 * hdata * hdata;
          //          humi = ((temp*100 - 2500) * (100 + 12.8 * hata) / 100) + humi;
          //          humi = huni / 10000
          int32_t hh = ((5872L * hdata) - 20468L) - (4L * hdata * hdata);
          hh = (((tt - 2500) * (100L + (12L * hdata)) / 100) + hh) / 10000;
          hh = hh > 100 ? 100 : hh;
          hh = hh < 0 ? 0 : hh;
          // DDEC(_humidity);DPRINT(" <> ");DDECLN(hh);
          _temperature = tt / 10;
          _humidity = hh;
        }
      }
    }
  }

};

}

#endif
