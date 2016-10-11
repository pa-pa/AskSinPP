
#ifndef __BATTERYSENSOR_H__
#define __BATTERYSENSOR_H__

#include <Debug.h>
#include <AlarmClock.h>

#include <avr/power.h>

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
#define ADMUX_VCCWRT1V1 (_BV(MUX5) | _BV(MUX0))
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define ADMUX_VCCWRT1V1 (_BV(MUX3) | _BV(MUX2))
#else
#define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
#endif

namespace as {

/**
 * Use internal bandgap reference to measure battery voltage
 */
class BatterySensor : public Alarm {

  uint8_t  m_LowValue;
  uint8_t  m_LastValue;
  uint8_t  m_CriticalValue;
  uint32_t m_Period;

public:
  BatterySensor () : Alarm(0), m_LowValue(0), m_LastValue(0), m_CriticalValue(0xff), m_Period(0) {}
  virtual ~BatterySensor() {}

  virtual void trigger (AlarmClock& clock) {
    tick = m_Period;
    clock.add(*this);
    m_LastValue = voltage();
  }

  uint8_t current () {
    return m_LastValue;
  }

  void critical (uint8_t value) {
    m_CriticalValue = value;
  }

  bool critical () {
    return m_LastValue < m_CriticalValue;
  }

  bool low () {
    return m_LastValue < m_LowValue;
  }

  void init(uint8_t low,uint32_t period) {
    m_LowValue = low;
    m_LastValue = voltage();
    m_Period = period;
    tick = m_Period;
    aclock.add(*this);
  }

  virtual uint8_t voltage() {
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the internal 1.1V reference
    if (ADMUX != ADMUX_VCCWRT1V1) {
      ADMUX = ADMUX_VCCWRT1V1;
      // Bandgap reference start-up time: max 70us
      // Wait for Vref to settle.
      delayMicroseconds(350);
    }
    // Start conversion and wait for it to finish.
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC)) {};
    // Result is now stored in ADC.
    // Calculate Vcc (in V)
    uint16_t vcc = 1100UL * 1023 / ADC / 100;
    DPRINT(F("Bat: ")); DDECLN(vcc);
    return (uint8_t) vcc;
  }


};

/**
 * Measure battery voltage as used on the universal sensor board.
 */
class BatterySensorUni : public BatterySensor {
  uint8_t  m_SensePin; // A1
  uint8_t  m_ActivationPin; // D7
  uint8_t  m_Factor; // 57 = 470k + 100k / 10
public:

  BatterySensorUni (uint8_t sens,uint8_t activation) : BatterySensor(),
  m_SensePin(sens), m_ActivationPin(activation), m_Factor(57) {}
  virtual ~BatterySensorUni () {}

  void init( uint8_t low,uint32_t period,uint8_t factor=57) {
    m_Factor=factor;
    pinMode(m_SensePin, INPUT);
    pinMode(m_ActivationPin, INPUT);
    BatterySensor::init(low,period);
  }

  virtual uint8_t voltage () {
    pinMode(m_ActivationPin,OUTPUT);
    digitalWrite(m_ActivationPin,LOW);
    digitalWrite(m_SensePin,LOW);

    analogRead(m_SensePin);
    delay(2); // allow the ADC to stabilize
    uint16_t value = analogRead(m_SensePin);
    uint16_t vcc = (value * 3300UL * m_Factor) / 1024 / 1000;

    pinMode(m_ActivationPin,INPUT);
    digitalWrite(m_SensePin,HIGH);

    DPRINT(F("Bat: ")); DDECLN(vcc);
    return (uint8_t)vcc;
  }
};

/**
 * Measure on analog pin
 * See https://github.com/rlogiacco/BatterySense for setup
 */
class BatterySensorExt : public BatterySensor {
  uint8_t  m_SensePin;
  uint8_t  m_ActivationPin;
  uint8_t  m_DividerRatio;
  uint16_t m_RefVoltage;
public:

  BatterySensorExt (uint8_t sens,uint8_t activation=0xff) : BatterySensor(),
    m_SensePin(sens), m_ActivationPin(activation), m_DividerRatio(2), m_RefVoltage(3300) {}
  virtual ~BatterySensorExt () {}

  void init( uint8_t low,uint32_t period,uint16_t refvolt=3300,uint8_t divider=2) {
    m_DividerRatio=divider;
    m_RefVoltage = refvolt;
    pinMode(m_SensePin, INPUT);
    if (m_ActivationPin < 0xFF) {
      pinMode(m_ActivationPin, OUTPUT);
    }
    BatterySensor::init(low,period);
  }


  virtual uint8_t voltage () {
    if (m_ActivationPin != 0xFF) {
        digitalWrite(m_ActivationPin, HIGH);
        delayMicroseconds(10); // copes with slow switching activation circuits
      }
      analogRead(m_SensePin);
      delay(2); // allow the ADC to stabilize
      uint32_t value = analogRead(m_SensePin);
      uint16_t vcc = (value * m_DividerRatio * m_RefVoltage) / 1024 / 100;
      if (m_ActivationPin != 0xFF) {
        digitalWrite(m_ActivationPin, LOW);
      }
      DPRINT(F("Bat: ")); DDECLN(vcc);
      return (uint8_t)vcc;
  }

};

}

#endif
