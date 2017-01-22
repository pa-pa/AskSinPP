//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#include "EEProm.h"

namespace as {

EEProm eeprom;

uint16_t EEProm::crc16 (uint16_t crc,uint8_t d) {
  crc ^= d;
  for( uint8_t i = 8; i != 0; --i ) {
    crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0 );
  }
  return crc;
}

bool EEProm::setup (uint16_t checksum) {
  bool firststart = false;
  uint32_t mem;
  getData(0x0,(uint8_t*)&mem,4);
  uint32_t magic = 0xCAFE0000 | checksum;
  if(magic != mem) {
    DPRINT(F("Init EEProm: "));
    DHEXLN(magic);
    // init eeprom
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

}
