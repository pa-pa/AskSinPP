//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ASKSINPP_h__
#define __ASKSINPP_h__

#define ASKSIN_PLUS_PLUS_VERSION "1.0.0"

#define ASKSIN_PLUS_PLUS_IDENTIFIER F("AskSin++ V" ASKSIN_PLUS_PLUS_VERSION)

// configure EnableInterrupt library
// #define EI_NOTEXTERNAL

#include <Debug.h>
#include <Activity.h>
#include <Led.h>
#include <AlarmClock.h>
#include <Message.h>
#include <Button.h>
#include <Radio.h>
#include <BatterySensor.h>

namespace as {

template <class StatusLed,class Battery,class Radio>
class AskSin {
public:
  typedef StatusLed LedType;
  typedef Battery   BatteryType;
  typedef Radio     RadioType;

  LedType     led;
  BatteryType battery;
  RadioType   radio;
  Activity    activity;

  static void pgm_read(uint8_t* dest,uint16_t adr,uint8_t size) {
    for( int i=0; i<size; ++i, ++dest ) {
      *dest = pgm_read_byte(adr + i);
    }
  }

};

}

#endif
