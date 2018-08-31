//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_DS18B20_h__
#define __SENSORS_DS18B20_h__

#include <Sensors.h>
#if defined(ARDUINO_ARCH_STM32F1)
#include <OneWireSTM.h>
#else
#include <OneWire.h>
#endif

// https://www.tweaking4all.com/hardware/arduino/arduino-ds18b20-temperature-sensor/

namespace as {

class Ds18b20 : public Temperature {

public:
  /**
   * Scan the bus and init max sensors with the address.
   */
  static uint8_t init (OneWire& ow,Ds18b20* devs,uint8_t max) {
    uint8_t a[8], num=0;
    while( ow.search(a) == 1 && num < max) {
      if( OneWire::crc8(a,7) == a[7] ) {
        if( Ds18b20::valid(a) == true ) {
          devs->init(ow,a);
          ++num;
          ++devs;
        }
      }
    }
    return num;
  }
  /**
   * Measure all sensors in one step
   */
  static void measure (Ds18b20* devs,uint8_t count) {
    devs->convert(true); // this will trigger all DS18b20 on the bus
    devs->wait(); // this will also wait for all to be finish
    for( uint8_t num=0; num < count; ++num, ++devs ) {
      devs->read(); // read value for every device
    }
  }
  /**
   * Check if this is a supported 1Wire device
   */
  static bool valid(uint8_t* addr) {
    return *addr == 0x10 || *addr == 0x28 || *addr == 0x22;
  }

private:
  uint8_t  _addr[8];
  OneWire* _wire;

public:
  Ds18b20 () : _wire(0) {}

  void init (OneWire& ow,uint8_t* addr) {
    _wire = &ow;
    for( uint8_t i=0; i<8; ++i )
      _addr[i]=addr[i];
    _present = true;
  }

  void convert(__attribute__((unused)) bool kick=false) {
    _wire->reset();
    if( kick == true ) {
      _wire->skip();
    }
    else {
      _wire->select(_addr);
    }
    _wire->write(0x44);        // start conversion, use ds.write(0x44,1) with parasite power on at the end
  }

  void wait () {
    //delay(750);
    while (_wire->read() == 0) ;
  }

  void read () {
    _wire->reset();
    _wire->select(_addr);
    _wire->write(0xBE);         // Read Scratchpad

    uint8_t data[9];
    for (uint8_t i = 0; i < 9; i++) { // we need 9 bytes
      data[i] = _wire->read();
    }

    if (OneWire::crc8(data, 8) == data[8]) {
      int16_t raw = (data[1] << 8) | data[0];
      if (_addr[0] == 0x10) {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
          // "count remain" gives full 12 bit resolution
          raw = (raw & 0xFFF0) + 12 - data[6];
        }
      } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        //// default is 12 bit resolution, 750 ms conversion time
      }
      _temperature = (raw*10)/16;
    }
  }

  void measure (__attribute__((unused)) bool async=false) {
    if( _present == true ) {
      convert();
      wait();
      read();
    }
  }

};

}

#endif
