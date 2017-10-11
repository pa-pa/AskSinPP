//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
#define USE_OTA_BOOTLOADER
#define NDEBUG

#include <AskSinPP.h>

#include <MultiChannelDevice.h>
#include <SwitchChannel.h>
#include <Remote.h>

// see https://github.com/eaconner/ATmega32-Arduino for Arduino Pin Mapping

#define RELAY1_PIN 0 // PB0
#define RELAY2_PIN 1 // PB1

#define BUTTON1_PIN 31 // PA0
#define BUTTON2_PIN 30 // PA1

// number of available peers per channel
// number of available peers per channel
#define PEERS_PER_SWCHANNEL  6
#define PEERS_PER_BTNCHANNEL 20

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x83,0x2A,0xE1},       // Device ID
    "HMSwC00001",           // Device Serial
    {0xf2,0x01},            // Device Model
    0x01,                   // Firmware Version
    as::DeviceType::Switch, // Device Type
    {0x01,0x00}             // Info Bytes
};

// Configure the used hardware
typedef AvrSPI<4,5,6,7> RadioSPI; // PB4-PB7
typedef StatusLed<12> LedType; // PD4
typedef AskSin<LedType,NoBattery,Radio<RadioSPI,11> > Hal; // PD3
Hal hal;

uint8_t SwitchPin (uint8_t number) {
  switch( number ) {
    case 4: return RELAY2_PIN;
  }
  return RELAY1_PIN;
}

typedef SwitchChannel<Hal,PEERS_PER_SWCHANNEL>  SwChannel;
typedef RemoteChannel<Hal,PEERS_PER_BTNCHANNEL> BtnChannel;

class MixDevice : public ChannelDevice<Hal,VirtBaseChannel<Hal>,4> {
public:
  VirtChannel<Hal,BtnChannel> c1,c2;
  VirtChannel<Hal,SwChannel>  c3,c4;
public:
  typedef ChannelDevice<Hal,VirtBaseChannel<Hal>,4> DeviceType;
  MixDevice (const DeviceInfo& info,uint16_t addr) : DeviceType(info,addr) {
    DeviceType::registerChannel(c1,1);
    DeviceType::registerChannel(c2,2);
    DeviceType::registerChannel(c3,3);
    DeviceType::registerChannel(c4,4);
  }
  virtual ~MixDevice () {}

  BtnChannel& btn1Channel () { return c1; }
  BtnChannel& btn2Channel () { return c2; }
  SwChannel&  sw1Channel  () { return c3; }
  SwChannel&  sw2Channel  () { return c4; }
};
MixDevice sdev(devinfo,0x20);
ConfigButton<MixDevice> cfgBtn(sdev);

void setup () {
  DINIT(19200,ASKSIN_PLUS_PLUS_IDENTIFIER);
  bool firstinit = sdev.init(hal);
  sdev.sw1Channel().lowactive(false);
  sdev.sw2Channel().lowactive(false);
  remoteChannelISR(sdev.btn1Channel(),BUTTON1_PIN);
  remoteChannelISR(sdev.btn2Channel(),BUTTON2_PIN);
  if( firstinit == true ) {
    // create internal peerings
    HMID devid;
    sdev.getDeviceID(devid);
    sdev.sw1Channel().peer(Peer(devid,1));
    sdev.sw2Channel().peer(Peer(devid,2));
    sdev.btn1Channel().peer(Peer(devid,3));
    sdev.btn2Channel().peer(Peer(devid,4));
    storage.store();
  }
  // delay next send by random time
  hal.waitTimeout((rand() % 3500)+1000);
}

void loop() {
  bool pinchanged = sdev.btn1Channel().checkpin();
  pinchanged |= sdev.btn2Channel().checkpin();
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
//  if( pinchanged == false && worked == false && poll == false ) {
//    hal.activity.savePower<Idle<> >(hal);
//  }
}
