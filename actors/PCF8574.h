//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-11-01 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2018-11-01 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __PCF8574Output_H__
#define __PCF8574Output_H__

#include <Arduino.h>
#include <Wire.h>

namespace as {

struct PCF8574Buffer { uint8_t data, mode; };

template <byte ADDRESS=0x38>
class PCF8574Output {
      static PCF8574Buffer& getBuffer();
  private:
      static void writeWire(uint8_t buffer) {
          Wire.beginTransmission(ADDRESS);
          Wire.write(buffer);
          Wire.endTransmission();
      }
  public:
      inline static void setOutput   (uint8_t pin) {
          getBuffer().mode |= bit(pin);
      }
      inline static void setInput    (uint8_t pin) {
          getBuffer().mode &= ~bit(pin);
      }
      inline static void setHigh     (uint8_t pin) {
          getBuffer().data |= bit(pin);
          writeWire((getBuffer().data & getBuffer().mode));
      }
      inline static void setLow      (uint8_t pin) {
          getBuffer().data &= ~bit(pin);
          writeWire((getBuffer().data & getBuffer().mode));
      }
      inline static uint8_t getState (uint8_t pin) {
          return ((getBuffer().data & bit(pin)) == 0) ? LOW : HIGH;
      }
      inline static void dump () {
        DPRINT("Address: "); DHEX(ADDRESS);
        DPRINT("   Mode: "); DHEX(getBuffer().mode);
        DPRINT("   Data: "); DHEXLN(getBuffer().data);
      }
      
      inline static void init () { 
      	Wire.begin();
      }
  };
  
  template<>
  inline PCF8574Buffer& PCF8574Output<0x20>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  template<>
  inline PCF8574Buffer& PCF8574Output<0x21>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  template<>
  inline PCF8574Buffer& PCF8574Output<0x22>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  template<>
  inline PCF8574Buffer& PCF8574Output<0x23>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  template<>
  inline PCF8574Buffer& PCF8574Output<0x24>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }  
  template<>
  inline PCF8574Buffer& PCF8574Output<0x25>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }  
  template<>
  inline PCF8574Buffer& PCF8574Output<0x26>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  template<>
  inline PCF8574Buffer& PCF8574Output<0x27>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  
  template<>
  inline PCF8574Buffer& PCF8574Output<0x38>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  template<>
  inline PCF8574Buffer& PCF8574Output<0x39>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  template<>
  inline PCF8574Buffer& PCF8574Output<0x3A>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  template<>
  inline PCF8574Buffer& PCF8574Output<0x3B>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  template<>
  inline PCF8574Buffer& PCF8574Output<0x3C>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }  
  template<>
  inline PCF8574Buffer& PCF8574Output<0x3D>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }  
  template<>
  inline PCF8574Buffer& PCF8574Output<0x3E>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
  template<>
  inline PCF8574Buffer& PCF8574Output<0x3F>::getBuffer () {
      static PCF8574Buffer b;
      return b;
  }
}

#endif
