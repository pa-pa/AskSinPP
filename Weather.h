//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2019-09-05 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __WEATHER_H__
#define __WEATHER_H__

#include <Message.h>
#include <Channel.h>

namespace as {

class WeatherEventMsg : public Message {
public:
  void init(uint8_t msgcnt,int16_t temp,uint8_t humidity, bool batlow) {
    uint8_t t1 = (temp >> 8) & 0x7f;
    uint8_t t2 = temp & 0xff;
    if( batlow == true ) {
      t1 |= 0x80; // set bat low bit
    }
    Message::init(0xc,msgcnt,0x70,BIDI,t1,t2);
    pload[0] = humidity;
  }
};

template <class HAL,class CLOCKTYPE,class SENSORSTYPE,int PEERS_PER_CHANNEL,int EXTRADELAY,class LIST0,class LIST1=List1,class LIST4=List4>
class WeatherChannel : public Channel<HAL,LIST1,EmptyList,LIST4,PEERS_PER_CHANNEL,LIST0>, protected RTCAlarm {

private:
  SENSORSTYPE sens;

public:
  WeatherChannel () : Channel<HAL,LIST1,EmptyList,LIST4,PEERS_PER_CHANNEL,LIST0>() {}
  virtual ~WeatherChannel () {}

  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    // check if delay for millis is active - only if using RTC
    if( delayMillis() == false ) {
      uint8_t msgcnt = this->device().nextcount();
      // send
      WeatherEventMsg& msg = (WeatherEventMsg&)this->device().message();
      msg.init(msgcnt,sens.temperature(),sens.humidity(),this->device().battery().low());
      this->device().broadcastEvent(msg);
      // reactivate for next send
      reactivate(msg);
    }
  }

  void reactivate (Message& msg) {
    uint32_t nextsend = AskSinBase::nextSendSlot(msg.from(),msg.count()) + EXTRADELAY;
    CLOCKTYPE::instance().add(*this,nextsend);
    // reactive measure before send
    CLOCKTYPE::instance().add(sens,nextsend-sens.before());
  }

  void setup(Device<HAL,LIST0>* dev,uint8_t number,uint16_t addr) {
    Channel<HAL,LIST1,EmptyList,LIST4,PEERS_PER_CHANNEL,LIST0>::setup(dev,number,addr);
    sens.init();
    CLOCKTYPE::instance().add(sens,1000);
    CLOCKTYPE::instance().add(*this,sens.before()+1000);
  }

  uint8_t status () const { return 0; }

  uint8_t flags () const { return this->device().battery().low() ? 0x80 : 0x00; }

  SENSORSTYPE& sensors () { return sens; }
};

}

#endif
