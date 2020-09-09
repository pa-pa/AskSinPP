
#include <AskSinPP.h>
#include <BatterySensor.h>

namespace as {

void* __gb_store;
void* __gb_radio;

const char* __gb_chartable = "0123456789ABCDEF";

void(* resetFunc) (void) = 0;

#ifdef ARDUINO_ARCH_AVR
volatile uint16_t IrqBaseBatt::__gb_BatCurrent = 0;
volatile uint8_t IrqBaseBatt::__gb_BatCount = 0;
uint16_t (*IrqBaseBatt::__gb_BatIrq)() = 0;

void IrqBaseBatt::__vectorfunc() {
  if( __gb_BatIrq != 0 ) {
    __gb_BatCount++;
    if( __gb_BatCount > 10 ) { // ignore first 10 values
      uint16_t v = __gb_BatIrq();
      if( __gb_BatCurrent == 0 ) {
        __gb_BatCurrent = v;
      }
      else {
        if( v < __gb_BatCurrent ) {
          __gb_BatCurrent = v;
        }
      }
    }

    if( __gb_BatIrq != 0 )
      ADCSRA |= (1 << ADSC);        // start conversion again
  }
}
#endif

}
