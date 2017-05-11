//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ALARMCLOCK_H__
#define __ALARMCLOCK_H__

#include "Debug.h"
#include "Alarm.h"
#ifdef ARDUINO_ARCH_AVR
  #include "TimerOne.h"
#endif


namespace as {

#define TICKS_PER_SECOND 100UL

#define seconds2ticks(tm) ( tm * TICKS_PER_SECOND )
#define ticks2seconds(tm) ( tm / TICKS_PER_SECOND )

#define decis2ticks(tm) ( tm * TICKS_PER_SECOND / 10 )
#define ticks2decis(tm) ( tm * 10UL / TICKS_PER_SECOND )

#define centis2ticks(tm)  ( tm * TICKS_PER_SECOND / 100 )
#define ticks2centis(tm)  ( tm * 100UL / TICKS_PER_SECOND )

#define millis2ticks(tm) ( tm * TICKS_PER_SECOND / 1000 )
#define ticks2millis(tm) ( tm * 1000UL / TICKS_PER_SECOND )

class AlarmClock: protected Link {

  Link ready;

public:

  void cancel(Alarm& item);

  AlarmClock& operator --();

  bool isready () const {
    return ready.select() != 0;
  }

  bool runready() {
    bool worked = false;
    Alarm* a;
    while ((a = (Alarm*) ready.unlink()) != 0) {
      a->trigger(*this);
      worked = true;
    }
    return worked;
  }

  void add(Alarm& item);

  uint32_t get(const Alarm& item) const;

  uint32_t next () const {
    Alarm* n = (Alarm*)select();
    return n != 0 ? n->tick : 0;
  }

  // correct the alarms after sleep
  void correct (uint32_t ticks) {
    ticks--;
    Alarm* n = (Alarm*)select();
    if( n != 0 ) {
      uint32_t nextticks = n->tick-1;
      n->tick -= nextticks < ticks ? nextticks : ticks;
    }
    --(*this);
  }

};


extern void callback(void);

class SysClock : public AlarmClock {
public:
  void init() {
  #ifdef ARDUINO_ARCH_AVR
    // use TimeOne on AVR
    Timer1.initialize(1000000 / TICKS_PER_SECOND); // initialize timer1, and set a 1/10 second period
  #endif
  #ifdef ARDUINO_ARCH_STM32F1
    // Setup Timer2 on ARM
    Timer2.setMode(TIMER_CH2,TIMER_OUTPUT_COMPARE);
    Timer2.setPeriod(1000000 / TICKS_PER_SECOND); // in microseconds
    Timer2.setCompare(TIMER_CH2, 1); // overflow might be small
  #endif
    enable();
  }

  void disable () {
  #ifdef ARDUINO_ARCH_AVR
    Timer1.detachInterrupt();
  #endif
  #ifdef ARDUINO_ARCH_STM32F1
    Timer2.detachInterrupt(TIMER_CH2);
  #endif
  }

  void enable () {
  #ifdef ARDUINO_ARCH_AVR
    Timer1.attachInterrupt(callback);
  #endif
  #ifdef ARDUINO_ARCH_STM32F1
    Timer2.attachInterrupt(TIMER_CH2,callback);
  #endif
  }

};

extern SysClock sysclock;



class RTC : public AlarmClock {
public:

  RTC () {}

  void init () {
#ifdef ARDUINO_ARCH_AVR
    //Disable timer2 interrupts
    TIMSK2  = 0;
    //Enable asynchronous mode
    ASSR  = (1<<AS2);
    //set initial counter value
    TCNT2=0;
    //set prescaller 128
    TCCR2B = (1<<CS22)|(1<<CS00);
    //wait for registers update
    while (ASSR & ((1<<TCN2UB)|(1<<TCR2BUB)));
    //clear interrupt flags
    TIFR2  = (1<<TOV2);
    //enable TOV2 interrupt
    TIMSK2  = (1<<TOIE2);
#else
  #warning "RTC not supported"
#endif
  }
};

extern RTC rtc;

}

#endif
