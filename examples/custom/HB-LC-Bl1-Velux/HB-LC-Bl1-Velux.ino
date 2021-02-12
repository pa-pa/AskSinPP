//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2020-05-02 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// ci-test=yes board=328p aes=no
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define USE_WOR // set this if the KLI battery is used for power
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Blind.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

#ifdef USE_WOR
  // pins used on 328Stamp
  #define UP_PIN 5
  #define STOP_PIN 6
  #define DOWN_PIN 7
#else
  // pins used on HMSensor
  #define UP_PIN 19
  #define STOP_PIN 3
  #define DOWN_PIN 6
#endif

// number of available peers per channel
#define PEERS_PER_CHANNEL 12

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x59,0x32,0xee},              // Device ID
    "papa5932ee",                  // Device Serial
#ifdef USE_WOR
    {0xf2,0x0a},                   // HB-LC-Bl1-Velux
#else
    {0x00,0x05},                   // HM-LC-Bl1-FM
#endif
    0x24,                          // Firmware Version
    as::DeviceType::BlindActuator, // Device Type
    {0x01,0x00}                    // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> RadioSPI;
#ifdef USE_WOR
typedef IrqInternalBatt BattSensType;
#else
typedef NoBattery BattSensType;
#endif
typedef AskSin<StatusLed<LED_PIN>,BattSensType,Radio<RadioSPI,2> > Hal;

DEFREGISTER(BlindReg0,MASTERID_REGS,DREG_INTKEY,DREG_CONFBUTTONTIME,DREG_LOCALRESETDISABLE)

class BlindList0 : public RegList0<BlindReg0> {
public:
  BlindList0 (uint16_t addr) : RegList0<BlindReg0>(addr) {}
  void defaults () {
    clear();
    // intKeyVisible(false);
    confButtonTime(0xff);
    // localResetDisable(false);
  }
};

// we use this interface to press the buttons of the velux remote
// the methods will be implemented by the button class later
class PushPin : public Alarm {
public:
  PushPin () { async(true); }
  virtual ~PushPin () {}
  virtual void press () {};
  virtual void unpress () {}
  virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
    unpress();
  }
};


class BlChannel : public ActorChannel<Hal,BlindList1,BlindList3,PEERS_PER_CHANNEL,BlindList0,BlindStateMachine> {
  bool     m_Drive;
  PushPin* m_PinUp;
  PushPin* m_PinDown;
  PushPin* m_PinStop;

public:
  typedef ActorChannel<Hal,BlindList1,BlindList3,PEERS_PER_CHANNEL,BlindList0,BlindStateMachine> BaseChannel;

  BlChannel () : m_Drive(false), m_PinUp(0), m_PinDown(0), m_PinStop(0) {}
  virtual ~BlChannel () {}

  void init (PushPin& up, PushPin& down, PushPin& stop) {
    BaseChannel::init();
    m_PinUp   = &up;
    m_PinDown = &down;
    m_PinStop = &stop;
  }

  virtual void switchState(uint8_t oldstate,uint8_t newstate, uint32_t stateDelay) {
    BaseChannel::switchState(oldstate, newstate, stateDelay);
    if( newstate == AS_CM_JT_RAMPON && stateDelay > 0 ) {
      motorUp();
    }
    else if( newstate == AS_CM_JT_RAMPOFF && stateDelay > 0 ) {
      motorDown();
    }
    else if( m_Drive==true ){
      motorStop();
      DPRINT("Bat: ");DDECLN(device().battery().current());
    }
  }

  void motorUp () {
    m_PinUp->press();
    m_Drive=true;
  }

  void motorDown () {
    m_PinDown->press();
    m_Drive=true;
  }

  void motorStop () {
    // we need no stop for end positions
    if( status() != 0 && status() != 200 ) {
      m_PinStop->press();
    }
    m_Drive=false;
  }

  void init () {
    motorStop();
    BaseChannel::init();
  }
};

// setup the device with channel type and number of channels
typedef MultiChannelDevice<Hal,BlChannel,1,BlindList0> BlindType;

Hal hal;
BlindType sdev(devinfo,0x20);
ConfigButton<BlindType> cfgBtn(sdev);

// this class handles a single button of the velux remote
class VeluxBtn : public StateButton<>, public PushPin {
public:
  enum Mode {UP, DOWN, STOP};

private:
  Mode m_Mode;
  volatile bool m_IgnorePress;

public:
  VeluxBtn (Mode m) : m_Mode(m), m_IgnorePress(false) {}
  virtual ~VeluxBtn () {}
  virtual void state(uint8_t s) {
    uint8_t last = StateButton<>::state();
    StateButton<>::state(s);
    if( s == StateButton<>::released || (s == StateButton<>::longpressed && last != StateButton<>::longpressed) ) {
      DPRINT("SETIGNORE: ");DDECLN(getPin());
      m_IgnorePress = true;
      switch( m_Mode ) {
        case UP: sdev.channel(1).set(200,0,0); break;
        case DOWN: sdev.channel(1).set(0,0,0); break;
        case STOP: sdev.channel(1).stop(); break;
      }
    }
    else if( s == StateButton<>::longreleased ) {
      sdev.channel(1).stop();
    }
  }
  void irq () {
    // irq is only handled if the button is not pressed by software / PushPin
    if( PushPin::active() == false ) {
      StateButton<>::irq();
    }
  }
  virtual void unpress () {
    DPRINT("Unpress ");DDECLN(getPin());
    // back to high
    digitalWrite(getPin(),HIGH);
    // setup for input again
    StateButton<>::init(getPin());
  }

  virtual void press () {
    if( getPin() != 0 && m_IgnorePress == false) {
      DPRINT("Press ");DDECLN(getPin());
      // start timer
      PushPin::set(millis2ticks(250));
      sysclock.add((PushPin&)*this);
      // set mode to output
      pinMode(getPin(),OUTPUT);
      // set pin to low
      digitalWrite(getPin(),LOW);
    }
    else {
      DPRINT("Ignore ");DDECLN(getPin());
      m_IgnorePress = false;
    }
  }
};


VeluxBtn upbtn(VeluxBtn::UP);
VeluxBtn downbtn(VeluxBtn::DOWN);
VeluxBtn stopbtn(VeluxBtn::STOP);

void initPeerings (bool first) {
  // create internal peerings - CCU2 needs this
  if( first == true ) {
  }
}

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  //storage().setByte(0,0);
  bool first = sdev.init(hal);
  sdev.channel(1).init(upbtn,downbtn,stopbtn);
#ifdef USE_WOR
  hal.led.invert(true);
  hal.battery.init(seconds2ticks(60UL*60),sysclock);
  hal.battery.low(22);
  hal.battery.critical(19);
#endif
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  buttonISR(upbtn,UP_PIN);
  buttonISR(stopbtn,STOP_PIN);
  buttonISR(downbtn,DOWN_PIN);

  initPeerings(first);

  sdev.initDone();
  // wait for valid battery value
  while( hal.battery.current() == 0 ) ;
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    // deep discharge protection
    // if we drop below critical battery level - switch off all and sleep forever
    if( hal.battery.critical() ) {
      // this call will never return
      hal.sleepForever();
    }
#ifdef USE_WOR
    hal.activity.savePower<Sleep<> >(hal);
#else
    hal.activity.savePower<Idle<> >(hal);
#endif
  }
}
