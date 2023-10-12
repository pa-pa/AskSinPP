//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// HM-LC-DW-WM modified for the STM32L1xx CPU by trilu
//- -----------------------------------------------------------------------------------------------------------------------

#if __has_include(".private.h") 
#include ".private.h"
#else
  #define DEVICE_ID {0x12, 0x34, 0x56}
  #define SERIAL_ID "HB01234567"
#endif

//#define HIDE_IGNORE_MSG
//#define DIMMER_EXTRA_DEBUG
//#define NDEBUG
#undef NDEBUG

// -- Asksin32duino specific ---------------------
HardwareSerial Serial1(PA10, PA9);
#define DSERIAL Serial1

#define PIN_SPI_MOSI       PB_15
#define PIN_SPI_MISO       PB_14
#define PIN_SPI_SCK        PB_13
#define CC1101_GDO0_PIN    PA_8
#define CC1101_CS_PIN      PB_12
#define CC1101_EN_PIN      PA_15

// needed for cpu temp measurement function
#include "stm32yyxx_ll_adc.h"
// ------------------------------------------------


#include <SPI.h>
#include <AskSinPP.h>
#include <Dimmer.h>


// Pin definition of the specific device
#define CONFIG_BUTTON_PIN   PC15
#define LED1_PIN            PC14
#define LED2_PIN            PC13
#define DIMMER1_PIN         PB4
#define DIMMER2_PIN         PB5

//#define BTN_PIN_1           PB6
//#define BTN_PIN_2           PB7

// number of available peers per channel
#define PEERS_PER_CHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
  DEVICE_ID,              // Device ID
  SERIAL_ID,              // Device Serial
  
  {0x01,0x08},            // Device Model: HM-LC-DW-WM dual white LED dimmer
  //{0x00,0x67},          // Device Model: HM-LC-Dim1PWM-CV
  0x2C,                   // Firmware Version
  as::DeviceType::Dimmer, // Device Type
  {0x01,0x00}             // Info Bytes
};

// Configure the used hardware
typedef LibSPI<CC1101_CS_PIN> RadioSPI;
//typedef Radio<RadioSPI, CC1101_GDO0_PIN> RadioType;
typedef CalibratedRadio<RadioSPI, CC1101_GDO0_PIN, CC1101_EN_PIN> RadioType;
typedef DualStatusLed<LED1_PIN, LED2_PIN> LedType;
typedef AskSin<LedType, NoBattery, RadioType> HalType;
typedef DimmerChannel<HalType,PEERS_PER_CHANNEL> ChannelType;
typedef DimmerDevice<HalType, ChannelType, 6, 3> DimmerType;     // HM-LC-DW-WM dual white LED dimmer
//typedef DimmerDevice<HalType, ChannelType, 3, 3> DimmerType;   // HM-LC-Dim1PWM-CV

HalType hal;
DimmerType sdev(devinfo,0x20);
DualWhiteControl<HalType, DimmerType, PWM16<> > control(sdev);   // HM-LC-DW-WM dual white LED dimmer
//DimmerControl<HalType,DimmerType, PWM16<> > control(sdev);     // HM-LC-Dim1PWM-CV

ConfigToggleButton<DimmerType> cfgBtn(sdev);
//ConfigButton<DimmerType> cfgBtn(sdev);
//InternalButton<DimmerType, LOW, HIGH, INPUT_PULLDOWN> btn1(sdev, 1);
//InternalButton<DimmerType, LOW, HIGH, INPUT_PULLDOWN> btn2(sdev, 2);


// internal cpu reading function for STM32L1x

class TempSens : public Alarm {
  uint16_t vref;
  uint16_t temp;

  static uint16_t readVref() {
    return (__LL_ADC_CALC_VREFANALOG_VOLTAGE(analogRead(AVREF), LL_ADC_RESOLUTION_12B));
  }
  static uint16_t readTempSensor(int32_t VRef) {
    return (__LL_ADC_CALC_TEMPERATURE(VRef, analogRead(ATEMP), LL_ADC_RESOLUTION_12B));
  }

public:
  TempSens() : Alarm(0) {}
  virtual ~TempSens() {}

  void init() {
    DPRINT(F("internal temp sensor "));
    // read vref to see if it works
    vref = readVref();
    if (vref) {
      DPRINTLN(F("found"));
      set(seconds2ticks(10));
      sysclock.add(*this);
    }
    else {
      DPRINTLN(F("not available"));
    }
  }

  virtual void trigger(AlarmClock& clock) {
    vref = readVref();
    temp = readTempSensor(vref) * 10;
    DPRINT(F("tmp ")); DPRINTLN(temp);
    control.setTemperature(temp);
    set(seconds2ticks(120));
    clock.add(*this);
  }
};
TempSens tempsensor;


void setup () {

  //storage().setByte(0, 0);
  delay(1000);
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);

  bool first = control.init(hal, DIMMER1_PIN, DIMMER2_PIN);      // HM-LC-DW-WM dual white LED dimmer
  //bool first = control.init(hal, DIMMER1_PIN);                 // HM-LC-Dim1PWM-CV
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  //buttonISR(btn1, BTN_PIN_1);
  //buttonISR(btn2, BTN_PIN_2);

  if (first == true) {
    sdev.channel(1).peer(cfgBtn.peer());
    sdev.channel(2).peer(cfgBtn.peer());
    sdev.channel(3).peer(cfgBtn.peer());
    sdev.channel(4).peer(cfgBtn.peer());
    sdev.channel(5).peer(cfgBtn.peer());
    sdev.channel(6).peer(cfgBtn.peer());

    //sdev.channel(1).peer(btn1.peer());
    //sdev.channel(2).peer(btn2.peer());
    //sdev.channel(3).peer(btn1.peer());
    //sdev.channel(4).peer(btn2.peer());
    //sdev.channel(5).peer(btn1.peer());
    //sdev.channel(6).peer(btn2.peer());
  }

  tempsensor.init();
  sdev.initDone();
  DDEVINFO(sdev);
}


void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Idle<true> >(hal);
  }

}

