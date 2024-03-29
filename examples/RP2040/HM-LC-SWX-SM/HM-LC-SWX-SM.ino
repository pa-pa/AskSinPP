//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// ci-test=yes board=rp2040 aes=no
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

#define USE_HW_SERIAL
#define DEVICE_CONFIG CFG_LOWACTIVE_OFF

// #define HM_SENSOR_RELAY
#include <SPI.h>
#include <AskSinPP.h>
#include <Switch.h>

#define LED_PIN           D25
#define CONFIG_BUTTON_PIN D6
#define CC1101_GDO0_PIN   D20
#define CC1101_CS_PIN     SS

#ifdef HM_SENSOR_RELAY
  // relay pins for the HMSensor Relay Board
  #define RELAY1_PIN 21
  #define RELAY2_PIN 22
  #define RELAY3_PIN 25
  #define RELAY4_PIN 26
  #define BUTTON1_PIN 14
  #define BUTTON2_PIN 15
  #define BUTTON3_PIN 32
  #define BUTTON4_PIN 33
#else
  // relay output pins compatible to the HM_Relay project
  #define RELAY1_PIN D2
  #define RELAY2_PIN D3
  #define RELAY3_PIN D4
  #define RELAY4_PIN D5
#endif

// number of available peers per channel
#define PEERS_PER_CHANNEL 8


// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x43,0x43,0x65},       // Device ID
    "papa000000",           // Device Serial
    {HM_LC_SW4_SM},         // Device Model
    0x16,                   // Firmware Version
    as::DeviceType::Switch, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef LibSPI<CC1101_CS_PIN> RadioSPI;
typedef AskSin<StatusLed<LED_PIN>,NoBattery,Radio<RadioSPI,CC1101_GDO0_PIN> > Hal;

// setup the device with channel type and number of channels
typedef MultiChannelDevice<Hal,SwitchChannel<Hal,PEERS_PER_CHANNEL,List0>,4> SwitchType;

Hal hal;
SwitchType sdev(devinfo,0x20);
#ifdef HM_SENSOR_RELAY
ConfigButton<SwitchType> cfgBtn(sdev);
InternalButton<SwitchType> btn1(sdev,1);
InternalButton<SwitchType> btn2(sdev,2);
InternalButton<SwitchType> btn3(sdev,3);
InternalButton<SwitchType> btn4(sdev,4);
#else
ConfigToggleButton<SwitchType> cfgBtn(sdev);
#endif

// if 17 and 34 connected
// we use LOW for ON and HIGH for OFF
bool checkLowActive () {
  pinMode(17,OUTPUT);
  pinMode(17,INPUT_PULLUP);
  digitalWrite(34,HIGH);
  digitalWrite(17,LOW);
  bool result = digitalRead(34) == LOW;
  digitalWrite(17,HIGH);
  return result;
}

void initPeerings (bool first) {
  // create internal peerings - CCU2 needs this
  if( first == true ) {
    HMID devid;
    sdev.getDeviceID(devid);
    for( uint8_t i=1; i<=sdev.channels(); ++i ) {
      Peer ipeer(devid,i);
      sdev.channel(i).peer(ipeer);
    }
  }
}

void initModelType () {
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
}


void setup () {
  delay(2500);
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  bool first = sdev.init(hal);
#ifdef HM_SENSOR_RELAY
  bool low = false;
#else
  bool low = (sdev.getConfigByte(CFG_LOWACTIVE_BYTE) == CFG_LOWACTIVE_ON) || checkLowActive();
#endif
  DPRINT("Invert ");low ? DPRINTLN("active") : DPRINTLN("disabled");
  sdev.channel(1).init(RELAY1_PIN,low);
  sdev.channel(2).init(RELAY2_PIN,low);
  sdev.channel(3).init(RELAY3_PIN,low);
  sdev.channel(4).init(RELAY4_PIN,low);

  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
#ifdef HM_SENSOR_RELAY
  buttonISR(btn1,BUTTON1_PIN);
  buttonISR(btn2,BUTTON2_PIN);
  buttonISR(btn3,BUTTON3_PIN);
  buttonISR(btn4,BUTTON4_PIN);
#endif
  initModelType();
  initPeerings(first);
  sdev.initDone();
  DDEVINFO(sdev);
}

void loop() {
  hal.runready();
  sdev.pollRadio();
}
