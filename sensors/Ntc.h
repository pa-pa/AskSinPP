//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_NTC_h__
#define __SENSORS_NTC_h__

#include <Sensors.h>

namespace as {

/*
 * NTC needs to be connected like shown here: http://playground.arduino.cc/uploads/ComponentLib/simple10ktherm_schem_new.png

 * SENSEPIN is the analog pin
 * ACTIVATEPIN is a output pin to power up the NTC only while measuring. Use 0 if the NTC is connected to VCC and not to a pin.
 * R0 is the resitance both of the NTC and of the second resistor
 * B is a material based number of the NTC. Look it up in the datasheet of the NTC. E.g. the ATC 103AT-2 has a B of 3435
 * T0 is the temperature where the NTC has the resistance R0. In most cases this is 25°C.
 * OVERSAMPLING are the additional oversampled bits, e.g. choosing 2 will increase the internal sample size of the ATmega ADC from 10 bit to 12 bit.
 */

template <uint8_t SENSEPIN,int R0=10000,int B=3435,int ACTIVATEPIN=0,int T0=25,int OVERSAMPLING=0>
class Ntc : public Temperature {
  float _t0Abs;
  
public:
  Ntc () : _t0Abs((float)T0 + 273.15) {}

  void init () {
  }

  bool measure (__attribute__((unused)) bool async=false) {
    int vo = 0;

    if(ACTIVATEPIN != 0) {
      pinMode(ACTIVATEPIN, OUTPUT);
      digitalWrite(ACTIVATEPIN, HIGH);
    }

    for (int i = 0; i < (1 << (OVERSAMPLING * 2)); i++) {
      vo += analogRead(SENSEPIN);
    }

    if (OVERSAMPLING > 0) {
      vo = vo >> OVERSAMPLING;
    }

    if(ACTIVATEPIN != 0) {
      digitalWrite(ACTIVATEPIN, LOW);
    }
    
    float rNtc = R0 * (((1 << (10 + OVERSAMPLING)) - 1) / (float)vo - 1);

    _temperature = (_t0Abs * B / (B + _t0Abs * log(rNtc / R0)) - 273.15) * 10;

    return true;
  }
};
}

#endif
