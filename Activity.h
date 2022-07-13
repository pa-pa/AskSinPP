//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __ACTIVITY_H__
#define __ACTIVITY_H__

#include "Debug.h"
#include <AlarmClock.h>
#include <Radio.h>
#if defined(ARDUINO_ARCH_AVR) && ! ( defined(ARDUINO_AVR_ATmega32) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega128__))
#include <LowPower.h>
#endif
#if defined ARDUINO_ARCH_STM32 && defined STM32L1xx
#include "low_power.h"
#include "rtc.h"
#include <time.h>

typedef void (*voidFuncPtrVoid)(void);

static void rtcmatch(void*) {
  //DPRINT('.');
}

static class STM32L1xx_LowPower {
public:
  STM32L1xx_LowPower() {};

  void begin(void) {
    RTC_SetClockSource(LSI_CLOCK);
    RTC_setPrediv(36, 0);
    RTC_init(HOUR_FORMAT_24, LSI_CLOCK, true);
    LowPower_init();
  }

  void idle(uint32_t millis = 0) {
    if (millis > 0) programRtcWakeUp(millis);
    LowPower_sleep(PWR_MAINREGULATOR_ON);
  }

  void sleep(uint32_t millis = 0) {
    if (millis > 0) programRtcWakeUp(millis);
    LowPower_sleep(PWR_LOWPOWERREGULATOR_ON);
  }

  void deepSleep(uint32_t millis = 0) {
    if (millis > 0) programRtcWakeUp(millis);
    LowPower_stop(_serial);
    detachAlarmCallback();
  }

  void shutdown(uint32_t millis = 0) {
    if (millis > 0) programRtcWakeUp(millis);
    LowPower_shutdown();
  }

  void programRtcWakeUp(uint32_t millis) {
    uint32_t _subSeconds; hourAM_PM_t p;
    uint8_t _day, _hour, _min, _sec, _dmy;
    RTC_GetTime(&_hour, &_min, &_sec, &_subSeconds, &p);
    RTC_GetDate(&_dmy, &_dmy, &_day, &_dmy);
    // tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday, tm_isdst
    struct tm tm = { _sec, _min, _hour, _day, 0, 100, 0, 0, -1 };
    time_t tmp = mktime(&tm);
    tmp += millis;

    struct tm* ptm = gmtime(&tmp);
    //DPRINT(millis); DPRINT(", "); DPRINTLN((uint32_t)tmp);
    //DPRINT(tm.tm_mday); DPRINT(':'); DPRINT(tm.tm_hour); DPRINT(':'); DPRINT(tm.tm_min); DPRINT(':'); DPRINTLN(tm.tm_sec);
    //DPRINT(ptm->tm_mday); DPRINT(':'); DPRINT(ptm->tm_hour); DPRINT(':'); DPRINT(ptm->tm_min); DPRINT(':'); DPRINTLN(ptm->tm_sec);
    RTC_StartAlarm(ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, _subSeconds, p, 15);
    attachAlarmCallback(rtcmatch, NULL);
  }

  void attachInterruptWakeup(uint32_t pin, voidFuncPtrVoid callback, uint32_t mode) {
    attachInterrupt(pin, callback, mode);
    LowPower_EnableWakeUpPin(pin, mode);
  }

private:
  serial_t* _serial;    // Serial for wakeup from deep sleep
} LowPower;
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "esp_sleep.h"
#endif

namespace as {

#if defined(ARDUINO_ARCH_AVR) && ! (defined(ARDUINO_AVR_ATmega32) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega128__))


template <bool ENABLETIMER2=false, bool ENABLEADC=false, bool ENABLETWI=false>
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
#if defined __AVR_ATmega644P__ || defined (__AVR_ATmega1284P__)
    LowPower.idle(SLEEP_FOREVER,ENABLEADC==true?ADC_ON:ADC_OFF,ENABLETIMER2==false?TIMER2_OFF:TIMER2_ON,TIMER1_ON,TIMER0_OFF,SPI_ON,USART1_OFF,USART0_ON,ENABLETWI==true?TWI_ON:TWI_OFF);
#elif defined __AVR_ATmega2560__
    //there is an issue, so you have to manual change something in Low-Power.cpp: https://github.com/rocketscream/Low-Power/issues/30#issuecomment-336801240
    LowPower.idle(SLEEP_FOREVER,ENABLEADC==true?ADC_ON:ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF,ENABLETIMER2==false?TIMER2_OFF:TIMER2_ON, TIMER1_ON, TIMER0_OFF, SPI_ON, USART3_OFF,USART2_OFF, USART1_OFF, USART0_ON, ENABLETWI==true?TWI_ON:TWI_OFF);
#else
    LowPower.idle(SLEEP_FOREVER,ENABLEADC==true?ADC_ON:ADC_OFF,ENABLETIMER2==false?TIMER2_OFF:TIMER2_ON,TIMER1_ON,TIMER0_OFF,SPI_ON,USART0_ON,ENABLETWI==true?TWI_ON:TWI_OFF);
#endif
  }

};

template <bool ENABLETIMER2=false>
class Sleep : public Idle<ENABLETIMER2> {
public:
  static uint32_t doSleep (uint32_t ticks) {
    uint32_t offset = 0;
    period_t sleeptime = SLEEP_FOREVER;

    if( ticks > seconds2ticks(8) ) { offset = seconds2ticks(8); sleeptime = SLEEP_8S; }
//    else if( ticks > seconds2ticks(4) )  { offset = seconds2ticks(4);  sleeptime = SLEEP_4S; }
//    else if( ticks > seconds2ticks(2) )  { offset = seconds2ticks(2);  sleeptime = SLEEP_2S; }
    else if( ticks > seconds2ticks(1) )  { offset = seconds2ticks(1);  sleeptime = SLEEP_1S; }
//    else if( ticks > millis2ticks(500) ) { offset = millis2ticks(500); sleeptime = SLEEP_500MS; }
//    else if( ticks > millis2ticks(250) ) { offset = millis2ticks(250); sleeptime = SLEEP_250MS; }
    else if( ticks > millis2ticks(120) ) { offset = millis2ticks(120); sleeptime = SLEEP_120MS; }
//    else if( ticks > millis2ticks(60)  ) { offset = millis2ticks(60);  sleeptime = SLEEP_60MS; }
//    else if( ticks > millis2ticks(30)  ) { offset = millis2ticks(30);  sleeptime = SLEEP_30MS; }
    else if( ticks > millis2ticks(15)  ) { offset = millis2ticks(15);  sleeptime = SLEEP_15MS; }

    if( ENABLETIMER2 == false ) {
      LowPower.powerDown(sleeptime,ADC_OFF,BOD_OFF);
    }
    else {
      LowPower.powerExtStandby(sleeptime,ADC_OFF,BOD_OFF,TIMER2_ON);
    }
    return offset;
  }

  template <class Hal>
  static void powerSave (Hal& hal) {
    sysclock.disable();
    uint32_t ticks = sysclock.next();
    if( sysclock.isready() == false ) {
      if( ticks == 0 || ticks > millis2ticks(15) ) {
        hal.setIdle();
        uint32_t offset = doSleep(ticks);
        hal.unsetIdle();
        sysclock.correct(offset);
        sysclock.enable();
      }
      else{
        sysclock.enable();
        //Idle<ENABLETIMER2>::powerSave(hal);
      }
    }
    else {
      sysclock.enable();
    }
  }
};

#ifndef NORTC
class SleepRTC : public Idle<true> {
public:
  static uint32_t doSleep (uint32_t ticks) {
    uint32_t offset = 0;
    period_t sleeptime = SLEEP_FOREVER;

    if( ticks > seconds2ticks(1) ) { sleeptime = SLEEP_FOREVER; }
//    else if( ticks > millis2ticks(500) ) { sleeptime = SLEEP_500MS; }
//    else if( ticks > millis2ticks(250) ) { sleeptime = SLEEP_250MS; }
    else if( ticks > millis2ticks(120) ) { sleeptime = SLEEP_120MS; }
//    else if( ticks > millis2ticks(60)  ) { sleeptime = SLEEP_60MS; }
//    else if( ticks > millis2ticks(30)  ) { sleeptime = SLEEP_30MS; }
    else if( ticks > millis2ticks(15)  ) { sleeptime = SLEEP_15MS; }

    uint32_t c1 = rtc.getCounter(true);
    LowPower.powerSave(sleeptime,ADC_OFF,BOD_OFF,TIMER2_ON);
    uint32_t c2 = rtc.getCounter(false);
    offset = (c2 - c1) * seconds2ticks(1) / 256;
    // DHEX(ticks);DPRINT("  ");DHEX(c1);DPRINT(":");DHEX(c2);DPRINT("  ");DHEXLN(offset);

    return min(ticks,offset);
  }

  template <class Hal>
  static void powerSave (Hal& hal) {
    sysclock.disable();
    uint32_t ticks = sysclock.next();
    if( sysclock.isready() == false ) {
      if( ticks == 0 || ticks > millis2ticks(15) ) {
        hal.setIdle();
        uint32_t offset = doSleep(ticks);
        hal.unsetIdle();
        sysclock.correct(offset);
        sysclock.enable();
      }
      else{
        sysclock.enable();
        Idle<true>::powerSave(hal);
      }
    }
    else {
      sysclock.enable();
    }
  }
};
#endif // NORTC

#endif

#if defined ARDUINO_ARCH_STM32 && defined STM32L1xx
// more time to spend here
template <bool ENABLETIMER2 = false, bool ENABLEADC = false>
class Idle {
public:

  static void waitSerial() {
    // DPRINT(F("Go sleep - ")); DHEXLN((uint16_t)sysclock.next());
    Serial.flush(); // waits for the transmission of outgoing serial data to complete
   }

  template <class Hal>
  static void powerSave(__attribute__((unused)) Hal& hal) {
    // ENABLEADC == true ? ADC_ON : ADC_OFF, ENABLETIMER2 == false ? TIMER2_OFF : TIMER2_ON, TIMER1_ON, TIMER0_OFF, SPI_ON, USART0_ON, TWI_OFF);
    LowPower.idle();
  }

};

template <bool ENABLETIMER2 = false>
class Sleep : public Idle<ENABLETIMER2> {
public:
  static uint32_t doSleep(uint32_t ticks) {
    uint32_t sleeptime = 0;

    // limit the max sleeptime to 8 seconds
    if (ticks > seconds2ticks(8)) ticks = seconds2ticks(8); 
    sleeptime = ticks2millis(ticks);

    // ADC_OFF, BOD_OFF, TIMER_OFF
    //DPRINT(millis2ticks(sleeptime)); delay(500);
    LowPower.deepSleep(sleeptime);
    return millis2ticks(sleeptime-1);
  }

  template <class Hal>
  static void powerSave(Hal& hal) {
    sysclock.disable();
    uint32_t ticks = sysclock.next();
    if (sysclock.isready() == false) {
      if (ticks == 0 || ticks > millis2ticks(15)) {
        hal.setIdle();
        uint32_t offset = doSleep(ticks);
        hal.unsetIdle();
        sysclock.correct(offset);
        sysclock.enable();
      } else {
        sysclock.enable();
        //Idle<ENABLETIMER2>::powerSave(hal);
      }
    } else {
      sysclock.enable();
    }
  }
};
#endif

#if defined ARDUINO_ARCH_EFM32
class Sleep {
public:
  template <class Hal>
  static void powerSave(Hal& hal) {
    uint32_t priMask = __get_PRIMASK();
    __set_PRIMASK(1);
    EMU_EnterEM2(false);
    __set_PRIMASK(priMask);
    uint32_t ticks = sysclock.next();
    if (sysclock.isready() == false) {
      sysclock.disable();
      hal.setIdle();
      sysclock.enable(ticks2millis(ticks));
      sysclock.correct(ticks == 0 ? 0 : ticks -1);
    } else {
      hal.unsetIdle();
      sysclock.enable();
    }
  }
};
#endif
class Activity : public Alarm {

  volatile bool  awake;

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

  bool stayAwake () const {
    return awake;
  }

  template <class Saver,class Hal>
  void savePower (Hal& hal) {
    if( awake == false ) {
      Saver::powerSave(hal);
    }
    else {
      // ensure radio is up and running
      hal.wakeup();
    }
  }

  template <class Hal>
  void sleepForever (Hal& hal) {
    hal.setIdle();
    while( true ) {
#if defined(ARDUINO_ARCH_AVR) && ! (defined(ARDUINO_AVR_ATmega32) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega128__))
  #ifndef NDEBUG
      Idle<>::waitSerial();
  #endif
      LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_OFF);
#endif
#if defined ARDUINO_ARCH_STM32 && defined STM32L1xx
  #ifndef NDEBUG
    Idle<>::waitSerial();
  #endif
      //DPRINTLN("shutdown");
      LowPower.shutdown(0);
#endif
#if defined(ARDUINO_ARCH_EFM32)
      while (1);
#endif
    }
  }

};

template <class HalType>
class BurstDetector : public Alarm {
  bool burst;
  HalType& hal;
public:
  BurstDetector (HalType& h) : Alarm(millis2ticks(250)), burst(false), hal(h) {}
  virtual ~BurstDetector () {}
  virtual void trigger (AlarmClock& clock) {
    uint32_t next = millis2ticks(250);
    if( hal.activity.stayAwake() == false ) {
      bool detect = hal.radio.detectBurst();
      if( detect == true ) {
        if( burst == false ) {
          burst = true;
          next = millis2ticks(30);
          // DPRINTLN("1");
        }
        else {
          burst = false;
          hal.activity.stayAwake(millis2ticks(500));
          // DPRINTLN("2");
        }
      }
      else {
        burst = false;
      }
    }
    set(next);
    clock.add(*this);
  }
  void enable(AlarmClock& clock) {
    clock.add(*this);
  }
};

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_RP2040)
class Sleep {
public:
  static uint32_t doSleep (uint32_t ticks) {
    uint32_t sleeptime = ticks2millis(ticks);

#ifdef ARDUINO_ARCH_RP2040
    sleep_ms(sleeptime); // https://raspberrypi.github.io/pico-sdk-doxygen/group__sleep.html
#else
    esp_sleep_enable_timer_wakeup(sleeptime * 100000);
    esp_light_sleep_start();
#endif

    return ticks;
  }

  static void waitSerial() {Serial.flush();};

  template <class Hal>
  static void powerSave (Hal& hal) {
    sysclock.disable();
    uint32_t ticks = sysclock.next();
    if( sysclock.isready() == false ) {
      if( ticks == 0 || ticks > millis2ticks(15) ) {
        hal.radio.setIdle();
        uint32_t offset = doSleep(ticks);
        sysclock.correct(offset);
        sysclock.enable();
      }
      else{
        sysclock.enable();
//        Idle<ENABLETIMER2>::powerSave(hal);
        powerSave(hal);
      }
    }
    else {
      sysclock.enable();
    }
  }
};
#endif

}
#endif
