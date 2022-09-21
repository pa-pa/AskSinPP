//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __BATTERYSENSOR_H__
#define __BATTERYSENSOR_H__

#include "Debug.h"
#include <AlarmClock.h>

#ifdef ARDUINO_ARCH_AVR

#ifndef __AVR_ATmega128__
#include <avr/power.h>
#endif

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

/**
 * Fake Battery class - empty implementation of all methods. This class can be used
 * if battery measurement is not needed.
 */
class NoBattery {
public:
  /// get current battery voltage - returns always 3.3
  uint8_t current () const { return 33; }
  /// check if battery voltage is below critical value - returns always false
  bool critical () const { return false; }
  /// set critical value - do nothing
  void critical (__attribute__((unused)) uint8_t value ) {}
  /// check if battery voltage is below low value - returns always false
  bool low () const { return false; }
  /// set low value - do nothing
  void low (__attribute__((unused)) uint8_t value ) {}
  /// called when systems enter idle state
  void setIdle () {}
  /// called when systems returns from idle state
  void unsetIdle () {}
  /// reset current battery value
  void resetCurrent () {}
};

#ifdef ARDUINO_ARCH_AVR

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
#define ADMUX_ADCMASK  ((1 << MUX4)|(1 << MUX3)|(1 << MUX2)|(1 << MUX1)|(1 << MUX0))
#else
#define ADMUX_ADCMASK  ((1 << MUX3)|(1 << MUX2)|(1 << MUX1)|(1 << MUX0))
#endif
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
#ifdef ARDUINO_ARCH_EFM32
    CMU_ClockEnable(cmuClock_ADC0, true);
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
#elif defined ARDUINO_ARCH_STM32 && defined STM32L1xx
    analogReadResolution(12);
    vcc = 1216 * 4096 / analogRead(AVREF);
#elif defined ARDUINO_ARCH_EFM32
    ADC_Init_TypeDef       init       = ADC_INIT_DEFAULT;
    ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;
    init.timebase = ADC_TimebaseCalc(0);
    init.prescale = ADC_PrescaleCalc(ADC_CLOCK, 0);

    /* Set oversampling rate */
    init.ovsRateSel = adcOvsRateSel256;

    ADC_Init(ADC0, &init);
    /* Init for single conversion, measure VDD/3 with 1.25V reference. */
    singleInit.input = adcSingleInpVDDDiv3;
    /* The datasheet specifies a minimum aquisition time when sampling VDD/3 */
    /* 32 cycles should be safe for all ADC clock frequencies */
    singleInit.acqTime = adcAcqTime32;
    /* Enable oversampling rate */
    singleInit.resolution = adcResOVS;
    ADC_InitSingle(ADC0, &singleInit);

    ADC_Start(ADC0, adcStartSingle);

    /* Wait while conversion is active */
    while (ADC0->STATUS & ADC_STATUS_SINGLEACT)
      ;
    /* Get ADC result */
    volatile uint16_t sampleValue = ADC_DataSingleGet(ADC0);

    /* Calculate supply voltage relative to 1.25V reference */
    vcc = (sampleValue * 1250 * 3) / ADC_16BIT_MAX;

    ADC_Reset(ADC0);
#endif
    DPRINT(F("iVcc: ")); DDECLN(vcc);
    return vcc;
  }
};

#ifdef ARDUINO_ARCH_EFM32
template<ADC_SingleInput_TypeDef SENS_CHANNEL>
class ExternalVCCEFM32 : public InternalVCC {
public:
  static const int DefaultDelay = 250;

  void init () {
    CMU_ClockEnable(cmuClock_ADC0, true);
    ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
    init.timebase = ADC_TimebaseCalc(0);
    init.prescale = ADC_PrescaleCalc(7000000, 0);
    ADC_Init(ADC0, &init);
  }

  void start () {
    ADC_InitSingle_TypeDef sInit = ADC_INITSINGLE_DEFAULT;
    sInit.input = SENS_CHANNEL;
    sInit.reference = adcRef2V5; //adcRefVDD;
    sInit.acqTime = adcAcqTime32;
    ADC_InitSingle(ADC0, &sInit);
    ADC_Start(ADC0, adcStartSingle);
  }

  uint16_t finish () {
    while ( ADC0->STATUS & ADC_STATUS_SINGLEACT);
    uint32_t value = ADC_DataSingleGet(ADC0);
    uint16_t vin = (value * 2500UL) / 4096UL;
    DPRINT(F("eVcc: ")); DDECLN(vin);
    return vin;
  }
};
#endif

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

  void resetCurrent() { m_Value = 0; }

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

  void resetCurrent() { m_Value = 0; }

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
  void resetCurrent() { m_Meter.resetCurrent(); }

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

  void setIdle () {}
  void unsetIdle () {}
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

#ifdef ARDUINO_ARCH_AVR

class IrqBaseBatt {
protected:
  /// value for low battery
  uint8_t m_Low;
  /// value for critical battery
  uint8_t m_Critical;

  static volatile uint16_t __gb_BatCurrent;
  static volatile uint8_t __gb_BatCount;
  static uint16_t (*__gb_BatIrq)();
  uint8_t m_BatSkip;

  IrqBaseBatt () : m_Low(0), m_Critical(0), m_BatSkip(0) {}
  ~IrqBaseBatt () {}

public:
  /** get current battery voltage value
   * \return the current battery value multiplied by 10
   */
  uint8_t current () const { return (__gb_BatCurrent + 50) / 100; }
  /** check if the battery is below critical value
   *  \return true if battery voltage below critical value
   */
  bool critical () const { return current() < m_Critical; }
  /** set critical battery value
   * \param value critical battery value
   */
  void critical (uint8_t value ) { m_Critical = value; }
  /** check if the battery is below low value
   *  \return true if battery voltage below low value
   */
  bool low () const { return current() < m_Low; }
  /** set low battery value
   * \param value low battery value
   */
  void low (uint8_t value ) { m_Low = value; }

  /// for backward compatibility
  uint16_t voltageHighRes() { return __gb_BatCurrent; }
  /// for backward compatibility
  uint8_t voltage() { return current(); }

  /// reset current battery value
  void resetCurrent() { __gb_BatCurrent = 0; }

protected:
  /**
   * Disable the continues battery measurement
   * Called by HAL before enter idle/sleep state
   * Call this before your application code uses the ADC.
   */
  void setIdle () {
    if( __gb_BatCount < 10 ) {
      // if we skip to often - force reading
      if( ++m_BatSkip > 10 ) {
        // wait for valid bat value
        while( __gb_BatCount++ < 10 ) {
          while (ADCSRA & (1 << ADSC)) ; // wait ADC finish
          ADCSRA |= (1 << ADSC);         // start conversion again
        }
        m_BatSkip = 0;
      }
    }
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
      __gb_BatIrq = 0;
    }

    ADCSRA &= ~((1 << ADIE) | (1 << ADIF));  // disable interrupt
    while (ADCSRA & (1 << ADSC)) ;  // wait finish
    __vectorfunc(); // ensure value is read and stored
  }

  void unsetIdle (uint16_t (*irqfunc)()) {
    //DDECLN(__gb_BatCurrent);
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
      __gb_BatCount = 0; // reset irq counter
      __gb_BatIrq = irqfunc; // set irq method
    }
    ADMUX &= ~(ADMUX_REFMASK | ADMUX_ADCMASK);
    ADMUX |= ADMUX_REF_AVCC;      // select AVCC as reference
    ADMUX |= ADMUX_ADC_VBG;       // measure bandgap reference voltage
    ADCSRA |= (1 << ADIE) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2); // enable interrupt & 128 prescaler
    ADCSRA |= (1 << ADSC);        // start conversion
  }

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
  static void __vectorfunc() __asm__("__vector_24")  __attribute__((__signal__, __used__, __externally_visible__));
#else
  static void __vectorfunc() __asm__("__vector_21")  __attribute__((__signal__, __used__, __externally_visible__));
#endif
};

/**
 * IrqInternalBatt class uses continue measurement in background.
 * It uses the ADC and IRQ to get battery voltage during normal operation. If a device needs to sample
 * analog values, it has to call setIdle() before and unsetIdle() after analogRead().
 */
class IrqInternalBatt : public IrqBaseBatt {

public:
  /** Constructor
   */
  IrqInternalBatt () {}
  /** Destructor
   */
  ~IrqInternalBatt() {}
  /** init measurement with period and used clock
   * \param period ticks until next measurement
   * \param clock clock to use for waiting
   */
  void init() {
    unsetIdle();
  }
  
  void init(__attribute__((unused)) uint32_t period,__attribute__((unused)) AlarmClock& clock) {
    init();
  }

  
  /**
   * Disable the continues battery measurement
   * Called by HAL before enter idle/sleep state
   * Call this before your application code uses the ADC.
   */
  void setIdle () {
    IrqBaseBatt::setIdle();
  }
  /**
   * Enable the continues measurement of the battery voltage
   * Called by HAL after return from idle/sleep state
   * Call this after the application doesn't need ADC longer
   */
  void unsetIdle () {
    IrqBaseBatt::unsetIdle(irq);
    // wait for stable values
    /*
    int maxnum = 50;  // we will wait max 50
    uint16_t last=0 ,current=0;
    do {
      last = current;
      while (ADCSRA & (1 << ADSC)) ; // wait ADC finish
      current = ADC >> 2; // remove some bits ???
    } while( current != last && --maxnum > 0);
    */
  }
  /** ISR function to get current measured value
   */
  static uint16_t irq () {
      return 1100UL * 1024 / ADC;
  }

};

extern volatile uint16_t intVCC;
template <uint8_t SENSPIN,uint8_t ACTIVATIONPIN,uint8_t FACTOR=57>
class IrqExternalBatt :  public IrqBaseBatt {
public:
  /** Constructor
   */
  IrqExternalBatt () {}
  /** Destructor
   */
  ~IrqExternalBatt() {}
  /** init measurement with period and used clock
   * \param period ticks until next measurement
   * \param clock clock to use for waiting
   */
   
  void init() {
    pinMode(SENSPIN, INPUT);
    unsetIdle();
  }
  
  void init(__attribute__((unused)) uint32_t period,__attribute__((unused)) AlarmClock& clock) {
    init();
  }
  
  uint16_t getInternalVcc() {
    //read internal Vcc as reference voltage
    ADMUX &= ~(ADMUX_REFMASK | ADMUX_ADCMASK);
    ADMUX |= ADMUX_REF_AVCC;      // select AVCC as reference
    ADMUX |= ADMUX_ADC_VBG;       // measure bandgap reference voltage
    _delay_us(350);
    ADCSRA |= (1 << ADSC);         // start conversion
    while (ADCSRA & (1 << ADSC)) ; // wait to finish
    return 1100UL * 1024 / ADC;
  }

  /**
   * Disable the continues battery measurement
   * Called by HAL before enter idle/sleep state
   * Call this before your application code uses the ADC.
   */
  void setIdle () {
    IrqBaseBatt::setIdle();
    pinMode(ACTIVATIONPIN, INPUT);
  }
  /**
   * Enable the continues measurement of the battery voltage
   * Called by HAL after return from idle/sleep state
   * Call this after the application doesn't need ADC longer
   */

#ifndef analogPinToChannel
 #define analogPinToChannel(p) ((p)-14)
#endif

  void unsetIdle () {
    pinMode(ACTIVATIONPIN, OUTPUT);
    digitalWrite(ACTIVATIONPIN, LOW);
   // _delay_ms(5);
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
      __gb_BatCount = 0; // reset irq counter
      __gb_BatIrq = irq; // set irq method
    }

    intVCC = getInternalVcc();

    ADMUX &= ~(ADMUX_REFMASK | ADMUX_ADCMASK);
    ADMUX |= ADMUX_REF_AVCC;    // select AVCC as reference
    ADMUX |= analogPinToChannel(SENSPIN);  // select channel
    ADCSRA |= (1 << ADIE) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2); // enable interrupt & 128 prescaler
    ADCSRA |= (1 << ADSC);        // start conversion*/
  }
  /** ISR function to get current measured value
   */
  static uint16_t irq () {
    return 1UL * intVCC * FACTOR * ADC / 1024 / 10;
  }
};

#endif

}

#endif
