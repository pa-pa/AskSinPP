//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2020-01-03 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
// Datasheet:
// https://ams.com/documents/20143/36005/AS5600_DS000365_5-00.pdf

#ifndef SENSORS_AS5600_H_
#define SENSORS_AS5600_H_

#include <Wire.h>

#define AS5600ADDRESS       0x36
#define ZMCOADDRESS         0x00
#define ZPOSADDRESSMSB      0x01
#define ZPOSADDRESSLSB      0x02
#define MPOSADDRESSMSB      0x03
#define MPOSADDRESSLSB      0x04
#define MANGADDRESSMSB      0x05
#define MANGADDRESSLSB      0x06
#define CONFADDRESSMSB      0x07
#define CONFADDRESSLSB      0x08
#define RAWANGLEADDRESSMSB  0x0C
#define RAWANGLEADDRESSLSB  0x0D
#define ANGLEADDRESSMSB     0x0E
#define ANGLEADDRESSLSB     0x0F
#define STATUSADDRESS       0x0B
#define AGCADDRESS          0x1A
#define MAGNITUDEADDRESSMSB 0x1B
#define MAGNITUDEADDRESSLSB 0x1C
#define BURNADDRESS         0xFF

namespace as {

enum AS5600PowerMode {NOM = 0, LPM1, LPM2, LPM3 };

template <uint8_t pmode = NOM>
class As5600  {

private:
  uint16_t  _angle;
  int16_t   _raw;
  bool      _present;
  uint8_t   _status;

  int _readSingleByte(uint8_t adr) {
    Wire.beginTransmission(AS5600ADDRESS);
    Wire.write(adr);
    Wire.endTransmission();
    delay(2);
    Wire.requestFrom(AS5600ADDRESS, 1);

    int ret = -1;
    if(Wire.available() <=1) {
      ret = Wire.read();
    }

    return ret;
  }

  int _readTwoBytes(uint8_t regMSB, uint8_t regLSB)
  {
    uint8_t _lsb = 0;
    uint8_t _msb = 0;

    _msb = _readSingleByte(regMSB);
    _lsb = _readSingleByte(regLSB);

    return (_msb << 8) | _lsb;
  }


  void _writeSingleByte(int adr_in, int dat_in)
  {
    Wire.beginTransmission(AS5600ADDRESS);
    Wire.write(adr_in);
    Wire.write(dat_in);
    Wire.endTransmission();
  }

public:

  As5600 () : _angle(0), _raw(0), _present(false), _status(0) {}

  uint8_t getConfigLo() {
    return _readSingleByte(CONFADDRESSLSB);
  }

  uint8_t getConfigHi() {
    return _readSingleByte(CONFADDRESSMSB);
  }

  void setPowerMode(uint8_t pm){
    if (pm <= LPM3) {
      uint8_t config = _readSingleByte(CONFADDRESSLSB);
      config &= ~((1 << 0) | (1 << 1));
      config |= pm;
      _writeSingleByte(CONFADDRESSLSB, config);
    } else {
      DPRINTLN(F("setPowerMode failed: wrong mode"));
    }
  }

  void setWatchDog(bool state) {
    uint8_t config = _readSingleByte(CONFADDRESSMSB);
    config &= ~(1 << 5);
    if (state) config |= 0x20;
    _writeSingleByte(CONFADDRESSMSB, config);
  }

  uint8_t getStatus() {
    return _readSingleByte(STATUSADDRESS) & 0b00111000;
  }

  uint8_t getAGC() {
    return _readSingleByte(AGCADDRESS);
  }

  void init () {
    Wire.begin();

    uint8_t status = getStatus();
    uint8_t agc    = getAGC();

    if (status == 0x20) {
      _present=true;
      setPowerMode(pmode);
      //setWatchDog(true);
      DPRINT(F("AS5600 OK. AGC: "));DDEC(agc);DPRINT(F(", CONFIG Lo: 0x"));DHEX(getConfigLo());DPRINT(", Hi: 0x");DHEXLN(getConfigHi());
    } else {
      DPRINT(F("AS5600 FAILURE. AGC: "));DDEC(agc);DPRINT(F(", Status: 0x"));DHEXLN(status);
      //0x08 = no Magnet detected, magnet too strong, AGC minimum gain overflow
      //0x10 = no Magnet detected, magnet too weak,   AGC maximum gain overflow
      //0x28 = Magnet detected, magnet too strong
      //0x30 = Magnet detected, magnet too weak
      //0x38 = no sensor
    }
  }

  void measure () {
    _status = getStatus();
    _present = (_status == 0x20);

    if (_present) {
       _raw = _readTwoBytes(RAWANGLEADDRESSMSB, RAWANGLEADDRESSLSB);
      _angle = (_raw > -1) ? map(_raw, 0, 4096, 0, 359) : 0xFFFF;
    }
  }

  uint16_t angle () { return _angle;  }
  int16_t  raw   () { return _raw;    }
  uint8_t  status() { return _status ;}
};

}



#endif /* SENSORS_AS5600_H_ */
