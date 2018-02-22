//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-05-10 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef LIBRARIES_ASKSINPP_MOTION_H_
#define LIBRARIES_ASKSINPP_MOTION_H_

#include "MultiChannelDevice.h"
#include "Register.h"
#include "Sensors.h"


namespace as {

DEFREGISTER(MotionReg1,CREG_EVENTFILTER,CREG_INTERVAL,CREG_AES_ACTIVE,CREG_LEDONTIME)
static const uint8_t MotionReg1Defaults[] = {0x11,0x74,0x00,0x64};
class MotionList1 : public RegList1<MotionReg1> {
public:
  MotionList1 (uint16_t addr) : RegList1<MotionReg1>(addr) {}
  void defaults () {
    init(MotionReg1Defaults,sizeof(MotionReg1Defaults));
//    clear();
//    eventFilterPeriod(1);
//    eventFilterNumber(1);
//    minInterval(4);
//    captureWithinInterval(false);
//    brightnessFilter(7);
//    aesActive(false);
//    ledOntime(100);
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

template <class HalType,int PeerCount,class List0Type=List0,class BrightnessSensor=Brightness>
class MotionChannel : public Channel<HalType,MotionList1,EmptyList,DefList4,PeerCount,List0Type>, public Alarm {

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
  BrightnessSensor brightsens;
  uint16_t         maxbright;

public:
  typedef Channel<HalType,MotionList1,EmptyList,DefList4,PeerCount,List0Type> ChannelType;

  MotionChannel () : ChannelType(), Alarm(0), counter(0), quiet(*this), cycle(*this), isrenabled(true),
      brightsens(), maxbright(1) {}
  virtual ~MotionChannel () {}

  void setup(Device<HalType,List0Type>* dev,uint8_t number,uint16_t addr) {
    ChannelType::setup(dev,number,addr);
    sysclock.add(cycle);
    pirInterruptOn();
    brightsens.init();
  }

  uint8_t status () {
    brightsens.measure();
    uint16_t bright = brightsens.brightness();
    if( bright > maxbright ) {
      maxbright = bright;
    }
    // scale to value between 0 - 200s
    return (uint8_t)(bright * 200UL / maxbright);
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
if( digitalPinToInterrupt(pin) == NOT_AN_INTERRUPT ) \
  enableInterrupt(pin,device##chan##ISRHandler::isr,RISING); \
else \
  attachInterrupt(digitalPinToInterrupt(pin),device##chan##ISRHandler::isr,RISING);

#define motionChannelISR(chan,pin) class __##pin##ISRHandler { \
  public: \
  static void isr () { chan.motionDetected(); } \
}; \
pinMode(pin,INPUT); \
if( digitalPinToInterrupt(pin) == NOT_AN_INTERRUPT ) \
  enableInterrupt(pin,__##pin##ISRHandler::isr,RISING); \
else \
  attachInterrupt(digitalPinToInterrupt(pin),__##pin##ISRHandler::isr,RISING);

} // end namespace

#endif /* LIBRARIES_ASKSINPP_MOTION_H_ */
