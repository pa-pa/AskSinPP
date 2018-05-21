
# Please use V3 Branch for your devices. The master branch is used for active development and may be instable

# AskSin++

C++ implementation of the AskSin protocol

- easy configuration of the device channels by using templates
- direct eeprom access for the channel data
- AES signature support
- for ATMega328, [ATMega32](https://github.com/eaconner/ATmega32-Arduino), [STM32F1](http://dan.drown.org/stm32duino/package_STM32duino_index.json)

## Required additional Arduino libraries:
- [EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt)
- [Low-Power](https://github.com/rocketscream/Low-Power.git)


## Optional required Sensor Libraries
- [Sensor Base Library](https://github.com/adafruit/Adafruit_Sensor)
- [DHT11/DHT22/AM2302/RHT03](https://github.com/adafruit/DHT-sensor-library)
- [SHT10](https://github.com/spease/Sensirion.git)
- [TSL2561](https://github.com/adafruit/TSL2561-Arduino-Library)
- [BMP085/BMP180](https://github.com/adafruit/Adafruit-BMP085-Library)
- [BMP280/BME280](https://github.com/finitespace/BME280)
- [BH1750](https://github.com/claws/BH1750)


## Enable AES Support

To enable the AES signature support **USE_AES** needs to be
defined. In addition the default key and the default key index 
has to provided with **HM_DEF_KEY** and **HM_DEF_KEY_INDEX**. 

    #define USE_AES
    #define HM_DEF_KEY 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
    #deifne HM_DEF_KEY_INDEX 0
    
In FHEM you can get the key from the *hmKey*, 
*hmKey2*, *hmKey3* attributes of the VCCU. The index is the key
number multiplied by 2.




