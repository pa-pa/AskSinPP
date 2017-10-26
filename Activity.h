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
    period_t sleeptime = SLEEP_FOREVER;

    if( ticks > seconds2ticks(8) ) { offset = seconds2ticks(8); sleeptime = SLEEP_8S; }
    else if( ticks > seconds2ticks(4) )  { offset = seconds2ticks(4);  sleeptime = SLEEP_4S; }
    else if( ticks > seconds2ticks(2) )  { offset = seconds2ticks(2);  sleeptime = SLEEP_2S; }
    else if( ticks > seconds2ticks(1) )  { offset = seconds2ticks(1);  sleeptime = SLEEP_1S; }
    else if( ticks > millis2ticks(500) ) { offset = millis2ticks(500); sleeptime = SLEEP_500MS; }
    else if( ticks > millis2ticks(250) ) { offset = millis2ticks(250); sleeptime = SLEEP_250MS; }
    else if( ticks > millis2ticks(120) ) { offset = millis2ticks(120); sleeptime = SLEEP_120MS; }
    else if( ticks > millis2ticks(60)  ) { offset = millis2ticks(60);  sleeptime = SLEEP_60MS; }
    else if( ticks > millis2ticks(30)  ) { offset = millis2ticks(30);  sleeptime = SLEEP_30MS; }
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
        hal.radio.setIdle();
        uint32_t offset = doSleep(ticks);
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

class SleepRTC : public Idle<true> {
public:
  static uint32_t doSleep (uint32_t ticks) {
    uint32_t offset = 0;
    period_t sleeptime = SLEEP_FOREVER;

    if( ticks > seconds2ticks(1) ) { sleeptime = SLEEP_FOREVER; }
    else if( ticks > millis2ticks(500) ) { sleeptime = SLEEP_500MS; }
    else if( ticks > millis2ticks(250) ) { sleeptime = SLEEP_250MS; }
    else if( ticks > millis2ticks(120) ) { sleeptime = SLEEP_120MS; }
    else if( ticks > millis2ticks(60)  ) { sleeptime = SLEEP_60MS; }
    else if( ticks > millis2ticks(30)  ) { sleeptime = SLEEP_30MS; }
    else if( ticks > millis2ticks(15)  ) { sleeptime = SLEEP_15MS; }

    uint16_t c1 = rtc.getCounter(true);
    LowPower.powerExtStandby(sleeptime,ADC_OFF,BOD_OFF,TIMER2_ON);
    uint16_t c2 = rtc.getCounter(false);
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
        hal.radio.setIdle();
        uint32_t offset = doSleep(ticks);
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
#ifndef NDEBUG
      Saver::waitSerial();
#endif
      Saver::powerSave(hal);
    }
    else {
      // ensure radio is up and running
      hal.radio.wakeup();
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

template <class HalType>
class BurstDetector : public Alarm {
  bool burst;
  HalType& hal;
public:
  BurstDetector (HalType& h) : Alarm(millis2ticks(250)), burst(false), hal(h) {}
  virtual ~BurstDetector () {}
  virtual void trigger (AlarmClock& clock) {
    uint32_t next = millis2ticks(250);
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
    set(next);
    clock.add(*this);
  }
  void enable(AlarmClock& clock) {
    clock.add(*this);
  }
};


}

#endif
