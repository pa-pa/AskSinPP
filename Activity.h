//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ACTIVITY_H__
#define __ACTIVITY_H__

#include <Debug.h>
#include <AlarmClock.h>
#include <Radio.h>

#ifdef ARDUINO_ARCH_AVR
#include <LowPower.h>
#endif

namespace as {

#ifdef ARDUINO_ARCH_AVR

template <bool ENABLETIMER2=false>
class Idle {
public:

  static void waitSerial () {
//      DPRINT(F("Go sleep - ")); DHEXLN((uint16_t)sysclock.next());
      Serial.flush();
      while (!(UCSR0A & (1 << UDRE0))) {  // Wait for empty transmit buffer
        UCSR0A |= 1 << TXC0;  // mark transmission not complete
      }
      while (!(UCSR0A & (1 << TXC0)));   // Wait for the transmission to complete
  }

  template <class Hal>
  static void powerSave (__attribute__((unused)) Hal& hal) {
    LowPower.idle(SLEEP_FOREVER,ADC_OFF,ENABLETIMER2==false?TIMER2_OFF:TIMER2_ON,TIMER1_ON,TIMER0_OFF,SPI_ON,USART0_ON,TWI_OFF);
  }

};

template <bool ENABLETIMER2=false>
class Sleep : public Idle<ENABLETIMER2> {
public:
  static uint32_t doSleep (uint32_t ticks) {
    uint32_t offset = 0;
    if( ticks == 0 ) {
      if( ENABLETIMER2 == false ) {
        LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_OFF);
      }
      else {
        LowPower.powerExtStandby(SLEEP_FOREVER,ADC_OFF,BOD_OFF,TIMER2_ON);
      }
    }
    else if( ticks > seconds2ticks(8) ) {
      if( ENABLETIMER2 == false ) {
        LowPower.powerDown(SLEEP_8S,ADC_OFF,BOD_OFF);
      }
      else {
        LowPower.powerExtStandby(SLEEP_8S,ADC_OFF,BOD_OFF,TIMER2_ON);
      }
      offset = seconds2ticks(8);
    }
    else if (ticks > seconds2ticks(1) ) {
      if( ENABLETIMER2 == false ) {
        LowPower.powerDown(SLEEP_1S,ADC_OFF,BOD_OFF);
      }
      else {
        LowPower.powerExtStandby(SLEEP_1S,ADC_OFF,BOD_OFF,TIMER2_ON);
      }
      offset = seconds2ticks(1);
    }
    else if (ticks > millis2ticks(500) ) {
      if( ENABLETIMER2 == false ) {
        LowPower.powerDown(SLEEP_500MS,ADC_OFF,BOD_OFF);
      }
      else {
        LowPower.powerExtStandby(SLEEP_500MS,ADC_OFF,BOD_OFF,TIMER2_ON);
      }
      offset = millis2ticks(500);
    }
    return offset;
  }

  template <class Hal>
  static void powerSave (Hal& hal) {
    sysclock.disable();
    uint32_t ticks = sysclock.next();
    if( sysclock.isready() == false ) {
      if( ticks == 0 || ticks > millis2ticks(500) ) {
        hal.radio.setIdle();
        uint32_t offset = doSleep(ticks);
        hal.radio.wakeup();
        sysclock.correct(offset);
        sysclock.enable();
      }
      else{
        sysclock.enable();
        Idle<ENABLETIMER2>::powerSave(hal);
      }
    }
    else {
      sysclock.enable();
    }
  }
};

#endif

class Activity : public Alarm {

  bool  awake;

public:

  Activity () : Alarm(0), awake(false) {
    async(true);
  }

  virtual ~Activity () {}

  virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
    awake = false;
  }

  // do not sleep for time in ticks
  void stayAwake (uint32_t time) {
    uint32_t old = sysclock.get(*this);
    if( old < time ) {
      awake = true;
      sysclock.cancel(*this);
      tick = time;
      sysclock.add(*this);
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
#ifdef ARDUINO_ARCH_AVR
      LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_OFF);
#endif
    }
  }

};

}

#endif
