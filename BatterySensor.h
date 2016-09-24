
#ifndef __BATTERYSENSOR_H__
#define __BATTERYSENSOR_H__

#include <Debug.h>
#include <AlarmClock.h>

#include <avr/power.h>

#define BAT_NUM_MESS_ADC                  20                // real measures to get the best average measure
#define BAT_DUMMY_NUM_MESS_ADC            40                // dummy measures to get the ADC working

#define AVR_BANDGAP_VOLTAGE     1100UL                      // band gap reference for Atmega328p

namespace as {

class BatterySensor : public Alarm {

  uint8_t  m_LowValue;
  uint8_t  m_LastValue;
  uint32_t m_Period;

public:
  BatterySensor () : Alarm(0), m_LowValue(0), m_LastValue(0), m_Period() {}
  virtual ~BatterySensor() {}

  virtual void trigger (AlarmClock& clock) {
    tick = m_Period;
    clock.add(*this);
    m_LastValue = voltage();
  }

  bool low () {
    return m_LastValue <= m_LowValue;
  }

  void init(uint8_t low,uint32_t period) {
    m_LowValue = low;
    m_LastValue = voltage();
    m_Period = period;
    tick = m_Period;
    aclock.add(*this);
  }

  uint8_t voltage(void) {
    uint16_t adcValue = getAdcValue(                    // Voltage Reference = AVCC with external capacitor at AREF pin; Input Channel = 1.1V (V BG)
        (0 << REFS1) | (1 << REFS0) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0)
    );
    adcValue = (AVR_BANDGAP_VOLTAGE * 1023) / adcValue / 100;         // calculate battery voltage in V/10
    DPRINT(F("Bat: ")); DHEXLN(adcValue);
    return (uint8_t)adcValue;
  }

protected:
  uint16_t getAdcValue(uint8_t adcmux) {
    uint16_t adcValue = 0;

    #if defined(__AVR_ATmega32U4__)                       // save content of Power Reduction Register
      uint8_t tmpPRR0 = PRR0;
      uint8_t tmpPRR1 = PRR1;
    #else
      uint8_t tmpPRR = PRR;
    #endif
    power_adc_enable();

    ADMUX = adcmux;                               // start ADC
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);             // Enable ADC and set ADC pre scaler

    for (uint8_t i = 0; i < BAT_NUM_MESS_ADC + BAT_DUMMY_NUM_MESS_ADC; i++) { // take samples in a round
      ADCSRA |= (1 << ADSC);                          // start conversion
      while (ADCSRA & (1 << ADSC)) {}                     // wait for conversion complete

      if (i >= BAT_DUMMY_NUM_MESS_ADC) {                    // we discard the first dummy measurements
        adcValue += ADCW;
      }
    }

    ADCSRA &= ~(1 << ADEN);                           // ADC disable
    adcValue = adcValue / BAT_NUM_MESS_ADC;                   // divide adcValue by amount of measurements

    #if defined(__AVR_ATmega32U4__)                       // restore power management
      PRR0 = tmpPRR0;
      PRR1 = tmpPRR1;
    #else
      PRR = tmpPRR;
    #endif

    ADCSRA = 0;                                 // ADC off

    return adcValue;                              // return the measured value
  }

};

}

#endif
