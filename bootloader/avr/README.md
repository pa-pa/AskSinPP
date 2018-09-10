
The bootloader was build from https://github.com/jabdoa2/Asksin_OTA_Bootloader

srecord and bin2eq3.php was extracted form https://github.com/jabdoa2/Asksin_OTA_Bootloader

flash-ota can be found here https://git.zerfleddert.de/cgi-bin/gitweb.cgi/hmcfgusb



Required Windows tools
  * cygwin with bash, sed, php-cli
  * avrdude
  
The first step is to create a bootloader with the specific device data
  * Device Model  - 2  Byte Hex (=DEVMODEL, see list of known devicemodels, this is used to connect to images and more info in device.xml files)
  * Device ID     - 3  Byte Hex (=HMID, like binary serial, unique) 
  * Device Serial - 10 Byte ASCII (=SERIAL, easy identified and used everywhere in HM webui)
  * Device Config - 16 Byte Hex (=CONFIG, optional - all 0x00 by default)

This infos correspondent to the DeviceInfo in devices.h. But not completly. Why? Take a look:
 * part belongs to hardware and is different even on a device with same firmware, this part is flashed
 inside the bootloader: DEVMODEL, HMID, SERIAL, CONFIG
 * part of the DeviceInfo belongs to firmware, e.g. `uint8_t Firmware;` so it is not part of the bootloader
 
The specific boot loader can created by using the makeota.html page. Load the "Bootloader-OTA-atmega328.hex" 
into the web browser and fill all fields. After pressing the "Create" button the bootloader can be downloaded 
to the loacl disk. The complete page runs inside your web browser. There is no internet access needed.

The bootloader can also created by the makeota.sh schell script. Call the script with the following options:

`makeota.sh DEVMODEL HMID SERIAL [CONFIG]`
   
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

<hr/>

CCU(1/2/3) / raspberrymatic-users (updating device firmware via webui):
 
- create a gzipped tar-file (i.e. (`fwupdate_1.1.tar.gz`) containing 3 files
  - `firmware.eq3` (created with `prepareforota.sh path_to_firmware.hex` (see steps above))
  - `changelog.txt` containing a short changelog 
  - `info` containing 4 lines (i.e. for [HB-UNI-Sen-CAP-MOIST](https://github.com/jp112sdl/HB-UNI-Sen-CAP-MOIST/blob/master/HB-UNI-Sen-CAP-MOIST/HB-UNI-Sen-CAP-MOIST.ino):
     - `TypeCode=62225` -> [Device Model](https://github.com/jp112sdl/HB-UNI-Sen-CAP-MOIST/blob/77cb0f7961f1ac95622271b9b154303f51c6c992/HB-UNI-Sen-CAP-MOIST/HB-UNI-Sen-CAP-MOIST.ino#L63) in decimal 
     - `Name=HB-UNI-Sen-CAP-MOIST` -> [Device Id](https://github.com/jp112sdl/JP-HB-Devices-addon/blob/a532a19f5006cbd98625bee86a20ebeef6dde12a/src/addon/firmware/rftypes/hb-uni-sen-cap-moist.xml#L4) in XML File
     - `CCUFirmwareVersionMin=2.27.0` -> minimum required CCU FW Version
     - `FirmwareVersion=1.1` -> [firmware version](https://github.com/jp112sdl/HB-UNI-Sen-CAP-MOIST/blob/77cb0f7961f1ac95622271b9b154303f51c6c992/HB-UNI-Sen-CAP-MOIST/HB-UNI-Sen-CAP-MOIST.ino#L64) provided by this update
 
 - upload created .tar.gz on device firmware page (like normal firmware updates for hm devices)
   - for further information on how to update device firmware, see https://www.eq-3.de/Downloads/eq3/pdf_FAQ/Manuelles_Update_Geraete-Firmware.pdf
 

