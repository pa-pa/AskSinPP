//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <SwitchChannel.h>
#include <Remote.h>

#include <avr/boot.h>

#define LED_PIN 4
#define CONFIG_BUTTON_PIN 8

#define RELAY1_PIN 5
#define RELAY2_PIN 6

#define BUTTON1_PIN 14
#define BUTTON2_PIN 15

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
    0x16,                   // Firmware Version
    as::DeviceType::Switch, // Device Type
    {0x01,0x00}             // Info Bytes
};

// Configure the used hardware
typedef AvrSPI<10,11,12,13> RadioSPI;
typedef AskSin<StatusLed<LED_PIN>,NoBattery,Radio<RadioSPI,2> > Hal;
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
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  bool firstinit = sdev.init(hal);
  sdev.sw1Channel().lowactive(false);
  sdev.sw2Channel().lowactive(false);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
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
  }
  // delay next send by random time
  hal.waitTimeout((rand() % 3500)+1000);

  uint8_t fuselow = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
  uint8_t fusehigh = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
  DPRINT("Low  ");DHEXLN(fuselow);
  DPRINT("High ");DHEXLN(fusehigh);
}

void loop() {
  bool pinchanged = sdev.btn1Channel().checkpin();
  pinchanged |= sdev.btn2Channel().checkpin();
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( pinchanged == false && worked == false && poll == false ) {
    hal.activity.savePower<Idle<> >(hal);
  }
}
