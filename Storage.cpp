//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#include "Storage.h"

namespace as {

Storage storage;


bool Storage::setup (uint16_t checksum) {
  bool firststart = false;
  uint32_t mem;
  getData(0x0,(uint8_t*)&mem,4);
  uint32_t magic = 0xCAFE0000 | checksum;
  if(magic != mem) {
    DPRINT(F("Init Storage: "));
    DHEXLN(magic);
    // init eeprom
    setData(0x0,(uint8_t*)&magic,4);
    firststart = true;
  }
  return firststart;
}

void Storage::dump (uint16_t start, uint16_t num) {
  for( uint16_t i=0; i<num; ++i ) {
    DHEX(getByte(start + i));
    DPRINT(F(" "));
  }
  DPRINT(F("\n"));
}

}
