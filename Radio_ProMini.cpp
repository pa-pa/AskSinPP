
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "Radio.h"

#define pinOutput(PORT,PIN)  ((PORT) |=  _BV(PIN))              // pin functions
#define pinInput(PORT,PIN)   ((PORT) &= ~_BV(PIN))
#define setPinHigh(PORT,PIN) ((PORT) |=  _BV(PIN))
#define setPinLow(PORT,PIN)  ((PORT) &= ~_BV(PIN))
#define setPinCng(PORT,PIN)  ((PORT) ^= _BV(PIN))
#define getPin(PORT,PIN)     ((PORT) &  _BV(PIN))


#define CC_CS_DDR              DDRB                   // SPI chip select definition
#define CC_CS_PORT             PORTB
#define CC_CS_PIN              PORTB2

#define CC_GDO0_DDR            DDRD                   // GDO0 pin, signals received data
#define CC_GDO0_PORT           PORTD
#define CC_GDO0_PIN            PORTB2

#define CC_GDO0_PCICR          PCICR                  // GDO0 interrupt register
#define CC_GDO0_PCIE           PCIE2
#define CC_GDO0_PCMSK          PCMSK2                 // GDO0 interrupt mask
#define CC_GDO0_INT            PCINT18                  // pin interrupt


//- atmega328 cc1100 hardware definitions ---------------------------------------------------------------------------------------------
#define SPI_PORT                PORTB                     // SPI port definition
#define SPI_DDR                 DDRB
#define SPI_MISO                PORTB4
#define SPI_MOSI                PORTB3
#define SPI_SCLK                PORTB5

namespace as {

void radioISR();

//- cc1100 hardware functions ---------------------------------------------------------------------------------------------
void CC1101::ccInitHw(void) {
  pinOutput( CC_CS_DDR, CC_CS_PIN );                      // set chip select as output
  pinOutput( SPI_DDR, SPI_MOSI );                       // set MOSI as output
  pinInput ( SPI_DDR, SPI_MISO );                       // set MISO as input
  pinOutput( SPI_DDR, SPI_SCLK );                       // set SCK as output
  pinInput ( CC_GDO0_DDR, CC_GDO0_PIN );                    // set GDO0 as input

  SPCR = _BV(SPE) | _BV(MSTR);                        // SPI enable, master, speed = CLK/4

  CC_GDO0_PCICR |= _BV(CC_GDO0_PCIE);                     // set interrupt in mask active
}
uint8_t CC1101::ccSendByte(uint8_t data) {
  SPDR = data;                                // send byte
  while (!(SPSR & _BV(SPIF)));                        // wait until transfer finished
  return SPDR;
}


uint8_t CC1101::getGDO0 () {
  return getPin(CC_GDO0_PORT,CC_GDO0_PIN);
}

void CC1101::enableGDO0Int(void) {
  ::attachInterrupt(0, radioISR, FALLING);
}

void CC1101::disableGDO0Int(void) {
  ::detachInterrupt(0);
}

void CC1101::waitMiso(void) {
  while(SPI_PORT &   _BV(SPI_MISO));
}
void CC1101::ccSelect(void) {
  setPinLow( CC_CS_PORT, CC_CS_PIN);
}
void CC1101::ccDeselect(void) {
  setPinHigh( CC_CS_PORT, CC_CS_PIN);
}
//- -----------------------------------------------------------------------------------------------------------------------

void radioISR(void) {
  radio.handleGDO0Int();
}

}
