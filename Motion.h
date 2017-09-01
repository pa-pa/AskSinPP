//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-05-10 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef LIBRARIES_ASKSINPP_MOTION_H_
#define LIBRARIES_ASKSINPP_MOTION_H_

#include "MultiChannelDevice.h"

extern uint8_t measureBrightness ();

namespace as {

class MotionList1Data {
public:
  uint8_t EventFilterPeriod : 4;     // 0x01
  uint8_t EventFilterNumber : 4;     // 0x01
  uint8_t MinInterval       : 3;     // 0x02
  uint8_t CaptureWithinInterval : 1; // 0x02
  uint8_t BrightnessFilter  : 4;     // 0x02
  uint8_t AesActive         :1;      // 0x08, s:0, e:1
  uint8_t LedOntime;                 // 0x20

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x01: return 0;
      case 0x02: return 1;
      case 0x08: return 2;
      case 0x20: return 3;
      default: break;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x01;
      case 1:  return 0x02;
      case 2:  return 0x08;
      case 3:  return 0x20;
      default: break;
    }
    return 0xff;
  }
};

class MotionList1 : public ChannelList<MotionList1Data> {
public:
  MotionList1(uint16_t a) : ChannelList(a) {}

  uint8_t eventFilterPeriod () const { return getByte(0,0x0f,0); }
  bool eventFilterPeriod (uint8_t value) const { return setByte(0,value,0x0f,0); }
  uint8_t eventFilterNumber () const { return getByte(0,0xf0,4); }
  bool eventFilterNumber (uint8_t value) const { return setByte(0,value,0xf0,4); }

  uint8_t minInterval () const { return getByte(1,0x07,0); }
  bool minInterval (uint8_t value) const { return setByte(1,value,0x07,0); }
  bool captureWithinInterval () const { return isBitSet(1,0x08); }
  bool captureWithinInterval (bool value) const { return setBit(1,0x08,value); }
  uint8_t brightnessFilter () const { return getByte(1,0xf0,4); }
  bool brightnessFilter (uint8_t value) const { return setByte(1,value,0xf0,4); }

  bool aesActive () const { return isBitSet(2,0x01); }
  bool aesActive (bool s) const { return setBit(2,0x01,s); }

  uint8_t ledOntime () const { return getByte(3); }
  bool ledOntime (uint8_t value) const { return setByte(3,value); }

  void defaults () {
    eventFilterPeriod(1);
    eventFilterNumber(1);
    minInterval(4);
    captureWithinInterval(false);
    brightnessFilter(7);
    aesActive(false);
    ledOntime(100);
  }
};

class MotionEventMsg : public Message {
public:
  void init(uint8_t msgcnt,uint8_t ch,uint8_t counter,uint8_t brightness,uint8_t next) {
    Message::init(0xd,msgcnt,0x41,Message::BIDI|Message::WKMEUP,ch & 0x3f,counter);
    pload[0] = brightness;
    pload[1] = (next+4) << 4;
  }
};

template <class HalType,int PeerCount>
class MotionChannel : public Channel<HalType,MotionList1,EmptyList,List4,PeerCount>, public Alarm {

  class QuietMode : public Alarm {
  public:
    bool  enabled;
    bool  motion;
    MotionChannel& channel;
    QuietMode (MotionChannel& c) : Alarm(0), enabled(false), motion(false), channel(c) {}
    virtual ~QuietMode () {}
    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      DPRINTLN(F("minInterval End"));
      enabled = false;
      if( motion == true ) {
        motion = false;
        channel.motionDetected();
      }
    }
  };

  // send the brightness every 5 minutes to the master
  #define LIGHTCYCLE seconds2ticks(5*60)
  class Cycle : public Alarm {
  public:
    MotionChannel& channel;
    Cycle (MotionChannel& c) : Alarm(LIGHTCYCLE), channel(c) {}
    virtual ~Cycle () {}
    virtual void trigger (AlarmClock& clock) {
      tick = LIGHTCYCLE;
      clock.add(*this);
      channel.sendState();
    }
  };

  // return timer ticks
  uint32_t getMinInterval () {
    switch( ChannelType::getList1().minInterval() ) {
      case 0: return seconds2ticks(15);
      case 1: return seconds2ticks(30);
      case 2: return seconds2ticks(60);
      case 3: return seconds2ticks(120);
    }
    return seconds2ticks(240);
  }

private:
  uint8_t          counter;
  QuietMode        quiet;
  Cycle            cycle;
  volatile bool    isrenabled : 1;

public:
  typedef Channel<HalType,MotionList1,EmptyList,List4,PeerCount> ChannelType;

  MotionChannel () : ChannelType(), Alarm(0), counter(0), quiet(*this), cycle(*this), isrenabled(true) {
    sysclock.add(cycle);
    pirInterruptOn();
  }
  virtual ~MotionChannel () {}

  uint8_t status () const {
    return measureBrightness();
  }

  uint8_t flags () const {
    return ChannelType::device().battery().low() ? 0x80 : 0x00;
  }

  void sendState () {
    pirInterruptOff();
    typename ChannelType::DeviceType& d = ChannelType::device();
    d.sendInfoActuatorStatus(d.getMasterID(),d.nextcount(),*this);
    pirInterruptOn();
  }

  void pirInterruptOn () {
    isrenabled=true;
  }

  void pirInterruptOff () {
    isrenabled=false;
  }

  // this runs synch to application
  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    if( quiet.enabled == false ) {
  	  // reset state timer because motion will be send now
	    sysclock.cancel(cycle);
	    cycle.set(LIGHTCYCLE);
	    sysclock.add(cycle);
	  
      DPRINTLN(F("Motion"));
      // start timer to end quiet interval
      quiet.tick = getMinInterval();
      quiet.enabled = true;
      sysclock.add(quiet);
      // blink led
      if( ChannelType::device().led().active() == false ) {
        ChannelType::device().led().ledOn( centis2ticks(ChannelType::getList1().ledOntime()) / 2);
      }
      MotionEventMsg& msg = (MotionEventMsg&)ChannelType::device().message();
      msg.init(ChannelType::device().nextcount(),ChannelType::number(),++counter,status(),ChannelType::getList1().minInterval());
	  
	    pirInterruptOff();
      ChannelType::device().sendPeerEvent(msg,*this);
	    pirInterruptOn();
    }
    else if ( ChannelType::getList1().captureWithinInterval() == true ) {	
      // we have had a motion during quiet interval
      quiet.motion = true;
    }
  }

  // runs in interrupt
  void motionDetected () {
    if( isrenabled==true ) {
      // cancel may not needed but anyway
      sysclock.cancel(*this);
      // activate motion message handler
      sysclock.add(*this);
    }
  }
};

#define motionISR(device,chan,pin) class device##chan##ISRHandler { \
  public: \
  static void isr () { device.channel(chan).motionDetected(); } \
}; \
pinMode(pin,INPUT); \
enableInterrupt(pin,device##chan##ISRHandler::isr,RISING);

#define motionChannelISR(chan,pin) class __##pin##ISRHandler { \
  public: \
  static void isr () { chan.motionDetected(); } \
}; \
pinMode(pin,INPUT); \
enableInterrupt(pin,__##pin##ISRHandler::isr,RISING);


} // end namespace

#endif /* LIBRARIES_ASKSINPP_MOTION_H_ */
