//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-08-10 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __PWM_H__
#define __PWM_H__


#include <Arduino.h>
#include "PhaseCut.h"



namespace as {

enum DimCurve { linear, quadratic };

#if ARDUINO_ARCH_AVR
// we use this table for the dimmer levels
static const uint8_t pwmtable[32] PROGMEM = {
    1, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23,
    27, 32, 38, 45, 54, 64, 76, 91, 108, 128, 152, 181, 215, 255
};
static const uint8_t zctable[45] PROGMEM = {
     11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 58, 75 
};
template<uint8_t STEPS=200, bool LINEAR=false, bool INVERSE=false,class PINTYPE=ArduinoPins>
class PWM8 {
  uint8_t pin, dimCurve;
public:
  PWM8 () : pin(0), dimCurve((LINEAR)?linear:quadratic) {}
  ~PWM8 () {}

  void init(uint8_t p) {
    pin = p;
    PINTYPE::setPWM(pin);
  }

  void param(uint8_t m, uint8_t c) {
    dimCurve = c;
  }

  void set(uint8_t value) {
    uint8_t pwm = 0;
    if(dimCurve == linear) {
      if(INVERSE) {
        pwm = map(value, 0, STEPS, 255, 0); // https://www.arduino.cc/reference/en/language/functions/math/map/
      } else {
        pwm = map(value, 0, STEPS, 0, 255);
      }
    } else {
      if( value > 0 ) {
        uint8_t offset = value*31/STEPS;
        pwm = pgm_read_word (& pwmtable[offset]);
      }
    }
    PINTYPE::setPWM(pin,pwm);
  }
};
template<uint8_t STEPS=200>
class ZC_Control {
	uint8_t  outpin;
	public:
	ZC_Control () : outpin(0) {}
	~ZC_Control () {}
		
	void init(uint8_t p) {
    outpin = p;
    phaseCut.init(outpin);
	  phaseCut.Start();
  }

  void param(uint8_t m, uint8_t c) {
  }
  
  void set(double value){
		uint8_t pwm = 0;
		if ( value > 0 ) {
			if (!phaseCut.isrunning()){
				phaseCut.Start();
			}
			#if PHASECUTMODE == 1
				uint8_t offset = (STEPS - value)*44/STEPS;
			#else
				uint8_t offset = value*44/STEPS;
			#endif
			pwm = pgm_read_word (& zctable[offset]);
			phaseCut.SetDimValue(pwm);
		}
	  else{
		phaseCut.Stop();
	  }  
   }
};
#endif

#ifdef ARDUINO_ARCH_STM32F1

template<uint8_t STEPS=200,uint16_t FREQU=65535,class PINTYPE=ArduinoPins>
class PWM16 {
  float R;
  uint8_t pin, dimCurve;
public:
  PWM16 () : pin(0) {
    R = (STEPS * log10(2))/(log10(FREQU));
  }
  ~PWM16 () {}

  void init(uint8_t p) {
    pin = p;
    PINTYPE::setPWM(pin);
    set(0);
  }

  void param(uint8_t m, uint8_t c) {
    //DPRINT("multiplier: "); DPRINT(m); DPRINT(", dimCurve: "); DPRINTLN(c);
    //PINTYPE::setPWMFreq(m * 200);
    dimCurve = c;
  }
  //void param(__attribute__((unused)) uint8_t m,__attribute__((unused)) uint8_t c) {}


  void set(uint8_t value) {
    uint16_t duty = 0;
    if (value == 0) {
      duty = value;
    }
    else if (value == STEPS) {
      duty = 0xFFFF;
    }
    else if (dimCurve == linear) {
      duty = map(value, 0, STEPS, 0, 65535);
    }
    else {
      // https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms/
      // duty = pow(2,(value/R)) + 4;
      // duty = pow(2,(value/20.9)+6.5);
      // duty = pow(1.37,(value/15.0)+22.0)-500;
      duty = pow(1.28, (value / 13.0) + 29.65) - 1300;
      // http://harald.studiokubota.com/wordpress/index.php/2010/09/05/linear-led-fading/index.html
      //duty = exp(value/18.0) + 4;
    }
    // DDEC(pin);DPRINT(" - ");DDECLN(duty);
    PINTYPE::setPWM(pin,duty);
  }
};

#endif

#if defined ARDUINO_ARCH_STM32 && defined STM32L1xx
template<uint8_t STEPS = 200, class PINTYPE = ArduinoPins>
class PWM16 {
  uint8_t pin, curve;
public:
  PWM16() : pin(0) {}
  ~PWM16() {}

  void init(uint8_t p) {
    //DPRINT(F("init: ")); DPRINTLN(p);
    pin = p;
    PINTYPE::setPWM(pin);
    PINTYPE::setPWMRes(16);
  }
  
  void param(uint8_t m, uint8_t c) {
    PINTYPE::setPWMFreq(m * 200);
    curve = c;
    //DPRINT(F("multiplier: ")); DPRINT(m); DPRINT(F(", curve: ")); DPRINTLN(curve);
  }

  void set(uint8_t value) {
    uint16_t ret = 0;

    if (curve == linear) {
      //ret = map(value, 0, STEPS, 0, 65535);
      ret = ((uint32_t)value * (uint32_t)13107) / 40;  // same as above but 60 byte less
    }
    else {
      // https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms/
      // ret = pow(2,(value/R)) + 4;
      // ret = pow(2,(value/20.9)+6.5);
      // ret = pow(1.37,(value/15.0)+22.0)-500;
      //ret = pow(1.28, (value / 13.0) + 29.65) - 1300;
      // http://harald.studiokubota.com/wordpress/index.php/2010/09/05/linear-led-fading/index.html
      //ret = exp(value/18.0) + 4;
      
      // 734 seems the best factor to calculate from quadratic step 200 to 0xFF, 0xFFF or 0xFFFF
      // divisor 16 bit: 448, 12 bit: 7168, 8 bit: 114688
      ret = ((uint32_t)value * (uint32_t)value * 734) / 448;
    }

    DDEC(pin); DPRINT(F(":c")); DPRINT(curve); DPRINT(F("\t(")); DDEC(value); DPRINT(F(")\t")); DDECLN(ret);
    PINTYPE::setPWM(pin, ret);
  }

};
#endif

};

#endif
