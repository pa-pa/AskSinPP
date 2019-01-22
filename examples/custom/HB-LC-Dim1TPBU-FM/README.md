## HB-LC-Dim1TPBU-FM
Example implementation of an HM-LC-Dim1TPBU-FM dimmer with both buttons as remotes, to control other HM Devices.

features: 
* trailing-edge phase cut dimming (as on original hardware)![](https://github.com/SCUBA82/AskSinPP/blob/master/examples/custom/HB-LC-Dim1TPBU-FM/Images/K800_trailing-edge-cut.JPG)
* leading-edge phase cut dimming (not as clean as trailing-edge phase cut) ![](https://github.com/SCUBA82/AskSinPP/blob/master/examples/custom/HB-LC-Dim1TPBU-FM/Images/K800_leading-edge-cut.JPG)
* temperature monitoring
* shutdown at overload detection
* both buttons as remote

# ISP Pins from left (PRG1) to right:

GND | RESET | MOSI | MISO | VCC | SCK

![](https://github.com/SCUBA82/AskSinPP/blob/master/examples/custom/HB-LC-Dim1TPBU-FM/Images/K800_Programming-Pins.JPG)
 
 
Attention! the original hardware uses an Atmega644 (not Atmega644P/PA ) so you have to choose the right mcu for flashing. 