//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2019-01-05 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// ci-test=yes board=328p aes=no
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <Remote.h>

// Arduino pin for the config button
// B0 == PIN 8
#define CONFIG_BUTTON_PIN 8
// Arduino pins for the buttons
#define BTN1_PIN 14  // A0
#define CLK_PIN  15  // A1
#define DATA_PIN 16  // A2


// number of available peers per channel
#define PEERS_PER_CHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x00,0x1e,0x00},       // Device ID
    "HMRC001E00",           // Device Serial
    {0x00,0x1d},            // Device Model
    0x11,                   // Firmware Version
    as::DeviceType::Remote, // Device Type
    {0x00,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef DualStatusLed<5,4> LedType;
typedef AskSin<LedType,BatterySensor,RadioType> HalType;
class Hal : public HalType {
  // extra clock to count button press events
  AlarmClock btncounter;
public:
  void init (const HMID& id) {
    HalType::init(id);
    // get new battery value after 50 key press
    battery.init(50,btncounter);
    battery.low(22);
    battery.critical(19);
  }

  void sendPeer () {
    --btncounter;
  }

  bool runready () {
    return HalType::runready() || btncounter.runready();
  }
};

typedef RemoteChannel<Hal,PEERS_PER_CHANNEL,List0> ChannelType;
typedef MultiChannelDevice<Hal,ChannelType,3> RemoteType;

Hal hal;
RemoteType sdev(devinfo,0x20);
ConfigButton<RemoteType> cfgBtn(sdev);
RemoteEncoder<RemoteType,2,3> enc(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  remoteISR(sdev,1,BTN1_PIN);
  encoderISR(enc,CLK_PIN,DATA_PIN);

  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  enc.process();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Sleep<>>(hal);
  }
}
