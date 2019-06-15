//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __BATTERYSENSOR_H__
#define __BATTERYSENSOR_H__

#include <Debug.h>
#include <AlarmClock.h>

#ifdef ARDUINO_ARCH_AVR

#include <avr/power.h>

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
#define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
#define ADMUX_VCCWRT1V1 (_BV(MUX5) | _BV(MUX0))
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define ADMUX_VCCWRT1V1 (_BV(MUX3) | _BV(MUX2))
#else
#define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
#endif

#endif


namespace as {

class NoBattery {
public:
  uint8_t current () const { return 0; }
  bool critical () const { return false; }
  bool low () const { return false; }
};

#ifdef ARDUINO_ARCH_AVR

#define ADMUX_ADCMASK  ((1 << MUX3)|(1 << MUX2)|(1 << MUX1)|(1 << MUX0))
#define ADMUX_REFMASK  ((1 << REFS1)|(1 << REFS0))

#define ADMUX_REF_AREF ((0 << REFS1)|(0 << REFS0))
#define ADMUX_REF_AVCC ((0 << REFS1)|(1 << REFS0))
#define ADMUX_REF_RESV ((1 << REFS1)|(0 << REFS0))
#define ADMUX_REF_VBG  ((1 << REFS1)|(1 << REFS0))

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
#define ADMUX_ADC_VBG  ((1 << MUX4)|(1 << MUX3)|(1 << MUX2)|(1 << MUX1)|(0 << MUX0))
#else
#define ADMUX_ADC_VBG  ((1 << MUX3)|(1 << MUX2)|(1 << MUX1)|(0 << MUX0))
#endif

#endif


class InternalVCC {
public:
  typedef uint16_t ValueType;
  static const int DefaultDelay = 0;

  void init () {
#ifdef ARDUINO_ARCH_STM32F1
    adc_reg_map *regs = ADC1->regs;
    regs->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
    regs->SMPR1 =  ADC_SMPR1_SMP17;  // sample rate for VREFINT ADC channel
#endif
  }

  void start () {}

  uint16_t finish () {
    uint16_t vcc=0;
#ifdef ARDUINO_ARCH_AVR
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the internal 1.1V reference
    ADMUX &= ~(ADMUX_REFMASK | ADMUX_ADCMASK);
    ADMUX |= ADMUX_REF_AVCC;      // select AVCC as reference
    ADMUX |= ADMUX_ADC_VBG;       // measure bandgap reference voltage
    _delay_us(350);
    ADCSRA |= (1 << ADSC);         // start conversion
    while (ADCSRA & (1 << ADSC)) ; // wait to finish
    vcc = 1100UL * 1024 / ADC;
#elif defined ARDUINO_ARCH_STM32F1
    vcc = 1200 * 4096 / adc_read(ADC1, 17);  // ADC sample to millivolts
#endif
    DPRINT(F("iVcc: ")); DDECLN(vcc);
    return vcc;
  }
};

template<uint8_t SENSPIN, uint8_t ACTIVATIONPIN, uint8_t ACTIVATIONSTATE=LOW, uint16_t VCC=3300, uint8_t FACTOR=57>
class ExternalVCC : public InternalVCC {
public:
  static const int DefaultDelay = 250;

  void init () {
    pinMode(SENSPIN, INPUT);
    pinMode(ACTIVATIONPIN, INPUT);
  }

  void start () {
    pinMode(ACTIVATIONPIN, OUTPUT);
    digitalWrite(ACTIVATIONPIN, ACTIVATIONSTATE==LOW ? LOW : HIGH);
    digitalWrite(SENSPIN,LOW);
//    analogRead(SENSPIN);
  }

  uint16_t finish () {
    uint32_t value = analogRead(SENSPIN);
    digitalWrite(SENSPIN,HIGH);
    digitalWrite(ACTIVATIONPIN, ACTIVATIONSTATE==LOW ? HIGH : LOW);
    pinMode(ACTIVATIONPIN,INPUT);

    uint16_t refvcc = VCC;
    if( refvcc == 0 ) {
      InternalVCC::start(); // in case we add something here later
      refvcc = InternalVCC::finish();
    }
    uint16_t vin = (value * refvcc * FACTOR) / 1024 / 10;

    DPRINT(F("eVcc: ")); DDECLN(vin);
    return vin;
  }
};


template <class SENSOR,int DELAY=SENSOR::DefaultDelay>
class SyncMeter {
  SENSOR m_Sensor;
  volatile typename SENSOR::ValueType m_Value;
public:
  SyncMeter () : m_Value(0) {}
  void start () {
    sensor().start();
    if( DELAY > 0 ) {
      _delay_ms(DELAY);
    }
    typename SENSOR::ValueType tmp = sensor().finish();
    if( m_Value == 0 || tmp < m_Value ) {
      m_Value = tmp;
    }
  }
  typename SENSOR::ValueType value () const { return m_Value; }
  SENSOR& sensor () { return m_Sensor; }
  typename SENSOR::ValueType measure () {
    start();
    return value();
  }
};

template <class SENSOR,int DELAY=SENSOR::DefaultDelay>
class AsyncMeter : public Alarm {
  SENSOR m_Sensor;
  volatile typename SENSOR::ValueType m_Value;
public:
  AsyncMeter () : Alarm(0), m_Value(0) {}
  virtual ~AsyncMeter () {}
  virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
    typename SENSOR::ValueType tmp = sensor().finish();
    if( m_Value == 0 || tmp < m_Value ) {
      m_Value = tmp;
    }
  }
  void start () {
    sensor().start();
    set(millis2ticks(DELAY));
    sysclock.add(*this);
  }
  typename SENSOR::ValueType value () const { return m_Value; }
  SENSOR& sensor () { return m_Sensor; }
  typename SENSOR::ValueType measure () {
    sensor().start();
    _delay_ms(DELAY);
    m_Value = sensor().finish();
    return value();
  }
};

template <class METER>
class BattSensor : public Alarm {
  uint32_t  m_Period;
  uint8_t   m_Low, m_Critical;
  METER     m_Meter;
public:
  BattSensor () : Alarm(0), m_Period(0), m_Low(0), m_Critical(0) {}
  virtual ~BattSensor() {}

  virtual void trigger (AlarmClock& clock) {
    tick = m_Period;
    clock.add(*this);
    m_Meter.start();
  }

  uint8_t current () const { return (m_Meter.value() + 50) / 100; }
  bool critical () const { return current() < m_Critical; }
  void critical (uint8_t value ) { m_Critical = value; }
  bool low () const { return current() < m_Low; }
  void low (uint8_t value ) { m_Low = value; }

  void init(uint32_t period,AlarmClock& clock) {
    m_Meter.sensor().init();
    m_Meter.measure();
    m_Period = period;
    set(m_Period);
    clock.add(*this);
  }
  // for backward compatibility
  uint16_t voltageHighRes() { return m_Meter.value(); }
  uint8_t voltage() { return current(); }

  METER& meter () { return m_Meter; }
};

typedef BattSensor<SyncMeter<InternalVCC> > BatterySensor;

template <uint8_t SENSPIN,uint8_t ACTIVATIONPIN,uint16_t VCC=3300>
class BatterySensorUni : public BattSensor<SyncMeter<ExternalVCC<SENSPIN,ACTIVATIONPIN,LOW,VCC,57> > > {
public:
  BatterySensorUni () {}
  virtual ~BatterySensorUni () {}
};


/**
 * Measure on analog pin
 * See https://github.com/rlogiacco/BatterySense for setup
 */
template <uint8_t SENSPIN,uint8_t ACTIVATIONPIN,uint16_t VCC=3300>
class BatterySensorExt : public BatterySensor {
  uint8_t  m_SensePin;
  uint8_t  m_ActivationPin;
  uint8_t  m_DividerRatio;
  uint16_t m_RefVoltage;
public:

  BatterySensorExt () : BatterySensor (),
    m_SensePin(SENSPIN), m_ActivationPin(ACTIVATIONPIN), m_DividerRatio(2), m_RefVoltage(VCC) {}
  virtual ~BatterySensorExt () {}

  void init(uint32_t period,AlarmClock& clock,uint16_t refvolt=VCC,uint8_t divider=2) {
    m_DividerRatio=divider;
    m_RefVoltage = refvolt;
    pinMode(m_SensePin, INPUT);
    if (m_ActivationPin < 0xFF) {
      pinMode(m_ActivationPin, OUTPUT);
    }
    BatterySensor::init(period,clock);
  }


  virtual uint8_t voltage () {
    if (m_ActivationPin != 0xFF) {
        digitalWrite(m_ActivationPin, HIGH);
        _delay_us(10); // copes with slow switching activation circuits
      }
      analogRead(m_SensePin);
      _delay_ms(2); // allow the ADC to stabilize
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
