
#include <AskSinPP.h>
#include <BatterySensor.h>

namespace as {

void* __gb_store;
void* __gb_radio;

const char* __gb_chartable = "0123456789ABCDEF";

void(* resetFunc) (void) = 0;

#ifdef ARDUINO_ARCH_AVR
volatile uint16_t IrqInternalBatt::__gb_BatCurrent = 0;
volatile uint8_t IrqInternalBatt::__gb_BatCount = 0;
void (*IrqInternalBatt::__gb_BatIrq)() = 0;

void IrqInternalBatt::vecfunc() {
  if( __gb_BatIrq != 0 ) {
    __gb_BatIrq();
  }
}
#endif


}
