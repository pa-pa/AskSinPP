//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EXTRAMILLIS 730 // 730 millisecond extra time to better hit the slot
#define EI_NOTEXTERNAL // Disable all external interruptes
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <Register.h>
#include <MultiChannelDevice.h>
#include <Weather.h>

#define LED_PIN 4
#define CONFIG_BUTTON_PIN 8

// === Choose one Temperture/Humidity sensor ===
#define SENSOR_BME280
//#define SENSOR_DHT22
//#define SENSOR_SHT10
//#define SENSOR_SHT21
//#define SENSOR_SHT31
//#define SENSOR_SI7021

// === Define the clock ===
// Remove comment if you have an external oscillator like a 32kHz crystal
// Necessary for exact timings if you want to pair with a HM-CC-RT-DN
//#define USE_RTC

// === Battery measurement ===
#define BAT_VOLT_LOW        21  // 2.1V low voltage threshold
#define BAT_VOLT_CRITICAL   19  // 1.9V critical voltage threshold, puts AVR into sleep-forever mode
// Internal measuring: AVR voltage
#define BAT_SENSOR BatterySensor
// External measuring: Potential devider on GPIO; required if a StepUp converter is used
// one can consider lower thresholds (low=20; cri=13) 
//#define BAT_SENSOR BatterySensorUni<17,7,3000> // <SensPIN, ActivationPIN, RefVcc>

// === Sensor offset settings ===
// OFFSET for Temperature -> measured Temp +/- Offset = Announced Temperature
#define OFFSETtemp 0 //e.g. -50 ≙ -5°C / 50 ≙ +5°C
// OFFSET for Humidity -> measured Humidity +/- Offset = Announced Humidity
#define OFFSEThumi 0 //e.g. -10 ≙ -10%RF / 10 ≙ +10%RF

// Number of available peers per channel
#define PEERS_PER_CHANNEL 6

// All library classes are placed in the namespace 'as'
using namespace as;

// Define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x34,0x56,0x79},       // Device ID
    "papa111111",           // Device Serial
    {0x00,0x3f},            // Device Model: HM-WDS40-TH-I
    0x10,                   // Firmware Version
    as::DeviceType::THSensor, // Device Type
    {0x01,0x00}             // Info Bytes
};


#ifdef SENSOR_BME280
#include <sensors/Bme280.h>
typedef Bme280 SensorType; // I2C
#endif
#ifdef SENSOR_DHT22
#include <sensors/Dht.h>
typedef Dht<4,DHT22> SensorType; // <DataPin, Type>
#endif
#ifdef SENSOR_SHT10
#include <sensors/Sht10.h>
typedef Sht10<A4, A5> SensorType; // <DataPin, ClockPin>
#endif
#ifdef SENSOR_SHT21
#include <sensors/Sht21.h>
typedef Sht21<> SensorType; // I2C
#endif
#ifdef SENSOR_SHT31
#include <sensors/Sht31.h>
typedef Sht31<> SensorType; // I2C
#endif
#ifdef SENSOR_SI7021
#include <sensors/Si7021.h>
typedef Si7021 SensorType; // I2C
#endif


// Configure the used hardware
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef StatusLed<LED_PIN> LedType;
#ifdef USE_RTC
typedef AskSinRTC<LedType,BAT_SENSOR,RadioType> Hal;
#else
typedef AskSin<LedType,BAT_SENSOR,RadioType> Hal;
#endif

// Define List0 registers
DEFREGISTER(WeatherRegsList0,MASTERID_REGS,DREG_BURSTRX)
typedef RegList0<WeatherRegsList0> WeatherList0;

// Sensors class is used by the WeatherChannel to measure the data.
// It has to implement temperature() and humidity().
class Sensors : public Alarm {
  SensorType    sensor;
public:
  Sensors () {}
  // init the used hardware
  void init () { sensor.init(); }
  // return how many milliseconds the measure should start in front of sending the message
  uint16_t before () const { return 4000; }
  // get the data
  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    DPRINTLN("Measure...  ");
    sensor.measure();
    DPRINT("T: ");DDEC(sensor.temperature()+OFFSETtemp);DPRINT("  H: ");DDECLN(sensor.humidity()+OFFSEThumi);
  }
  uint16_t temperature () { return sensor.temperature()+OFFSETtemp; }
  uint8_t  humidity () { return sensor.humidity()+OFFSEThumi; }
};

#ifdef USE_RTC
typedef WeatherChannel<Hal,RTC,Sensors,PEERS_PER_CHANNEL,EXTRAMILLIS,WeatherList0> ChannelType;
#else
typedef WeatherChannel<Hal,SysClock,Sensors,PEERS_PER_CHANNEL,EXTRAMILLIS,WeatherList0> ChannelType;
#endif

typedef MultiChannelDevice<Hal,ChannelType,1,WeatherList0> WeatherType;

Hal hal;
WeatherType sdev(devinfo,0x20);
ConfigButton<WeatherType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER); // Init serial console
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN); // Register btn interrupt
  hal.initBattery(60UL*60,BAT_VOLT_LOW,BAT_VOLT_CRITICAL); // Measure Battery every 1h 
  sdev.initDone();
  DDEVINFO(sdev); // Print DeviceInfo to serial console
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    if( hal.battery.critical() ) {
      // this call will never return
      hal.activity.sleepForever(hal);
    }    
    hal.sleep<>();
  }
}
