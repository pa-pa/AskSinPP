//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-03-20 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

//#define STORAGEDRIVER at24c32
#define STORAGEDRIVER at24cX<0x50,128,32>

#include <SPI.h>    // when we include SPI.h - we can use LibSPI class
#include <Wire.h>
#include <EEPROM.h> // the EEPROM library contains Flash Access Methods
#include <AskSinPP.h>
#include <OneWireSTM.h>
#include <sensors/Bme280.h>

#include <Register.h>
#include <Device.h>
#include <MultiChannelDevice.h>
#include <Switch.h>
#include <Remote.h>

#include <Sensors.h>
#include <sensors/Bh1750.h>

// use builtin led
#define LED_PIN LED_BUILTIN
// Arduino pin for the config button
// use button on maple mini board
#define CONFIG_BUTTON_PIN PB8

#define BELL_BUTTON_PIN PB4
#define BELL_LIGHT_PIN PA8

#define DOOR_PIN PC13
#define BUTTON_GREEN PC14
#define BUTTON_RED PC15

#define IBUTTON_READER_PIN PA0

#define IBUTTON_CHANNELS 6

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0xaa,0x11,0x33},       // Device ID
    "papaaa1133",           // Device Serial
    {0xf2,0x03},            // Device Model
    0x01,                   // Firmware Version
	  as::DeviceType::Remote, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef LibSPI<PA4> RadioSPI;
typedef AskSin<StatusLed<LED_BUILTIN>,NoBattery,Radio<RadioSPI,PB0> > Hal;

DEFREGISTER(DoorReg0,MASTERID_REGS,DREG_BACKONTIME)
class DoorList0 : public RegList0<DoorReg0> {
public:
  DoorList0 (uint16_t addr) : RegList0<DoorReg0>(addr) {}
  void defaults () {
    clear();
    backOnTime(3);
  }
};

class BellChannel : public RemoteChannel<Hal,PEERS_PER_CHANNEL,DoorList0> {
  class LightOff : public Alarm {
  public:
    LightOff () : Alarm(0) {}
    virtual ~LightOff () {}
    virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
      pwmWrite(BELL_LIGHT_PIN, 2048);
    }
  } loff;
public:
  BellChannel () {}
  virtual ~BellChannel () {}

  virtual void state(uint8_t s) {
    RemoteChannel<Hal,PEERS_PER_CHANNEL,DoorList0>::state(s);
    if( s == Button::pressed || s == Button::longpressed ) {
      pwmWrite(BELL_LIGHT_PIN, 65535);
      sysclock.cancel(loff);
      loff.set(seconds2ticks(device().getList0().backOnTime()));
      sysclock.add(loff);
    }
  }
};

class ValuesMsg : public Message {
public:
  void init(uint8_t msgcnt,uint8_t ch) {
    Message::init(0x0b,msgcnt,0x53,BIDI|WKMEUP,ch,0);
  }
  template <typename T>
  void add (T value) {
    uint8_t* values = buffer() + len + sizeof(T) - 1;
    uint8_t num = sizeof(T);
    while( num > 0 ) {
      *values = value & 0xff;
      value >>= 8;
      --values;
      --num;
    }
    // update length of message
    len += sizeof(T);
    // store number of values inside this message
    subcommand(subcommand()+1);
  }
};

DEFREGISTER(ValuesReg1,CREG_AES_ACTIVE)
class ValuesList1 : public RegList1<ValuesReg1> {
public:
  ValuesList1 (uint16_t addr) : RegList1<ValuesReg1>(addr) {}
  void defaults () {
    clear();
  }
};

class ValuesChannel : public Channel<Hal,ValuesList1,EmptyList,EmptyList,0,DoorList0>, Alarm {
  Bme280    bme280;
  Bh1750<>  bh1750;
public:
  typedef Channel<Hal,ValuesList1,EmptyList,EmptyList,0,DoorList0> BaseChannel;

  ValuesChannel () : BaseChannel(), Alarm(0) {}
  virtual ~ValuesChannel () {}

  virtual void trigger (AlarmClock& clock) {
    bme280.measure();
    bh1750.measure();

    DPRINT("T: ");DDEC(bme280.temperature());
    DPRINT("   P: ");DDEC(bme280.pressure());
    DPRINT("   H: ");DDECLN(bme280.humidity());
    DPRINT("B: ");DDECLN(bh1750.brightness());

    ValuesMsg& msg = (ValuesMsg&)device().message();
    msg.init(device().nextcount(),number());
    msg.add(bme280.temperature());
    msg.add(bme280.humidity());
    msg.add(bme280.pressure());
    msg.add(bh1750.brightness());
    device().send(msg, device().getMasterID());

    set(seconds2ticks(30));
    clock.add(*this);
  }

  void setup(Device<Hal,DoorList0>* dev,uint8_t number,uint16_t addr) {
    BaseChannel::setup(dev, number, addr);
    bme280.init();
    bh1750.init();
    set(seconds2ticks(5));
    sysclock.add(*this);
  }

  uint8_t status () const { return 0; }
  uint8_t flags () const { return 0; }

};

DEFREGISTER(IButtonReg1,CREG_AES_ACTIVE,0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7)
class IButtonList1 : public RegList1<IButtonReg1> {
public:
  IButtonList1 (uint16_t addr) : RegList1<IButtonReg1>(addr) {}
  void defaults () {
    clear();
  }
};

class IButtonChannel : public Channel<Hal,IButtonList1,EmptyList,DefList4,PEERS_PER_CHANNEL,DoorList0>, Alarm {

  enum { none=0, released, longpressed, longreleased };

  uint8_t state, matches, repeatcnt;

public:
  typedef Channel<Hal,IButtonList1,EmptyList,DefList4,PEERS_PER_CHANNEL,DoorList0> BaseChannel;

  IButtonChannel () : BaseChannel(), Alarm(0), state(0), matches(0),repeatcnt(0) {}
  virtual ~IButtonChannel () {}

  virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
    state = 0;
    changed(true);
  }

  uint8_t status () const {
    return state;
  }

  uint8_t flags () const {
    return 0;
  }

  bool match (uint8_t* addr) {
    matches <<= 1;
    uint8_t s = none;
    if( free() == false && isID(addr) == true ) {
      matches |= 0b00000001;
      // 3 or 6 matches are longpress and longlongpress
      if( (matches & 0b00111111) == 0b00000111 || (matches & 0b00111111) == 0b00111111 ) {
        s = longpressed;
        DPRINTLN("longpressed");
        // clear longlong
        matches &= 0b11000111;
      }
    }
    // check for long release
    if( (matches & 0b00001111) == 0b00001110 ) {
      s = longreleased;
      DPRINTLN("longreleased");
    }
    // check for release
    else if( (matches & 0b00000011) == 0b00000010 ) {
      s = released;
      DPRINTLN("released");
    }
    if( s != none ) {
      RemoteEventMsg& msg = (RemoteEventMsg&)this->device().message();
      msg.init(this->device().nextcount(),this->number(),repeatcnt,(s==longreleased || s==longpressed),this->device().battery().low());
      if( s == released || s == longreleased) {
        // send the message to every peer
        this->device().sendPeerEvent(msg,*this);
        repeatcnt++;
      }
      else if (s == longpressed) {
        // broadcast the message
        this->device().broadcastPeerEvent(msg,*this);
      }
    }
    return (matches & 0b00000001) == 0b00000001;
  }

  bool isID (uint8_t* buf) {
    IButtonList1 l = getList1();
    for( uint8_t n=0; n<8; ++n ) {
      if( l.readRegister(0xe0+n) != buf[n] ) {
        return false;
      }
    }
    return true;
  }

  void storeID (uint8_t* buf) {
    if( learn() == true ) {
      for( uint8_t n=0; n<8; ++n ) {
        getList1().writeRegister(0xe0+n,buf[n]);
      }
      state = 0;
      changed(true);
      sysclock.cancel(*this);
    }
  }

  bool free () {
    return getList1().readRegister(0xe0) == 0;
  }

  bool learn () const {
    return state == 200;
  }

  bool process (const ActionSetMsg& msg) {
    state = msg.value();
    changed(true);
    if( state != 0 ) {
      set(seconds2ticks(60));
      sysclock.add(*this);
    }
    return true;
  }

  bool process (__attribute__((unused)) const RemoteEventMsg& msg) { return false; }
  bool process (__attribute__((unused)) const SensorEventMsg& msg) { return false; }

};

typedef SwitchChannel<Hal,PEERS_PER_CHANNEL,DoorList0> OpenerChannel;

class DoorBellDev :  public ChannelDevice<Hal,VirtBaseChannel<Hal,DoorList0>,IBUTTON_CHANNELS+3,DoorList0> {
public:
  VirtChannel<Hal,BellChannel,DoorList0>  bell;
  VirtChannel<Hal,IButtonChannel,DoorList0>  ibut[IBUTTON_CHANNELS];
  VirtChannel<Hal,OpenerChannel,DoorList0>  opener;
  VirtChannel<Hal,ValuesChannel,DoorList0>  values;
public:
  typedef ChannelDevice<Hal,VirtBaseChannel<Hal,DoorList0>,IBUTTON_CHANNELS+3,DoorList0> DeviceType;
  DoorBellDev(const DeviceInfo& info,uint16_t addr) : DeviceType(info,addr) {
    int ch=1;
    DeviceType::registerChannel(bell,ch++);
    for( uint8_t i=0; i<IBUTTON_CHANNELS; ++i ) {
      DeviceType::registerChannel(ibut[i],ch++);
    }
    DeviceType::registerChannel(opener,ch++);
    DeviceType::registerChannel(values,ch++);
  }
  virtual ~DoorBellDev () {}

  BellChannel& bellChannel () { return bell; }
  IButtonChannel& ibutChannel (uint8_t num) { return ibut[num]; }
  OpenerChannel& openerChannel () { return opener; }
  ValuesChannel& valuesChannel () { return values; }
};

class IButtonScanner : public Alarm {
  OneWire   ow;
  DoorBellDev& dev;
  DualStatusLed<BUTTON_GREEN,BUTTON_RED> led;
  uint8_t cnt;
public:
  IButtonScanner (DoorBellDev& d) : Alarm(millis2ticks(500)), ow(IBUTTON_READER_PIN), dev(d), cnt(0) {
    led.init();
  }
  virtual ~IButtonScanner () {}

  IButtonChannel* learning () {
    for( uint8_t i=0; i<IBUTTON_CHANNELS; ++i ) {
      IButtonChannel& bc = dev.ibutChannel(i);
      if( bc.learn() == true ) {
        return &bc;
      }
    }
    return 0;
  }

  IButtonChannel* matches (uint8_t* addr) {
    for( uint8_t i=0; i<IBUTTON_CHANNELS; ++i ) {
      IButtonChannel& bc = dev.ibutChannel(i);
      if( bc.match(addr) == true ) {
        return &bc;
      }
    }
    return 0;
  }

  bool scan (uint8_t* addr) {
    ow.reset_search();
    if ( ow.search(addr) == false || OneWire::crc8(addr, 7) != addr[7] ) {
      memset(addr,0,8);
      return false;
    }
    return true;
  }

  void trigger (AlarmClock& clock) {
    // reactivate
    set(millis2ticks(250));
    clock.add(*this);
    ++cnt;
    // check if we have a learning channel
    IButtonChannel* lc = learning();
    if( lc != 0 ) {
      uint8_t cycle = cnt & 0x01;
      led.ledOn(cycle == 0 ? tick : 0, cycle == 0 ? 0 : tick);
    }
    // scan the bus now
    uint8_t addr[8];
    bool found = scan(addr);
    // search matching channel
    IButtonChannel* match = matches(addr);
    if( found == true ) {
      if( lc != 0 ) {
        clock.cancel(*this);
        set(seconds2ticks(5));
        led.ledOff();
        led.ledOn(tick);
        clock.add(*this);
        lc->storeID(addr);
      }
      else {
        if( match != 0 ) {
          led.ledOn(millis2ticks(500),0);
        }
        else {
          led.ledOn(0,millis2ticks(500));
        }
      }
    }
  }
};

Hal hal;
DoorBellDev sdev(devinfo,0x20);
ConfigButton<DoorBellDev,LOW,HIGH,INPUT_PULLDOWN> cfgBtn(sdev);
IButtonScanner scanner(sdev);

BME280I2C bmp;

void setup () {
  delay(2000); // wait until Maple Mini USB-Serial is available
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  Wire.begin();
  sdev.init(hal);
  remoteChannelISR(sdev.bellChannel(),BELL_BUTTON_PIN);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  sdev.openerChannel().init(DOOR_PIN);
  sdev.initDone();

  sysclock.add(scanner);

  pinMode(BELL_LIGHT_PIN,PWM);
  pwmWrite(BELL_LIGHT_PIN, 2048);

  DHEX(sdev.bellChannel().getList1().address()); DPRINT("  ");
  sdev.bellChannel().getList1().dump();
  for( uint8_t i=0; i<IBUTTON_CHANNELS; ++i ) {
    DHEX(sdev.ibutChannel(i).getList1().address()); DPRINT("  ");
    sdev.ibutChannel(i).getList1().dump();
  }
  DHEX(sdev.openerChannel().getList1().address()); DPRINT("  ");
  sdev.openerChannel().getList1().dump();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
//    hal.activity.savePower<Sleep<>>(hal);
  }
}
