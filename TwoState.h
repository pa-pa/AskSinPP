//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-10-19 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-12-28 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __TWOSTATE_H__
#define __TWOSTATE_H__

#include "ThreeState.h"

#ifndef SABOTAGE_ACTIVE_STATE
#define SABOTAGE_ACTIVE_STATE LOW
#endif

namespace as {

class OnePinPosition : public Position {
  uint8_t sens;
  uint8_t en;
public:
  OnePinPosition () : sens(0), en(0) { _present = true; }

  void init (uint8_t pin, uint8_t enpin) {
    sens=pin;
    en = enpin;
    if (en > 0) pinMode(en, OUTPUT);
  }

  void measure (__attribute__((unused)) bool async=false) {
    _position = ( AskSinBase::readPin(sens, en, 50) == 0) ? State::PosA : State::PosB;
  }
};


template <class HALTYPE,class List0Type,class List1Type,class List4Type,int PEERCOUNT>
class TwoStateChannel : public StateGenericChannel<OnePinPosition,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> {
public:
  typedef StateGenericChannel<OnePinPosition,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> BaseChannel;

  TwoStateChannel () : BaseChannel() {};
  ~TwoStateChannel () {}

  void init (uint8_t pin, uint8_t en, uint8_t sab) {
    BaseChannel::init(sab);
    BaseChannel::possens.init(pin, en);
  }

  void init (uint8_t pin, uint8_t sab) {
    BaseChannel::init(sab);
    BaseChannel::possens.init(pin, 0);
  }

  void init (uint8_t pin) {
    BaseChannel::init();
    BaseChannel::possens.init(pin, 0);
  }
};

}

#endif
