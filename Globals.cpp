
#include <Arduino.h>

namespace as {

void* __gb_store;
void* __gb_radio;

const char* __gb_chartable = "0123456789ABCDEF";

uint16_t __gb_BatCurrent = 0;
void (*__gb_BatIrq)() = 0;

}
