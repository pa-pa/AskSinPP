
The bootloader was build from https://github.com/jabdoa2/Asksin_OTA_Bootloader

srecord and bin2eq3.php was extracted form https://github.com/jabdoa2/Asksin_OTA_Bootloader

flash-ota can be found here https://git.zerfleddert.de/cgi-bin/gitweb.cgi/hmcfgusb



Required Windows tools
  * cygwin with bash, sed, php-cli
  * avrdude
  
The first step is to create a bootloader with the specific device data
  * Device Type   - 2 Byte Hex
  * Device ID     - 3 Byte Hex
  * Device Serial - 10 Byte ASCII
  * Device Config - 16 Byte Hex (optional - all 0x00 by default)
  
The specific boot loader can created by using the makeota.html page. Load the "Bootloader-OTA-atmega328.hex" 
into the web browser and fill all fields. After pressing the "Create" button the bootloader can be downloaded 
to the loacl disk. The complete page runs inside your web browser. There is no internet access needed.

The bootloader can also created by the makeota.sh schell script. Call the script with the following options:

`makeota.sh DEVID HMID SERIAL [CONFIG]`
   
and redirect the output into a file.

The bootloader is flashed with:

`flash.sh bootloader.hex`

This will set the fuses to internal 8MHz clock and disables Brown-out. After flashing the bootloader you need 
to transfer the firmware by air.

To build the OTA enabled software USE_OTA_BOOLOADER needs to be defined. This enables the access of the device 
specific data from the bootloader section. Before the firmware can be transfered it needs to converted.

`prepareforota.sh path_to_firmware.hex`

will convert your fimrware into the eq3 format. The result will be located in your current directory. The filename 
is automatically appended by the current datestamp. To transfer the firmware run

`flash-ota -f firmware.eq3 -s Serial` 


