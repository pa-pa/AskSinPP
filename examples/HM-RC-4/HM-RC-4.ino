//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

/*
 * Setup defines to configure the library.
 */
// #define USE_AES
// #define HM_DEF_KEY 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
// #define HM_DEF_KEY_INDEX 0

#include <EnableInterrupt.h>
#include <SPI.h>  // after including SPI Library - we can use LibSPI class
#include <AskSinPP.h>
#include <TimerOne.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <Remote.h>

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#ifdef USE_OTA_BOOTLOADER
  #define OTA_MODEL_START  0x7ff0 // start address of 2 byte model id in bootloader
  #define OTA_SERIAL_START 0x7ff2 // start address of 10 byte serial number in bootloader
  #define OTA_HMID_START   0x7ffc // start address of 3 byte device id in bootloader
#else
  // device ID
  #define DEVICE_ID HMID(0x78,0x90,0x12)
  // serial number
  #define DEVICE_SERIAL "papa333333"
#endif

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
#define LED_PIN2 5
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8
// Arduino pins for the buttons
// A0,A1,A2,A3 == PIN 14,15,16,17 on Pro Mini
#define BTN1_PIN 14
#define BTN2_PIN 15
#define BTN3_PIN 16
#define BTN4_PIN 17


// number of available peers per channel
#define PEERS_PER_CHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

/**
 * Configure the used hardware
 */
// typedef AvrSPI<10,11,12,13> RadioSPI;
typedef LibSPI<10> RadioSPI;
typedef AskSin<DualStatusLed<5,4>,BatterySensor<22,19>,Radio<RadioSPI,2> > HalBase;
class Hal : public HalBase {
  // extra clock to count button press events
  AlarmClock btncounter;
public:
  void init () {
    HalBase::init();
    // get new battery value after 50 key press
    battery.init(50,btncounter);
  }

  void sendPeer () {
    --btncounter;
  }

  bool runready () {
    return HalBase::runready() || btncounter.runready();
  }
} hal;


typedef MultiChannelDevice<Hal,RemoteChannel<Hal,PEERS_PER_CHANNEL>,4> RemoteType;
RemoteType sdev(0x20);

ConfigButton<RemoteType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.dumpSize();

  if( storage.setup(sdev.checksum()) == true ) {
    sdev.firstinit();
  }

  remoteISR(sdev,1,BTN1_PIN);
  remoteISR(sdev,2,BTN2_PIN);
  remoteISR(sdev,3,BTN3_PIN);
  remoteISR(sdev,4,BTN4_PIN);

  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);

#ifdef USE_OTA_BOOTLOADER
  sdev.init(hal,OTA_HMID_START,OTA_SERIAL_START);
  sdev.setModel(OTA_MODEL_START);
#else
  sdev.init(hal,DEVICE_ID,DEVICE_SERIAL);
  sdev.setModel(0x00,0x08);
#endif
  sdev.setFirmwareVersion(0x11);
  sdev.setSubType(DeviceType::Remote);
  sdev.setInfo(0x04,0x00,0x00);

  hal.init();
}

void loop() {
  bool pinchanged = false;
  for( int i=1; i<=sdev.channels(); ++i ) {
    if( sdev.channel(i).checkpin() == true) {
      pinchanged = true;
    }
  }
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( pinchanged == false && worked == false && poll == false ) {
    hal.activity.savePower<Sleep<>>(hal);
  }
}
