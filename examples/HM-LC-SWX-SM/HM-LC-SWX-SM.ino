//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

// number of relays by defining the device
#define HM_LC_SW1_SM 0x00,0x02
#define HM_LC_SW2_SM 0x00,0x0a
#define HM_LC_SW4_SM 0x00,0x03

#define CFG_LOWACTIVE_BYTE 0x00
#define CFG_LOWACTIVE_ON   0x01
#define CFG_LOWACTIVE_OFF  0x00

// define all device properties
#define DEVICE_ID HMID(0x12,0x34,0x56)
#define DEVICE_SERIAL "papa000000"
#define DEVICE_MODEL  HM_LC_SW4_SM
#define DEVICE_FIRMWARE 0x16
#define DEVICE_TYPE DeviceType::Switch
#define DEVICE_INFO 0x04,0x01,0x00
#define DEVICE_CONFIG CFG_LOWACTIVE_OFF


#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <SwitchChannel.h>


// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8


// relay output pins compatible to the HM_Relay project
#define RELAY1_PIN 5
#define RELAY2_PIN 6
#define RELAY3_PIN 7
#define RELAY4_PIN 3

// number of available peers per channel
#define PEERS_PER_CHANNEL 8


// all library classes are placed in the namespace 'as'
using namespace as;

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> RadioSPI;
typedef AskSin<StatusLed<4>,NoBattery,Radio<RadioSPI,2> > Hal;

// setup the device with channel type and number of channels
typedef MultiChannelDevice<Hal,SwitchChannel<Hal,PEERS_PER_CHANNEL>,4> SwitchType;

Hal hal;
SwitchType sdev(0x20);
ConfigToggleButton<SwitchType> cfgBtn(sdev);

// map number of channel to pin
// this will be called by the SwitchChannel class
uint8_t SwitchPin (uint8_t number) {
  switch( number ) {
    case 2: return RELAY2_PIN;
    case 3: return RELAY3_PIN;
    case 4: return RELAY4_PIN;
  }
  return RELAY1_PIN;
}

// if A0 and A1 connected
// we use LOW for ON and HIGH for OFF
bool checkLowActive () {
  pinMode(14,OUTPUT); // A0
  pinMode(15,INPUT_PULLUP);  // A1
  digitalWrite(15,HIGH);
  digitalWrite(14,LOW);
  bool result = digitalRead(15) == LOW;
  digitalWrite(14,HIGH);
  return result;
}

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);

  bool low = (sdev.getConfigByte(CFG_LOWACTIVE_BYTE) == CFG_LOWACTIVE_ON) || checkLowActive();
  DPRINT("Invert ");low ? DPRINTLN("active") : DPRINTLN("disabled");
  for( uint8_t i=1; i<=sdev.channels(); ++i ) {
    sdev.channel(i).lowactive(low);
  }

  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);

  uint8_t model[2];
  sdev.getDeviceModel(model);
  if( model[1] == 0x02 ) {
    sdev.channels(1);
    DPRINTLN(F("HM-LC-SW1-SM"));
  }
  else if( model[1] == 0x0a ) {
    sdev.channels(2);
    DPRINTLN(F("HM-LC-SW2-SM"));
  }
  else {
    DPRINTLN(F("HM-LC-SW4-SM"));
  }
  // create internal peerings - CCU2 needs this
  HMID devid;
  sdev.getDeviceID(devid);
  for( uint8_t i=1; i<=sdev.channels(); ++i ) {
    Peer ipeer(devid,i);
    // create internal peer if not already done
    uint8_t idx = 0; // make compiler happy
    if( sdev.channel(i).peer(idx) != ipeer ) {
      sdev.channel(i).peer(ipeer);
    }
  }
  // delay next send by random time
  hal.waitTimeout((rand() % 3500)+1000);
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Idle<>>(hal);
  }
}
