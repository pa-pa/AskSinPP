//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- AskSin cc1101 functions -----------------------------------------------------------------------------------------------
//- with a lot of copy and paste from culfw firmware
//- -----------------------------------------------------------------------------------------------------------------------
//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "Radio.h"

//- Pro Mini cc1100 hardware definitions ---------------------------------------------------------------------------------------------
#define CC_CS_PINNR 10
#define CC_MOSI_PINNR 11
#define CC_MISO_PINNR 12
#define CC_SCLK_PINNR 13
#define CC_GDO0_PINNR 2

namespace as {

void radioISR();

static CC1101* cc;

//- cc1100 hardware functions ---------------------------------------------------------------------------------------------
void CC1101::ccInitHw(void) {
  cc = this;
  pinMode(CC_CS_PINNR,OUTPUT);
  pinMode(CC_MOSI_PINNR,OUTPUT);
  pinMode(CC_MISO_PINNR,INPUT);
  pinMode(CC_SCLK_PINNR,OUTPUT);
  pinMode(CC_GDO0_PINNR,INPUT);

  SPCR = _BV(SPE) | _BV(MSTR);                        // SPI enable, master, speed = CLK/4
}
uint8_t CC1101::ccSendByte(uint8_t data) {
  SPDR = data;                                // send byte
  while (!(SPSR & _BV(SPIF)));                        // wait until transfer finished
  return SPDR;
}


uint8_t CC1101::getGDO0 () {
  return digitalRead(CC_GDO0_PINNR);
}

void CC1101::enableGDO0Int(void) {
  ::attachInterrupt(digitalPinToInterrupt(CC_GDO0_PINNR), radioISR, FALLING);
}

void CC1101::disableGDO0Int(void) {
  ::detachInterrupt(0);
}

void CC1101::waitMiso(void) {
  while(digitalRead(CC_MISO_PINNR));
}
void CC1101::ccSelect(void) {
  digitalWrite(CC_CS_PINNR,LOW);
}
void CC1101::ccDeselect(void) {
  digitalWrite(CC_CS_PINNR,HIGH);
}
//- -----------------------------------------------------------------------------------------------------------------------
void radioISR(void) {
  cc->handleGDO0Int();
}

}
