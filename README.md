
# Please use V2 Branch for your devices. The master branch is used for active development and may be instable

# AskSin++

C++ implementation of the AskSin protocol

- easy configuration of the device channels by using templates
- direct eeprom access for the channel data
- AES signature support

Required Arduino libraries:
- TimerOne - https://github.com/PaulStoffregen/TimerOne
- EnableInterrupt - https://github.com/GreyGnome/EnableInterrupt
- Low-Power - https://github.com/rocketscream/Low-Power.git

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

