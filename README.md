
# AskSin++  [![Build Status](https://travis-ci.com/pa-pa/AskSinPP.svg?branch=master)](https://travis-ci.com/pa-pa/AskSinPP)

C++ implementation of the AskSin protocol

- easy configuration of the device channels by using templates
- direct eeprom access for the channel data
- AES signature support
- Supported MCU:
  - ATMega328
  - [ATMega32](https://github.com/MCUdude/MightyCore) (Standard Pinout)
  - [ATMega644](https://github.com/MCUdude/MightyCore) (Bobuino Pinout is highly suggested)
  - [ATMega1284](https://github.com/MCUdude/MightyCore) (No OTA Bootloader support, yet)
  - [STM32F1](http://dan.drown.org/stm32duino/package_STM32duino_index.json)

## Required additional Arduino libraries:
- [EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt)
  - For ATMega644 and ATMega1284 the PR #49 needs to be included
- [Low-Power](https://github.com/rocketscream/Low-Power.git)
  - For ATMega644 and ATMega1284 the PR #57 needs to be included

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
    #define HM_DEF_KEY_INDEX 0
    
In FHEM you can get the key from the *hmKey*, 
*hmKey2*, *hmKey3* attributes of the VCCU. The index is the key
number multiplied by 2.

## Extra Defines for Configuration

All defines has to set in the sketch before any header file is included.

- **USE_WOR** - enables WOR listening during idle (burst devices)
- **USE_CCA** - enable Clear Channel Assessment / Listen Before Talk
- **DEVICE_CHANNEL_COUNT** - set number of device channels, skip calculation
- **USE_HW_SERIAL** - generate device id and serial from cpu identifier (ATmega644, ATmega1284, STM32F1 only)
- **USE_OTA_BOOTLOADER** - read device id and serial from the OTA bootloader
- **USE_OTA_BOOTLOADER_FREQUENCY** - reuse the CC1101 frequence setting from the bootloader
- **HIDE_IGNORE_MSG** - do not show `ignore`-packet dumps of received messages _(that are not for our device)_


## Defnies to reduce code size

The following defines can be used to reduce the size of the code.
All defines has to set in the sketch before any header file is included.

- **SENSOR_ONLY** - save some byte by exclude code for actor devices
- **NORTC** - removes code for RTC support
- **NOCRC** - removes CRC calculation during startup - The EEPROM will no longer initialized when channel configuration is changed. A extra RESET is needed to initialize it again.
- **SIMPLE_CC1101_INIT** - simple CC1101 initialization - No error reporting in case of errors.
- **NDEBUG** - no serial (debug) output; no hardware serial initialized/used at all

## Alternative Device RESET Method

Normally, a device can be reset to its default settings by pressing and holding down the config button for at least 6 seconds.
Alternatively, another reset method can be used with `ResetOnBoot.h` without pressing the config button (i.e. for devices in hard to reach places):
- to reset, reboot the device twice within 4 seconds after startup
  - explained in detail:
    - startup: the device led blinks with 5 Hz the first 4 seconds
    - if the device is restarted within this time, the device led blinks with 10 Hz for 4 seconds
    - only if the device is restarted again within this time, the device will do a RESET , otherwise it will proceed normal operation
- implemented in [`HM-LC-SW1-BA-PCB.ino`](https://github.com/pa-pa/AskSinPP/blob/master/examples/HM-LC-SW1-BA-PCB/HM-LC-SW1-BA-PCB.ino) to demonstrate the usage
- at least, an automated pairing can be triggered by uncommenting [line #113](https://github.com/pa-pa/AskSinPP/blob/8e235f54c6a31c9485be6e60632d58274ff199ba/examples/HM-LC-SW1-BA-PCB/HM-LC-SW1-BA-PCB.ino#L113)
