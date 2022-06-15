//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ALARMCLOCK_H__
#define __ALARMCLOCK_H__

#include "Debug.h"
#include "Alarm.h"

#ifdef ARDUINO_ARCH_EFM32
#include "rtcdriver.h"
#endif

#ifdef ARDUINO_ARCH_RP2040
  #include "hardware/rtc.h"
  #include "pico/stdlib.h"
  #include "pico/util/datetime.h"
#endif

namespace as {

#ifndef TICKS_PER_SECOND
  // default 100 ticks per second
  #define TICKS_PER_SECOND 100UL
#endif

#define seconds2ticks(tm) ( (tm) * TICKS_PER_SECOND )
#define ticks2seconds(tm) ( (tm) / TICKS_PER_SECOND )

#define decis2ticks(tm) ( (tm) * TICKS_PER_SECOND / 10 )
#define ticks2decis(tm) ( (tm) * 10UL / TICKS_PER_SECOND )

#define centis2ticks(tm)  ( (tm) * TICKS_PER_SECOND / 100 )
#define ticks2centis(tm)  ( (tm) * 100UL / TICKS_PER_SECOND )

#define millis2ticks(tm) ( (tm) * TICKS_PER_SECOND / 1000 )
#define ticks2millis(tm) ( (tm) * 1000UL / TICKS_PER_SECOND )

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
    while( runsingle()==true ) {
      worked=true;
    }
    return worked;
  }

  bool runsingle() {
    Alarm* a = (Alarm*) ready.unlink();
    if (a != 0) {
      a->active(false);
      a->trigger(*this);
      return true;
    }
    return false;
  }

  bool runwait () {
    return runsingle();
  }

  void add(Alarm& item);

  uint32_t get(const Alarm& item) const;

  uint32_t next () const {
    Alarm* n = (Alarm*)select();
    return n != 0 ? n->tick : 0;
  }

  Alarm* first () const {
    return (Alarm*)select();
  }

  // correct the alarms after sleep
  void correct (uint32_t ticks) {
    ticks--;
    Alarm* n = first();
    if( n != 0 ) {
      uint32_t nextticks = n->tick-1;
      n->tick -= nextticks < ticks ? nextticks : ticks;
    }
    --(*this);
  }

};


#ifdef ARDUINO_ARCH_EFM32
extern void callback(RTCDRV_TimerID_t id , void *user);
extern void rtccallback(RTCDRV_TimerID_t id , void *user);
extern uint32_t getTimeout();
#else
extern void callback(void);
extern void rtccallback(void);
#endif


class SysClock : public AlarmClock {

#ifdef ARDUINO_ARCH_ESP32
  hw_timer_t * Timer = NULL;
  portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
#endif

#if defined ARDUINO_ARCH_STM32 && defined STM32L1xx
  HardwareTimer* Timer = new HardwareTimer(TIM6);
#endif

#ifdef ARDUINO_ARCH_RP2040
private:
  struct repeating_timer  rp2040_timer;
  static bool TimerHandler(__attribute__((unused)) struct repeating_timer *t) { callback(); return true; }
#endif

#ifdef ARDUINO_ARCH_EFM32
  RTCDRV_TimerID_t id;
  uint32_t timerTimeout;
#endif

public:
  static SysClock& instance();

  void init() {
  #ifdef ARDUINO_ARCH_AVR
  #define TIMER1_RESOLUTION 65536UL  // Timer1 is 16 bit
    // use Time1 on AVR
    TCCR1B = _BV(WGM13);        // set mode as phase and frequency correct pwm, stop the timer
    TCCR1A = 0;                 // clear control register A
    const unsigned long cycles = (F_CPU / 2000000) * (1000000 / TICKS_PER_SECOND);
    unsigned short pwmPeriod;
    unsigned char clockSelectBits;

    if (cycles < TIMER1_RESOLUTION) {
      clockSelectBits = _BV(CS10);
      pwmPeriod = (unsigned short)cycles;
    }
    else if (cycles < TIMER1_RESOLUTION * 8) {
      clockSelectBits = _BV(CS11);
      pwmPeriod = cycles / 8;
    }
    else if (cycles < TIMER1_RESOLUTION * 64) {
      clockSelectBits = _BV(CS11) | _BV(CS10);
      pwmPeriod = cycles / 64;
    }
    else if (cycles < TIMER1_RESOLUTION * 256) {
      clockSelectBits = _BV(CS12);
      pwmPeriod = cycles / 256;
    }
    else if (cycles < TIMER1_RESOLUTION * 1024) {
      clockSelectBits = _BV(CS12) | _BV(CS10);
      pwmPeriod = cycles / 1024;
    }
    else {
      clockSelectBits = _BV(CS12) | _BV(CS10);
      pwmPeriod = TIMER1_RESOLUTION - 1;
    }
    TCNT1 = 0;
    ICR1 = pwmPeriod;
    TCCR1B = _BV(WGM13) | clockSelectBits;
  #endif
  #ifdef ARDUINO_ARCH_STM32F1
    // Setup Timer2 on ARM
    Timer2.setMode(TIMER_CH2,TIMER_OUTPUT_COMPARE);
    Timer2.setPeriod(1000000 / TICKS_PER_SECOND); // in microseconds
    Timer2.setCompare(TIMER_CH2, 1); // overflow might be small
  #endif
  #if defined (ARDUINO_ARCH_STM32) && defined (STM32L1xx)
    Timer->setOverflow(1000000 / TICKS_PER_SECOND, MICROSEC_FORMAT);
    Timer->attachInterrupt(callback);
  #endif
  #ifdef ARDUINO_ARCH_ESP32
    // https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
    Timer = timerBegin(0, 80, true);
    timerAlarmWrite(Timer, 1000000 / TICKS_PER_SECOND, true);
    timerAttachInterrupt(Timer, &callback, true);
  #endif
  #ifdef ARDUINO_ARCH_EFM32
    RTCDRV_Init();
    RTCDRV_AllocateTimer( &id );
    timerTimeout = 10;
 #endif
    enable();
  }

  void disable () {
  #if defined ARDUINO_AVR_ATmega32 || defined(__AVR_ATmega128__)
    TIMSK &= ~_BV(TOIE1);
  #elif defined(ARDUINO_ARCH_AVR)
    TIMSK1 &= ~_BV(TOIE1);
  #elif defined(ARDUINO_ARCH_STM32F1)
    Timer2.detachInterrupt(TIMER_CH2);
  #elif defined ARDUINO_ARCH_STM32 && defined STM32L1xx
    Timer->pause();
  #endif
  #ifdef ARDUINO_ARCH_ESP32
    if (Timer != NULL)
      timerAlarmDisable(Timer);
  #endif
  #ifdef ARDUINO_ARCH_RP2040
    cancel_repeating_timer(&rp2040_timer);
  #endif
  #ifdef ARDUINO_ARCH_EFM32
    bool timerIsRunning;
    RTCDRV_IsRunning(id, &timerIsRunning);
    if (timerIsRunning)
    {
      //DPRINTLN("stopTimer");
      RTCDRV_StopTimer(id);
    }
  #endif
  }

  uint32_t getTimeout() {
    return timerTimeout;
  }

  void enable(uint32_t ms) {
    timerTimeout=ms < 10 ? 10 : ms;
    //DPRINT("enable ");DDECLN(ms);
    bool timerIsRunning;
    RTCDRV_IsRunning(id, &timerIsRunning);
    if (!timerIsRunning) {
      RTCDRV_StopTimer(id);
    }
    RTCDRV_StartTimer( id, rtcdrvTimerTypeOneshot, timerTimeout, callback , 0  );
  }

  void enable () {
  #if defined ARDUINO_AVR_ATmega32|| defined(__AVR_ATmega128__)
    TIMSK |= _BV(TOIE1);
  #elif defined(ARDUINO_ARCH_AVR)
    TIMSK1 |= _BV(TOIE1);
  #elif defined(ARDUINO_ARCH_STM32F1)
    Timer2.attachInterrupt(TIMER_CH2,callback);
  #elif defined ARDUINO_ARCH_STM32 && defined STM32L1xx
    Timer->resume();
  #endif
  #ifdef ARDUINO_ARCH_ESP32
    if (Timer != NULL)
      timerAlarmEnable(Timer);
  #endif
  #ifdef ARDUINO_ARCH_RP2040
    if (rp2040_timer.alarm_id < 1) {
      add_repeating_timer_us(10000UL, TimerHandler, NULL, &rp2040_timer);
    }
  #endif
  #ifdef ARDUINO_ARCH_EFM32
    timerTimeout = 10;
    RTCDRV_StartTimer( id, rtcdrvTimerTypeOneshot, timerTimeout, callback , 0  );
  #endif
  }

#ifdef ARDUINO_ARCH_EFM32
  RTCDRV_TimerID_t getTimerID() {
    return id;
  }
#endif

  void add(Alarm& item) {
    AlarmClock::add(item);
  }

  void add(Alarm& item,uint32_t millis) {
    item.tick = (millis2ticks(millis));
    add(item);
  }
};

extern SysClock sysclock;


#ifndef NORTC

class RealTimeClock : public AlarmClock {
  uint8_t ovrfl;
#if defined(ARDUINO_ARCH_STM32F1) && defined(_RTCLOCK_H_)
  RTClock rt;
#endif

#ifdef ARDUINO_ARCH_EFM32
  RTCDRV_TimerID_t id;
#endif

public:

  RealTimeClock() : ovrfl(0) {}

  static RealTimeClock& instance();

  void init () {
#if defined ARDUINO_AVR_ATmega32
    TIMSK &= ~(1<<TOIE2); //Disable timer2 interrupts
    ASSR  |= (1<<AS2); //Enable asynchronous mode
    TCNT2 = 0; //set initial counter value
    TCCR2 = (1<<CS22)|(1<<CS20); // mode normal & set prescaller 128
    while (ASSR & (1<<TCN2UB)); //wait for registers update
    TIFR |= (1<<TOV2); //clear interrupt flags
    TIMSK |= (1<<TOIE2); //enable TOV2 interrupt
#elif defined(__AVR_ATmega128__)
    TIMSK &= ~(1<<TOIE2); //Disable timer2 interrupts
    ASSR  |= (1<<AS0); //Enable asynchronous mode
    TCNT2 = 0; //set initial counter value
    TCCR2 = (1<<CS22)|(1<<CS20); // mode normal & set prescaller 128
    while (ASSR & (1<<TCN0UB)); //wait for registers update
    TIFR |= (1<<TOV2); //clear interrupt flags
    TIMSK |= (1<<TOIE2); //enable TOV2 interrupt
#elif defined(ARDUINO_ARCH_AVR)
    TIMSK2  = 0; //Disable timer2 interrupts
    ASSR  = (1<<AS2); //Enable asynchronous mode
    TCNT2 = 0; //set initial counter value
    TCCR2A = 0; // mode normal
    TCCR2B = (1<<CS22)|(1<<CS20); //set prescaller 128
    while (ASSR & ((1<<TCN2UB)|(1<<TCR2BUB))); //wait for registers update
    TIFR2  = (1<<TOV2); //clear interrupt flags
    TIMSK2  = (1<<TOIE2); //enable TOV2 interrupt
#elif defined(ARDUINO_ARCH_STM32F1) && defined(_RTCLOCK_H_)
    rt = RTClock(RTCSEL_LSE);
    rt.attachSecondsInterrupt(rtccallback);
#elif defined(ARDUINO_ARCH_RP2040)
    datetime_t alarmT;
    rtc_init();
    rtc_get_datetime(&alarmT);
    alarmT.min = alarmT.hour = alarmT.day = alarmT.dotw = alarmT.month = alarmT.year = -1;
    alarmT.sec =  1;
    rtc_set_alarm(&alarmT, rtccallback); // https://raspberrypi.github.io/pico-sdk-doxygen/group__hardware__rtc.html
#elif defined(ARDUINO_ARCH_EFM32)
    RTCDRV_Init();
    RTCDRV_AllocateTimer( &id );
    RTCDRV_StartTimer( id, rtcdrvTimerTypePeriodic, 1000, rtccallback , 0  );
#else
  #warning "RTC not supported"
#endif
  }

  // return millis done of the current second
  uint32_t getCurrentMillis () {
#ifdef ARDUINO_ARCH_AVR
    return (TCNT2 * 1000) / 255;
#else
    return 0; // not supported ???
#endif
  }

  uint32_t getCounter (bool resetovrflow) {
    if( resetovrflow == true ) {
      ovrfl = 0;
    }
#ifdef ARDUINO_ARCH_AVR
    return (256 * ovrfl) + TCNT2;
#elif defined(ARDUINO_ARCH_STM32F1) && defined(_RTCLOCK_H_)
    return rtc_get_count();
#else
    return 0;
#endif
  }

  void overflow () {
    ovrfl++;
  }

  void debug () {
    if( select() != 0 ) {
      DDEC((uint16_t)((Alarm*)select())->tick);
      DPRINT(F(" "));
    }
  }

  void add(Alarm& item) {
    AlarmClock::add(item);
  }

  void add(Alarm& item,uint32_t millis) {
    millis += getCurrentMillis();
    item.tick = (millis / 1000); // seconds to wait
    add(item);
  }

  void add(RTCAlarm& item,uint32_t millis) {
    // correct the time with the millis of the current second
    millis += getCurrentMillis();
    item.tick = (millis / 1000); // seconds to wait
    item.millis = millis % 1000; // millis to wait
    add(item);
  }

};

// backward compatibility
#if not defined(ARDUINO_ARCH_STM32) && not defined(ARDUINO_ARCH_EFM32)
typedef RealTimeClock RTC;
#endif
extern RealTimeClock rtc;
#endif

}

#endif
