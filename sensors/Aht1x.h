//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2021-08-28 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_AHT1X_h__
#define __SENSORS_AHT1X_h__

#include <Arduino.h>
#include <Wire.h>
#include <Sensors.h>

#define AHT1X_INIT_CMD             0xE1
#define AHT1X_START_MEASURMENT_CMD 0xAC
#define AHT1X_NORMAL_CMD           0xA8
#define AHT1X_SOFT_RESET_CMD       0xBA

#define AHT1X_INIT_NORMAL_MODE     0x00
#define AHT1X_INIT_CYCLE_MODE      0x20
#define AHT1X_INIT_CMD_MODE        0x40
#define AHT1X_INIT_CAL_ENABLE      0x08

#define AHT1X_DATA_MEASURMENT_CMD  0x33
#define AHT1X_DATA_NOP             0x00

#define AHT1X_MEASURMENT_DELAY     80  
#define AHT1X_POWER_ON_DELAY       40  
#define AHT1X_CMD_DELAY            350 
#define AHT1X_SOFT_RESET_DELAY     20  

#define AHT1X_ERROR                0xFF


namespace as {

template <uint8_t ADDRESS = 0x38>
class Aht1x : public Temperature, public Humidity {
private:
  uint8_t  rawData[6];

  bool initNormalMode(void)  {
    Wire.beginTransmission(ADDRESS);

    Wire.write(AHT1X_NORMAL_CMD);
    Wire.write(AHT1X_DATA_NOP);
    Wire.write(AHT1X_DATA_NOP);

    if (Wire.endTransmission(true) != 0) return false;

    _delay_ms(AHT1X_CMD_DELAY);

    return true;
  }

  uint8_t readStatusByte() {
    Wire.requestFrom((uint8_t)ADDRESS, (uint8_t)1, (uint8_t)true);
    if (Wire.available() != 1) return AHT1X_ERROR;
    return Wire.read();
  }

  uint8_t calibrationBit(bool readData = true)   {
    if (readData == true) rawData[0] = readStatusByte();
    if (rawData[0] != AHT1X_ERROR) return bitRead(rawData[0], 3);
    return AHT1X_ERROR;
  }

  bool activateFactoryCalibCoefficient()   {
    Wire.beginTransmission(ADDRESS);
    Wire.write(AHT1X_INIT_CMD);
    Wire.write(AHT1X_INIT_CAL_ENABLE);
    Wire.write(AHT1X_DATA_NOP);
    if (Wire.endTransmission(true) != 0) return false;
    _delay_ms(AHT1X_CMD_DELAY);
    if (calibrationBit() == 0x01) return true;
    return false;
  }

  uint8_t busyBit(bool readData = true) {
    if (readData == true) rawData[0] = readStatusByte();
    if (rawData[0] != AHT1X_ERROR) return bitRead(rawData[0], 7);
    return AHT1X_ERROR;
  }

  uint8_t readRawData()  {
    Wire.beginTransmission(ADDRESS);
    Wire.write(AHT1X_START_MEASURMENT_CMD);
    Wire.write(AHT1X_DATA_MEASURMENT_CMD);
    Wire.write(AHT1X_DATA_NOP);
    if (Wire.endTransmission(true) != 0) return AHT1X_ERROR;
    if (calibrationBit() != 0x01)        return AHT1X_ERROR;
    if (busyBit(false) != 0x00)          _delay_ms(AHT1X_MEASURMENT_DELAY);

    Wire.requestFrom((uint8_t)ADDRESS, (uint8_t)6, (uint8_t)true);
    if (Wire.available() != 6) {
      rawData[0] = AHT1X_ERROR;
      return AHT1X_ERROR;
    }

    for (uint8_t i = 0; i < 6 ; i++) rawData[i] = Wire.read();

    return true;
  }

  int16_t readTemperatureX10(bool readData) {
    if (readData == true && (readRawData() == AHT1X_ERROR)) return AHT1X_ERROR;

    if (rawData[0] == AHT1X_ERROR) return AHT1X_ERROR;
    uint32_t t = ((uint32_t)(rawData[3] & 0x0F) << 16) | ((uint16_t)rawData[4] << 8) | rawData[5];
    return (int16_t)(t * 191UL / 100000) - 500;
  }


  uint8_t readHumidity(bool readData) {
    if (readData == true && (readRawData() == AHT1X_ERROR)) return AHT1X_ERROR;

    if (rawData[0] == AHT1X_ERROR) return AHT1X_ERROR;
    uint32_t h = (((uint32_t)rawData[1] << 16) | ((uint16_t)rawData[2] << 8) | (rawData[3])) >> 4;
    uint8_t humidity = h * 95UL/1000000;
    return (humidity > 100) ? 100 : humidity;
  }

public:

  Aht1x () : rawData{AHT1X_ERROR, 0, 0, 0, 0, 0} {}

  void init () {
    Wire.begin();
    //Wire.setClock(100000);
    _delay_ms(AHT1X_POWER_ON_DELAY);
    initNormalMode();
    _present = activateFactoryCalibCoefficient();
  }

  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      uint8_t readStatus = readRawData();
      if (readStatus != AHT1X_ERROR) {
        _temperature = readTemperatureX10(false);
        _humidity    = readHumidity(false);
      }
    }
  }

};

}

#endif
