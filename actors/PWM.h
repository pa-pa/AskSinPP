//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-08-10 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __PWM_H__
#define __PWM_H__


#include <Arduino.h>
#include "PhaseCut.h"




namespace as {

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
template<uint8_t STEPS=200, bool LINEAR=false, bool INVERSE=false>
class PWM8 {
  uint8_t  pin;
public:
  PWM8 () : pin(0) {}
  ~PWM8 () {}

  void init(uint8_t p) {
    pin = p;
    pinMode(pin,OUTPUT);
  }
  void set(uint8_t value) {
    uint8_t pwm = 0;
    if(LINEAR) {
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
    analogWrite(pin,pwm);
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

template<uint8_t STEPS=200,uint16_t FREQU=65535>
class PWM16ext {
  uint8_t pin;
public:
  PWM16ext () : pin(0) {}
  ~PWM16ext () {}

  void init(uint8_t p) {
    pin = p;
    //pinMode(pin, PWM);
    set(0);
  }

  void set(uint8_t value) {
    uint16_t duty = 0;
    if ( value == STEPS) {
      duty = FREQU;
    }
    else if (value > 0) {
      // https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms/
      // duty = pow(2,(value/R)) + 4;
      // duty = pow(2,(value/20.9)+6.5);
      // duty = pow(1.37,(value/15.0)+22.0)-500;
      duty = pow(1.28,(value/13.0)+29.65)-1300;
      // http://harald.studiokubota.com/wordpress/index.php/2010/09/05/linear-led-fading/index.html
      //duty = exp(value/18.0) + 4;
    }
    // DDEC(pin);DPRINT(" - ");DDECLN(duty);
    //pwmWrite(pin,duty);
  }
};


#ifdef ARDUINO_ARCH_STM32F1

template<uint8_t STEPS=200,uint16_t FREQU=65535>
class PWM16 {
  float R;
  uint8_t pin;
public:
  PWM16 () : pin(0) {
    R = (STEPS * log10(2))/(log10(FREQU));
  }
  ~PWM16 () {}

  void init(uint8_t p) {
    pin = p;
    pinMode(pin,PWM);
    set(0);
  }

  void set(uint8_t value) {
    uint16_t duty = 0;
    if ( value == STEPS) {
      duty = FREQU;
    }
    else if (value > 0) {
      // https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms/
      // duty = pow(2,(value/R)) + 4;
      // duty = pow(2,(value/20.9)+6.5);
      // duty = pow(1.37,(value/15.0)+22.0)-500;
      duty = pow(1.28,(value/13.0)+29.65)-1300;
      // http://harald.studiokubota.com/wordpress/index.php/2010/09/05/linear-led-fading/index.html
      //duty = exp(value/18.0) + 4;
    }
    // DDEC(pin);DPRINT(" - ");DDECLN(duty);
    pwmWrite(pin,duty);
  }
};

#endif

};

#endif
