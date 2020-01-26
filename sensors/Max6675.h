//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2020-01-09 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef MAX6675_H_
#define MAX6675_H_

#include <Sensors.h>

namespace as {

template <uint8_t SCK, uint8_t CS, uint8_t SO>
class MAX6675 : public Temperature {
  private:
    uint8_t spiread() {
      uint8_t d = 0;
      for (uint8_t i = 7; i >= 0; i--) {
        digitalWrite(SCK, LOW);
        _delay_ms(1);
        if (digitalRead(SO)) {
          d |= (1 << i);
        }
        digitalWrite(SCK, HIGH);
        _delay_ms(1);
      }
      return d;
    }

    uint16_t readCelsius() {
      uint16_t v;

      digitalWrite(CS, HIGH);
      _delay_ms(750);
      digitalWrite(CS, LOW);
      _delay_ms(1);

      v = spiread();
      v <<= 8;
      v |= spiread();

      if (v & 0x4) {
        DPRINTLN(F("thermocouple is unconnected"));
        return 0xFFFF;
      }

      v >>= 3;
      //DPRINT("C ");DDECLN(v*0.25);

      return v * 0.25;
    }

  public:
    MAX6675 () {}

    bool init () {
      pinMode(SO,  INPUT);
      pinMode(CS,  OUTPUT);
      pinMode(SCK, OUTPUT);

      _present = (readCelsius() != 0xFFFF);
      return _present;
    }

    bool measure (__attribute__((unused)) bool async = false) {
      if ( present() == true ) {
        uint16_t t = readCelsius();
        if (t != 0xFFFF) {
          _temperature = t * 10;
          return true;
        } else {
          return false;
        }
      }
      return false;
    }

    int16_t temperature() {
      return _temperature;
    }
};

}

#endif /* MAX6675_H_ */
