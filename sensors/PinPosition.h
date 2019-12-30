//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-10-19 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __PINPOSITION_H__
#define __PINPOSITION_H__


#include "Sensors.h"

namespace as {

template <uint16_t WAITMILLIS_AFTER_ENABLE=0>
class OnePinPosition : public Position {
  uint8_t sens;
  uint8_t en;
public:
  OnePinPosition () : sens(0), en(0) { _present = true; }

  void init (uint8_t pin, uint8_t enpin) {
    sens=pin;
    en = enpin;
    if (en != 0) pinMode(en, OUTPUT);
  }

  void measure (__attribute__((unused)) bool async=false) {
    _position = ( AskSinBase::readPin(sens, en, WAITMILLIS_AFTER_ENABLE) == 0) ? State::PosA : State::PosB;
  }
};

class TwoPinPosition : public Position {
  // pin mapping can be changed by bootloader config data
  // map pins to pos     00   01   10   11
  uint8_t posmap[4] = {State::PosC,State::PosC,State::PosB,State::PosA};
  uint8_t sens1, sens2;
public:
  TwoPinPosition () : sens1(0), sens2(0) { _present = true; }

  void init (uint8_t pin1,uint8_t pin2, const uint8_t* pmap) {
    memcpy(posmap,pmap,4);
    init(pin1,pin2);
  }

  void init (uint8_t pin1,uint8_t pin2) {
    sens1=pin1;
    sens2=pin2;
  }

  void measure (__attribute__((unused)) bool async=false) {
    uint8_t pinstate = AskSinBase::readPin(sens2) << 1 | AskSinBase::readPin(sens1);
    _position = posmap[pinstate & 0x03];
  }
};

}

#endif
