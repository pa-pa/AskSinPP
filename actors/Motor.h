//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-08-10 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <Arduino.h>

namespace as {

class Motor {
public:
  Motor () {}
  ~Motor () {}

  void init () {}
  void up () {}
  void down () {}
  void stop () {}

};


template<uint8_t DIR_RELAY_PIN,uint8_t ON_RELAY_PIN>
class TwoRelayMotor {
public:
  TwoRelayMotor () {}
  ~TwoRelayMotor () {}

  void init () {
    pinMode(DIR_RELAY_PIN,OUTPUT);
    pinMode(ON_RELAY_PIN,OUTPUT);
    stop();
  }

  void up () {
    digitalWrite(DIR_RELAY_PIN,HIGH);
    digitalWrite(ON_RELAY_PIN,HIGH);
  }

  void down () {
    digitalWrite(DIR_RELAY_PIN,LOW);
    digitalWrite(ON_RELAY_PIN,HIGH);
  }

  void stop () {
    digitalWrite(DIR_RELAY_PIN,LOW);
    digitalWrite(ON_RELAY_PIN,LOW);
  }

};

}

#endif
