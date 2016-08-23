
#include "EEProm.h"

EEProm eeprom;

bool EEProm::setup () {
  bool firststart = false;
  uint32_t magic;
  getData(0x0,(uint8_t*)&magic,4);
  if(magic != 0x0CAFFEE0) {
    DPRINTLN(F("Init EEProm"));
    // init eeprom
    magic = 0x0CAFFEE0;
    setData(0x0,(uint8_t*)&magic,4);
    firststart = true;
  }
  return firststart;
}

void EEProm::dump (uint16_t start, uint16_t num) {
  for( uint16_t i=0; i<num; ++i ) {
    DHEX(getByte(start + i));
    DPRINT(" ");
  }
  DPRINT("\n");
}
