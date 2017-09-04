//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

// define all device properties
#define DEVICE_ID HMID(0x78,0x90,0x12)
#define DEVICE_SERIAL "papa333333"
#define DEVICE_MODEL  0x00,0x08
#define DEVICE_FIRMWARE 0x11
#define DEVICE_TYPE DeviceType::Remote
#define DEVICE_INFO 0x04,0x00,0x00

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <SPI.h>  // after including SPI Library - we can use LibSPI class
#include <AskSinPP.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <Remote.h>


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
typedef LibSPI<10> SPIType;
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

typedef RemoteChannel<Hal,PEERS_PER_CHANNEL> ChannelType;
typedef MultiChannelDevice<Hal,ChannelType,4> RemoteType;

Hal hal;
RemoteType sdev(0x20);
ConfigButton<RemoteType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);

  remoteISR(sdev,1,BTN1_PIN);
  remoteISR(sdev,2,BTN2_PIN);
  remoteISR(sdev,3,BTN3_PIN);
  remoteISR(sdev,4,BTN4_PIN);

  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
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
