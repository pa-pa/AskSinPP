//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2019-09-30 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-09-30 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __RESETONBOOT_H__
#define __RESETONBOOT_H__

#define BOOT_STATE_NORMAL     0x00
#define BOOT_STATE_PRE_RESET  0x01
#define BOOT_STATE_RESET      0x02

#include <AskSinPP.h>

namespace as {

template <class DEVTYPE>
class ResetOnBoot : public Alarm {
  DEVTYPE& dev;
private:
  uint8_t cnt;
  uint8_t ms;
public:
  ResetOnBoot (DEVTYPE& d) : Alarm(0), dev(d), cnt(0), ms(200) { async(true); }
  virtual ~ResetOnBoot() {}

  void setBootState(uint8_t state) {
    StorageConfig sc = dev.getConfigArea();
    sc.setByte(CONFIG_BOOTSTATE, state);
    //DPRINT(F("SET NEXT BOOT STATE    : "));DDECLN(state);
    sc.validate();
  }

  uint8_t getBootState() {
    StorageConfig sc = dev.getConfigArea();
    //DPRINT(F("GET CURRENT BOOT STATE : "));DDECLN(sc.getByte(CONFIG_BOOTSTATE));
    return sc.getByte(CONFIG_BOOTSTATE);
  }

  void finish() {
    dev.led().ledOff();
    setBootState(BOOT_STATE_NORMAL);
  }

  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    if (cnt < (4000 / ms)) {
      cnt++;
      cnt % 2 == 0 ? dev.led().ledOn() : dev.led().ledOff();
      tick = millis2ticks(ms);
      clock.add(*this);
    } else {
      finish();
    }
  }

  void init() {
    if (getBootState() == BOOT_STATE_RESET) {
      DPRINT(F("Activated RESET ON BOOT"));
      finish();
      dev.reset();
    } else if (getBootState() == BOOT_STATE_PRE_RESET) {
      setBootState(BOOT_STATE_RESET);
      ms = 100;
    } else if (getBootState() == BOOT_STATE_NORMAL) {
      setBootState(BOOT_STATE_PRE_RESET);
    }
    set(millis2ticks(ms));
    sysclock.add(*this);
  }
};

}
#endif
