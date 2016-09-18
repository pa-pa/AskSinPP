
#ifndef __ACTIVITY_H__
#define __ACTIVITY_H__

#include "Debug.h"
#include "AlarmClock.h"
#include <LowPower.h>


namespace as {


class Activity : public Alarm {

  bool  awake;

public:

  Activity () : Alarm(0), awake(false) {
    async(true);
  }

  virtual ~Activity () {}

  virtual void trigger (AlarmClock& clock) {
    awake = false;
  }

  // do not sleep for time in 1/10 sec
  void stayAwake (uint32_t time) {
    uint32_t old = aclock.get(*this);
    if( old < time ) {
      awake = true;
      aclock.cancel(*this);
      tick = time;
      aclock.add(*this);
    }
  }

  void savePower () {
    if( awake == false ) {
#ifndef NDEBUG
//      DPRINT(F("Go sleep - ")); DHEXLN((uint16_t)aclock.next());
      Serial.flush();
      while (!(UCSR0A & (1 << UDRE0))) {  // Wait for empty transmit buffer
        UCSR0A |= 1 << TXC0;  // mark transmission not complete
      }
      while (!(UCSR0A & (1 << TXC0)));   // Wait for the transmission to complete
#endif
#ifdef POWER_SLEEP
      uint32_t ticks = aclock.next();
      if( aclock.isready() == false ) {
        if( ticks == 0 ) {
          LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_OFF);
        }
        else if( ticks > 80 ) {
          LowPower.powerDown(SLEEP_8S,ADC_OFF,BOD_OFF);
          aclock.correct(80);
        }
        else if (ticks > 10 ) {
          LowPower.powerDown(SLEEP_1S,ADC_OFF,BOD_OFF);
          aclock.correct(10);
        }
        else if (ticks > 5 ) {
          LowPower.powerDown(SLEEP_500MS,ADC_OFF,BOD_OFF);
          aclock.correct(5);
        }
      }
#else // default is idle
      LowPower.idle(SLEEP_FOREVER,ADC_OFF,TIMER2_OFF,TIMER1_ON,TIMER0_OFF,SPI_ON,USART0_ON,TWI_OFF);
#endif
    }
  }

};

extern Activity activity;

}


#endif
