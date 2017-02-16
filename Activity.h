//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ACTIVITY_H__
#define __ACTIVITY_H__

#include <Debug.h>
#include <AlarmClock.h>
#include <Radio.h>
#include <LowPower.h>


namespace as {

class Idle {
public:

  static void waitSerial () {
//      DPRINT(F("Go sleep - ")); DHEXLN((uint16_t)aclock.next());
      Serial.flush();
      while (!(UCSR0A & (1 << UDRE0))) {  // Wait for empty transmit buffer
        UCSR0A |= 1 << TXC0;  // mark transmission not complete
      }
      while (!(UCSR0A & (1 << TXC0)));   // Wait for the transmission to complete
  }

  template <class Hal>
  static void powerSave (Hal& hal) {
    LowPower.idle(SLEEP_FOREVER,ADC_OFF,TIMER2_OFF,TIMER1_ON,TIMER0_OFF,SPI_ON,USART0_ON,TWI_OFF);
  }

};

class Sleep : public Idle {
public:
  static uint32_t doSleep (uint32_t ticks) {
    uint32_t offset = 0;
    if( ticks == 0 ) {
      LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_OFF);
    }
    else if( ticks > seconds2ticks(8) ) {
      LowPower.powerDown(SLEEP_8S,ADC_OFF,BOD_OFF);
      offset = seconds2ticks(8);
    }
    else if (ticks > seconds2ticks(1) ) {
      LowPower.powerDown(SLEEP_1S,ADC_OFF,BOD_OFF);
      offset = seconds2ticks(1);
    }
    else if (ticks > millis2ticks(500) ) {
      LowPower.powerDown(SLEEP_500MS,ADC_OFF,BOD_OFF);
      offset = millis2ticks(500);
    }
    return offset;
  }

  template <class Hal>
  static void powerSave (Hal& hal) {
    aclock.disable();
    uint32_t ticks = aclock.next();
    if( aclock.isready() == false ) {
      if( ticks == 0 || ticks > millis2ticks(500) ) {
        hal.radio.setIdle();
        uint32_t offset = doSleep(ticks);
        hal.radio.wakeup();
        aclock.correct(offset);
        aclock.enable();
      }
      else{
        aclock.enable();
        Idle::powerSave(hal);
      }
    }
    else {
      aclock.enable();
    }
  }
};

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

  // do not sleep for time in ticks
  void stayAwake (uint32_t time) {
    uint32_t old = aclock.get(*this);
    if( old < time ) {
      awake = true;
      aclock.cancel(*this);
      tick = time;
      aclock.add(*this);
    }
  }

  template <class Saver,class Hal>
  void savePower (Hal& hal) {
    if( awake == false ) {
#ifndef NDEBUG
      Saver::waitSerial();
#endif
      Saver::powerSave(hal);
    }
  }

  template <class Hal>
  void sleepForever (Hal& hal) {
    hal.radio.setIdle();
    while( true ) {
      LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_OFF);
    }
  }

};

}

#endif
