//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

// define all device properties
#define DEVICE_ID HMID(0x34,0x56,0x78)
#define DEVICE_SERIAL "papa111111"
#define DEVICE_MODEL  0x00,0x3d
#define DEVICE_FIRMWARE 0x10
#define DEVICE_TYPE DeviceType::THSensor
#define DEVICE_INFO 0x03,0x01,0x00

#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <TimerOne.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>


// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8


// number of available peers per channel
#define PEERS_PER_CHANNEL 6


// all library classes are placed in the namespace 'as'
using namespace as;

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef StatusLed<4> LedType;
typedef BatterySensor<22,19> BatteryType;
typedef AskSin<LedType,BatteryType,RadioType> BaseHal;
class Hal : public BaseHal {
public:
  void init () {
    BaseHal::init();
    // measure battery every 1h
    battery.init(seconds2ticks(60UL*60),sysclock);
  }
} hal;

class WeatherEventMsg : public Message {
public:
  void init(uint8_t msgcnt,int16_t temp,uint8_t humidity, bool batlow) {
    uint8_t t1 = (temp >> 8) & 0x7f;
    uint8_t t2 = temp & 0xff;
    if( batlow == true ) {
      t1 |= 0x80; // set bat low bit
    }
    Message::init(0xc,msgcnt,0x70,Message::BIDI,t1,t2);
    pload[0] = humidity;
  }
};

class WeatherChannel : public Channel<Hal,List1,EmptyList,List4,PEERS_PER_CHANNEL>, public Alarm {

  WeatherEventMsg msg;
  uint8_t         msgcnt;
  int16_t         temp;
  uint8_t         humidity;

public:
  WeatherChannel () : Channel(), Alarm(5), msgcnt(0), temp(0), humidity(50) {}
  virtual ~WeatherChannel () {}

  virtual void trigger (AlarmClock& clock) {
    // reactivate for next measure
    tick = delay();
    clock.add(*this);
    DPRINT("Measure...\n");
    measure();

    msg.init(msgcnt,temp,humidity,false);
    device().sendPeerEvent(msg,*this);
  }

  // here we do the measurement
  void measure () {
    static int16_t tdx = -7;
    static int8_t  hdx = 1;
    temp += tdx;
    humidity += hdx;
    if( temp >= 40*10 || temp <= -15*10 ) tdx = -tdx;
    if( humidity == 99 || humidity == 5) hdx = -hdx;
  }

  // here we calc when to send next value
  uint32_t delay () {
    // for testing we use delay of 5sec
    return seconds2ticks(5);
  }

  void setup(Device<Hal>* dev,uint8_t number,uint16_t addr) {
    Channel::setup(dev,number,addr);
    sysclock.add(*this);
  }

  uint8_t status () const {
    return 0;
  }

  uint8_t flags () const {
    return 0;
  }

};


typedef MultiChannelDevice<Hal,WeatherChannel,1> WeatherType;
WeatherType sdev(0x20);

ConfigButton<WeatherType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Sleep<>>(hal);
  }
}
