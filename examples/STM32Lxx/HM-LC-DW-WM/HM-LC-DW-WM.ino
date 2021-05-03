//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// HM-LC-DW-WM modified for the STM32L1xx CPU by trilu
//- -----------------------------------------------------------------------------------------------------------------------

#define HIDE_IGNORE_MSG

// as we have no defined HW in STM32duino yet, we are working on base of the default STM32L152CB board
// which has a different pin map. deviances are handled for the moment in a local header file. 
// This might change when an own HW for AskSin is defined
#include "AskSin32Duino.h"

// Derive ID and Serial from the device UUID
//#define USE_HW_SERIAL

#include <SPI.h>
#include <Onewire.h>

#include <AskSinPP.h>
#include <Dimmer.h>
#include <Sensors.h>
#include <sensors/Ds18b20.h>


// Pin definition of the specific device
#define CONFIG_BUTTON_PIN   PC15
#define LED_PIN             PC14
#define DIMMER1_PIN         PA2
#define DIMMER2_PIN         PA3
#define ONE_WIRE_PIN        PB9

// number of available peers per channel
#define PEERS_PER_CHANNEL 6


// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
  {0x7, 0x1C, 0x17},       // Device ID
  "HB85190714",           // Device Serial
  {0x01,0x08},            // Device Model: HM-LC-DW-WM dual white LED dimmer
//{0x00,0x67},            // Device Model: HM-LC-Dim1PWM-CV
  0x2C,                   // Firmware Version
  as::DeviceType::Dimmer, // Device Type
  {0x01,0x00}             // Info Bytes
};


/**
 * Configure the used hardware
 */
typedef LibSPI<CC1101_CS_PIN> RadioSPI;
typedef Radio<RadioSPI, CC1101_GDO0_PIN> RadioType;
typedef StatusLed<LED_PIN> LedType;
typedef AskSin<LedType, NoBattery, RadioType> HalType;
typedef DimmerChannel<HalType,PEERS_PER_CHANNEL> ChannelType;
typedef DimmerDevice<HalType, ChannelType, 6, 3> DimmerType;     // HM-LC-DW-WM dual white LED dimmer
//typedef DimmerDevice<HalType, ChannelType, 3, 3> DimmerType;   // HM-LC-Dim1PWM-CV

HalType hal;
DimmerType sdev(devinfo,0x20);
DualWhiteControl<HalType, DimmerType, PWM16<> > control(sdev);   // HM-LC-DW-WM dual white LED dimmer
//DimmerControl<HalType,DimmerType, PWM16<> > control(sdev);     // HM-LC-Dim1PWM-CV
ConfigToggleButton<DimmerType> cfgBtn(sdev);

class TempSens : public Alarm {
  Ds18b20  temp;
  OneWire  ow;
  bool     measure;
public:
  TempSens() : Alarm(0), ow(ONE_WIRE_PIN), measure(false) {}
  virtual ~TempSens() {}

  void init() {
    DPRINTLN(F("Init onewire..."));
    Ds18b20::init(ow, &temp, 1);
    if (temp.present() == true) {
      DPRINTLN(F("device found"));
      set(seconds2ticks(10));
      sysclock.add(*this);
    } else {
      DPRINTLN(F("no device"));
    }
  }

  virtual void trigger(AlarmClock& clock) {
    if (measure == false) {
      temp.convert();
      set(millis2ticks(800));
    } else {
      temp.read();
      DPRINT(F("Temp: ")); DDECLN(temp.temperature());
      control.setTemperature(temp.temperature());
      set(seconds2ticks(120));
    }
    measure = !measure;
    clock.add(*this);
  }
};
TempSens tempsensor;

void setup () {


  // start cc1101 module
  pinMode(CC1101_EN_PIN, OUTPUT);
  digitalWrite(CC1101_EN_PIN, LOW);

  delay(1000);
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  //storage().setByte(0, 0);


  bool first = control.init(hal, DIMMER1_PIN, DIMMER2_PIN);      // HM-LC-DW-WM dual white LED dimmer
//bool first = control.init(hal, DIMMER1_PIN);                   // HM-LC-Dim1PWM-CV
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);

  if (first == true) {
    sdev.channel(1).peer(cfgBtn.peer());
    sdev.channel(2).peer(cfgBtn.peer());
    sdev.channel(3).peer(cfgBtn.peer());
    sdev.channel(4).peer(cfgBtn.peer());
    sdev.channel(5).peer(cfgBtn.peer());
    sdev.channel(6).peer(cfgBtn.peer());
  }

  //LowPower.begin();
  tempsensor.init();
  sdev.initDone();
  //sdev.led().invert(true);
  
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    //hal.activity.savePower<Idle<true> >(hal);
  }
}
