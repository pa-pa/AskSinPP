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

template <uint8_t SENSEPIN,uint32_t R0=10000,uint16_t B=3435,uint8_t ACTIVATEPIN=0,int8_t T0=25,uint8_t OVERSAMPLING=0>
class Ntc : public Temperature {
  const int32_t _t0Abs = T0 * 10 + 2732;
  const int32_t _max_ref = (1 << (10 + OVERSAMPLING)) - 10 - OVERSAMPLING;

public:
  Ntc () {}

  void init () {
    pinMode(SENSEPIN, INPUT);
  }

  bool measure (__attribute__((unused)) bool async=false) {
    uint32_t vo = 0;

    if (ACTIVATEPIN) {
      pinMode(ACTIVATEPIN, OUTPUT);
      digitalWrite(ACTIVATEPIN, HIGH);
    }

    for (uint16_t i = 0; i < 1 << (OVERSAMPLING * 2); i++) {
      vo += analogRead(SENSEPIN);
    }

    if (ACTIVATEPIN) {
      digitalWrite(ACTIVATEPIN, LOW);
    }

    if (OVERSAMPLING) {
      vo = vo >> OVERSAMPLING;
    }

    vo = _max_ref - vo;

    float rNtc = vo / (float)(_max_ref - vo);
    _temperature = 10 * B * _t0Abs / (10 * B + (int16_t)(log(rNtc) * _t0Abs)) - 2732;

    return true;
  }
};
}

#endif
