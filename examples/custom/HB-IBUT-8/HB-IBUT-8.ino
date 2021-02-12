//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-10-10 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// ci-test=yes board=328p aes=no
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <OneWire.h>

#include <Register.h>
#include <Device.h>
#include <MultiChannelDevice.h>

#include <IButton.h>

#define LED1_PIN 4
#define LED2_PIN 5
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

#define IBUTTON_READER_PIN 9  // B1

#define NUM_CHANNELS 8
// number of available peers per channel
#define PEERS_PER_CHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0xfa,0x2a,0xce},       // Device ID
    "papafa2ace",           // Device Serial
    {0xf2,0x06},            // Device Model
    0x02,                   // Firmware Version
    as::DeviceType::Sensor, // Device Type
    {0x00,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> RadioSPI;
typedef DualStatusLed<LED2_PIN,LED1_PIN> LedType;
typedef AskSin<LedType,BatterySensor,Radio<RadioSPI,2> > Hal;

DEFREGISTER(IButReg0,MASTERID_REGS,DREG_BUTTON_MODE)
class IButList0 : public RegList0<IButReg0> {
public:
  IButList0 (uint16_t addr) : RegList0<IButReg0>(addr) {}
};

typedef IButtonChannel<Hal,PEERS_PER_CHANNEL,IButList0> IButChannel;

class IButDev : public MultiChannelDevice<Hal,IButChannel,NUM_CHANNELS,IButList0> {
public:
  typedef MultiChannelDevice<Hal,IButChannel,NUM_CHANNELS,IButList0> DevType;
  IButDev (const DeviceInfo& i,uint16_t addr) : DevType(i,addr) {}
  virtual ~IButDev () {}
  // return ibutton channel from 0 - n-1
  IButChannel& ibuttonChannel (uint8_t num) { return channel(num+1); }
  // return number of ibutton channels
  uint8_t ibuttonCount () const { return channels(); }
};

Hal hal;
IButDev sdev(devinfo,0x20);
ConfigButton<IButDev> cfgBtn(sdev);
IButtonScanner<IButDev,IButChannel,IBUTTON_READER_PIN,LED2_PIN,LED1_PIN> scanner(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  sdev.initDone();
  sysclock.add(scanner);
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Idle<>>(hal);
  }
}
