//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-03-29 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __DIMMER_H__
#define __DIMMER_H__

#include "AlarmClock.h"
#include "ChannelList.h"
#include "cm.h"

#define LOGIC_INACTIVE 0
#define LOGIC_OR 1
#define LOGIC_AND 2
#define LOGIC_XOR 3
#define LOGIC_NOR 4
#define LOGIC_NAND 5
#define LOGIC_ORINVERS 6
#define LOGIC_ANDINVERS 7
#define LOGIC_PLUS 8
#define LOGIC_MINUS 9
#define LOGIC_MUL 10
#define LOGIC_PLUSINVERS 11
#define LOGIC_MINUSINVERS 12
#define LOGIC_MULINVERS 13
#define LOGIC_INVERSPLUS 14
#define LOGIC_INVERSMINUS 15
#define LOGIC_INVERSMUL 16


namespace as {

class DimmerList1Data {
public:
  uint8_t aesActive        :1; // 0x08, s:0, e:1
  uint8_t                  :7; //
  uint8_t transmitTryMax   :8; // 0x30, s:0, e:8
  uint8_t overtempLevel    :8; // 0x32 (50)
  uint8_t reduceTempLevel  :8; // 0x34 (52)
  uint8_t reduceLevel      :8; // 0x35 (53)
  uint8_t powerUpAction    :1; // 0x56, s:0, e:1
  uint8_t                  :7; // 0x56, s:1, e:8
  uint8_t statusInfoMinDly :5; // 0x57, s:0, e:5
  uint8_t statusInfoRandom :3; // 0x57, s:5, e:8
  uint8_t characteristic   :1; // 0x58 (88) 0.1
  uint8_t                  :7; // 0x58 (88) 0.7
  uint8_t logicCombination :5; // 0x59 (89) 0.5
  uint8_t                  :3; // 0x59 (88) 0.3

  static uint8_t getOffset(uint8_t reg) {
    switch (reg) {
      case 0x08: return 0;
      case 0x30: return 1;
      case 0x32: return 2;
      case 0x34: return 3;
      case 0x35: return 4;
      case 0x56: return 5;
      case 0x57: return 6;
      case 0x58: return 7;
      case 0x59: return 8;
      default:   break;
    }
    return 0xff;
  }

  static uint8_t getRegister(uint8_t offset) {
    switch (offset) {
      case 0:  return 0x08;
      case 1:  return 0x30;
      case 2:  return 0x32;
      case 3:  return 0x34;
      case 4:  return 0x35;
      case 5:  return 0x56;
      case 6:  return 0x57;
      case 7:  return 0x58;
      case 8:  return 0x59;
      default: break;
    }
    return 0xff;
  }
};


class DimmerList1 : public ChannelList<DimmerList1Data> {
public:
  DimmerList1(uint16_t a) : ChannelList(a) {}

  bool aesActive () const { return isBitSet(0,0x01); }
  bool aesActive (bool s) const { return setBit(0,0x01,s); }

  uint8_t transmitTryMax () { return getByte(1); }
  bool transmitTryMax (uint8_t v) { return setByte(1,v); }

  bool powerUpAction () const { return isBitSet(5,0x01); }
  bool powerUpAction (bool s) const { return setBit(5,0x01,s); }

  uint8_t statusInfoMinDly () const { return getByte(6) & 0x1f; }
  bool statusInfoMinDly (uint8_t value) { return setByte(6,value,0x1f,0); }
  uint8_t statusInfoRandom () const { return getByte(6) >> 5; }
  bool statusInfoRandom (uint8_t value) { return setByte(6,value,0xe0,5); }

  uint8_t overtempLevel () const { return getByte(2); }
  bool overtempLevel (uint8_t v) const { return setByte(2,v); }
  uint8_t reduceTempLevel () const { return getByte(3); }
  bool reduceTempLevel (uint8_t v) const { return setByte(3,v); }
  uint8_t reduceLevel () const { return getByte(4); }
  bool reduceLevel (uint8_t v) const { return setByte(4,v); }

  bool characteristic () const { return isBitSet(7,0x01); }
  bool characteristic (bool s) const { return setBit(7,0x01,s); }
  uint8_t logicCombination () const { return getByte(8) & 0x1f; }
  bool logicCombination (uint8_t value) { return setByte(8,value,0x1f,0); }

  void defaults () {
    setByte(0,0);
    aesActive(false);
    transmitTryMax(6);
    powerUpAction(false);
    statusInfoMinDly(4);
    statusInfoRandom(1);

    overtempLevel(80);
    reduceTempLevel(75);
    reduceLevel(80);
    characteristic(true);
    logicCombination(LOGIC_OR);
  }
};

class DimmerPeerData {
public:
  uint8_t  ctRampOn           :4;     // 0x01, s:0, e:4
  uint8_t  ctRampOff          :4;     // 0x01, s:4, e:8
  uint8_t  ctDlyOn            :4;     // 0x02, s:0, e:4
  uint8_t  ctDlyOff           :4;     // 0x02, s:4, e:8
  uint8_t  ctOn               :4;     // 0x03, s:0, e:4
  uint8_t  ctOff              :4;     // 0x03, s:4, e:8
  uint8_t  ctValLo;                   // 0x04, s:0, e:0
  uint8_t  ctValHi;                   // 0x05, s:0, e:0
  uint8_t  onDly;                     // 0x06, s:0, e:0
  uint8_t  onTime;                    // 0x07, s:0, e:0
  uint8_t  offDly;                    // 0x08, s:0, e:0
  uint8_t  offTime;                   // 0x09, s:0, e:0
  uint8_t  actionType         :4;     // 0x0a, s:0, e:4
  uint8_t                     :1;     //
  uint8_t  multiExec          :1;     // 0x0a, s:5, e:6
  uint8_t  offTimeMode        :1;     // 0x0a, s:6, e:7
  uint8_t  onTimeMode         :1;     // 0x0a, s:7, e:8
  uint8_t  jtOn               :4;     // 0x0b, s:0, e:4
  uint8_t  jtOff              :4;     // 0x0b, s:4, e:8
  uint8_t  jtDlyOn            :4;     // 0x0c, s:0, e:4
  uint8_t  jtDlyOff           :4;     // 0x0c, s:4, e:8
  uint8_t  jtRampOn           :4;     // 0x0d, s:0, e:4
  uint8_t  jtRampOff          :4;     // 0x0d, s:4, e:8
  uint8_t  offDelayBlink      :1;     // 0x0e, s:5, e:6
  uint8_t  onLevelPrio        :1;     // 0x0e, s:6, e:7
  uint8_t  onDelayMode        :1;     // 0x0e, s:7, e:8
  uint8_t  offLevel           :8;     // 0x0f, s:0, e:8
  uint8_t  onMinLevel         :8;     // 0x10, s:0, e:8
  uint8_t  onLevel            :8;     // 0x11, s:0, e:8
  uint8_t  rampStartStep      :8;     // 0x12, s:0, e:8
  uint8_t  rampOnTime         :8;     // 0x13, s:0, e:8
  uint8_t  rampOffTime        :8;     // 0x14, s:0, e:8
  uint8_t  dimMinLevel        :8;     // 0x15, s:0, e:8
  uint8_t  dimMaxLevel        :8;     // 0x16, s:0, e:8
  uint8_t  dimStep            :8;     // 0x17, s:0, e:8
  uint8_t  offDelayStep       :8;     // 0x18, s:0, e:8
  uint8_t  offDelayNewTime    :8;     // 0x19, s:0, e:8
  uint8_t  offDelayOldTime    :8;     // 0x1a, s:0, e:8
  uint8_t  elseActionType     :4;     // 0x26, s:0, e:4
  uint8_t  elseOffTimeMode    :1;     // 0x26, s:6, e:7
  uint8_t  elseOnTimeMode     :1;     // 0x26, s:7, e:8
  uint8_t  elseJtOn           :4;     // 0x27, s:0, e:4
  uint8_t  elseJtOff          :4;     // 0x27, s:4, e:8
  uint8_t  elseJtOnDelay      :4;     // 0x28, s:0, e:4
  uint8_t  elseJtOffDelay     :4;     // 0x28, s:4, e:8
  uint8_t  elseJtRampOn       :4;     // 0x29, s:0, e:4
  uint8_t  elseJtRampOff      :4;     // 0x29, s:4, e:8

  static uint8_t getRegister (uint8_t off) {
    if( off < sizeof(DimmerPeerData)-4 ) {
      return off + 1;
    }
    else if( off < sizeof(DimmerPeerData) ) {
      return off + (0x26 - 0x1a);
    }
    return 0xff;
  }

  static uint8_t getOffset (uint8_t reg) {
    if( reg >= 0x02 && reg <= 0x1a ) {
      return reg - 1;
    }
    else if( reg <= 0x29 ) {
      return reg - (0x26 - 0x1a);
    }
    return 0xff;
  }

};

class DimmerList3Data {
public:
  DimmerPeerData sh;
  DimmerPeerData lg;

  static uint8_t getRegister (uint8_t off) {
    if( off < sizeof(DimmerPeerData) ) {
      return DimmerPeerData::getRegister(off);
    }
    else if( off < sizeof(DimmerList3Data)) {
      return 0x80 + DimmerPeerData::getRegister(off - sizeof(DimmerPeerData));
    }
    return 0xff;
  }

  static uint8_t getOffset (uint8_t reg) {
    uint8_t off = ((reg & 0x80) == 0x80) ? sizeof(DimmerPeerData) : 0;
    uint8_t result = DimmerPeerData::getOffset(reg & ~0x80);
    if( result != 0xff ) {
      result += off;
    }
    return result;
  }

};

class DimmerPeerList : public ChannelList<DimmerPeerData> {
public:
  DimmerPeerList(uint16_t a) : ChannelList(a) {}

  uint8_t ctRampOn () const { return getByte(0,0x0f,0); }
  bool ctRampOn (uint8_t value) const { return setByte(0,value,0x0f,0); }
  uint8_t ctRampOff () const { return getByte(0,0xf0,4); }
  bool ctRampOff (uint8_t value) const { return setByte(0,value,0xf0,4); }

  uint8_t ctDlyOn () const { return getByte(1,0x0f,0); }
  bool ctDlyOn (uint8_t value) const { return setByte(1,value,0x0f,0); }
  uint8_t ctDlyOff () const { return getByte(1,0xf0,4); }
  bool ctDlyOff (uint8_t value) const { return setByte(1,value,0xf0,4); }

  uint8_t ctOn () const { return getByte(2,0x0f,0); }
  bool ctOn (uint8_t value) const { return setByte(2,value,0x0f,0); }
  uint8_t ctOff () const { return getByte(2,0xf0,4); }
  bool ctOff (uint8_t value) const { return setByte(2,value,0xf0,4); }

  uint8_t ctValLo() const { return getByte(3); }
  bool ctValLo(uint8_t value) const { return setByte(3,value); }
  uint8_t ctValHi() const { return getByte(4); }
  bool ctValHi(uint8_t value) const { return setByte(4,value); }
  uint8_t onDly() const { return getByte(5); }
  bool onDly(uint8_t value) const { return setByte(5,value); }
  uint8_t onTime() const { return getByte(6); }
  bool onTime(uint8_t value) const { return setByte(6,value); }
  uint8_t offDly() const { return getByte(7); }
  bool offDly(uint8_t value) const { return setByte(7,value); }
  uint8_t offTime() const { return getByte(8); }
  bool offTime(uint8_t value) const { return setByte(8,value); }

  uint8_t actionType() const { return getByte(9,0x0f,0); }
  bool actionType(uint8_t type) const { return setByte(9,type,0x0f,0); }
  bool multiExec() const { return isBitSet(9,0x20); }
  bool multiExec(bool s) const { return setBit(9,0x20,s); }
  bool offTimeMode() const { return isBitSet(9,0x40); }
  bool offTimeMode(bool s) const { return setBit(9,0x40,s); }
  bool onTimeMode() const { return isBitSet(9,0x80); }
  bool onTimeMode(bool s) const { return setBit(9,0x80,s); }

  uint8_t jtOn() const { return getByte(10,0x0f,0); }
  bool jtOn (uint8_t value) const { return setByte(10,value,0x0f,0); }
  uint8_t jtOff() const { return getByte(10,0xf0,4); }
  bool jtOff (uint8_t value) const { return setByte(10,value,0xf0,4); }
  uint8_t jtDlyOn() const { return getByte(11,0x0f,0); }
  bool jtDlyOn (uint8_t value) const { return setByte(11,value,0x0f,0); }
  uint8_t jtDlyOff() const { return getByte(11,0xf0,4); }
  bool jtDlyOff (uint8_t value) const { return setByte(11,value,0xf0,4); }
  uint8_t jtRampOn() const { return getByte(12,0x0f,0); }
  bool jtRampOn (uint8_t value) const { return setByte(12,value,0x0f,0); }
  uint8_t jtRampOff() const { return getByte(12,0xf0,4); }
  bool jtRampOff (uint8_t value) const { return setByte(12,value,0xf0,4); }

  bool offDelayBlink() const { return isBitSet(13,0x20); }
  bool offDelayBlink(bool s) const { return setBit(13,0x20,s); }
  bool onLevelPrio() const { return isBitSet(13,0x40); }
  bool onLevelPrio(bool s) const { return setBit(13,0x40,s); }
  bool onDelayMode() const { return isBitSet(13,0x80); }
  bool onDelayMode(bool s) const { return setBit(13,0x80,s); }

  uint8_t offLevel() const { return getByte(14); }
  bool offLevel(uint8_t value) const { return setByte(14,value); }
  uint8_t onMinLevel() const { return getByte(15); }
  bool onMinLevel(uint8_t value) const { return setByte(15,value); }
  uint8_t onLevel() const { return getByte(16); }
  bool onLevel(uint8_t value) const { return setByte(16,value); }
  uint8_t rampStartStep() const { return getByte(17); }
  bool rampStartStep(uint8_t value) const { return setByte(17,value); }
  uint8_t rampOnTime() const { return getByte(18); }
  bool rampOnTime(uint8_t value) const { return setByte(18,value); }
  uint8_t rampOffTime() const { return getByte(19); }
  bool rampOffTime(uint8_t value) const { return setByte(19,value); }
  uint8_t dimMinLevel() const { return getByte(20); }
  bool dimMinLevel(uint8_t value) const { return setByte(20,value); }
  uint8_t dimMaxLevel() const { return getByte(21); }
  bool dimMaxLevel(uint8_t value) const { return setByte(21,value); }
  uint8_t dimStep() const { return getByte(22); }
  bool dimStep(uint8_t value) const { return setByte(22,value); }
  uint8_t offDelayStep() const { return getByte(23); }
  bool offDelayStep(uint8_t value) const { return setByte(23,value); }
  uint8_t offDelayNewTime() const { return getByte(24); }
  bool offDelayNewTime(uint8_t value) const { return setByte(24,value); }
  uint8_t offDelayOldTime() const { return getByte(24); }
  bool offDelayOldTime(uint8_t value) const { return setByte(24,value); }

  uint8_t elseActionType() const { return getByte(25,0x0f,0); }
  bool elseActionType(uint8_t type) const { return setByte(25,type,0x0f,0); }
  bool elseOffTimeMode() const { return isBitSet(25,0x40); }
  bool elseOffTimeMode(bool s) const { return setBit(25,0x40,s); }
  bool elseOnTimeMode() const { return isBitSet(25,0x80); }
  bool elseOnTimeMode(bool s) const { return setBit(25,0x80,s); }

  uint8_t elseJtOn() const { return getByte(26,0x0f,0); }
  bool elseJtOn (uint8_t value) const { return setByte(26,value,0x0f,0); }
  uint8_t elseJtOff() const { return getByte(26,0xf0,4); }
  bool elseJtOff (uint8_t value) const { return setByte(26,value,0xf0,4); }
  uint8_t elseJtDlyOn() const { return getByte(27,0x0f,0); }
  bool elseJtDlyOn (uint8_t value) const { return setByte(27,value,0x0f,0); }
  uint8_t elseJtDlyOff() const { return getByte(27,0xf0,4); }
  bool elseJtDlyOff (uint8_t value) const { return setByte(27,value,0xf0,4); }
  uint8_t elseJtRampOn() const { return getByte(28,0x0f,0); }
  bool elseJtRampOn (uint8_t value) const { return setByte(28,value,0x0f,0); }
  uint8_t elseJtRampOff() const { return getByte(28,0xf0,4); }
  bool elseJtRampOff (uint8_t value) const { return setByte(28,value,0xf0,4); }

};

class DimmerList3 : public ChannelList<DimmerList3Data> {
protected:
public:
  typedef DimmerPeerList PeerList;

  DimmerList3(uint16_t a) : ChannelList(a) {}

  const DimmerPeerList sh() const { return DimmerPeerList(address()); }
  const DimmerPeerList lg() const { return DimmerPeerList(address() + sizeof(DimmerPeerData)); }

  void defaults() {
    DimmerPeerList ssl = sh();
    ssl.ctRampOn(0);
    ssl.ctRampOff(0);
    ssl.ctDlyOn(0);
    ssl.ctDlyOff(0);
    ssl.ctOn(0);
    ssl.ctOff(0);
    ssl.ctValLo(0x32);
    ssl.ctValHi(0x64);
    ssl.onDly(0);
    ssl.onTime(0xff);
    ssl.offDly(0);
    ssl.offTime(0xff);
    ssl.actionType(AS_CM_ACTIONTYPE_JUMP_TO_TARGET);
    ssl.offTimeMode(false);
    ssl.onTimeMode(false);
    ssl.offDelayBlink(true);
    ssl.offLevel(0);
    ssl.onMinLevel(20);
    ssl.onLevel(200); // 201 ???
    ssl.rampStartStep(10);
    ssl.rampOnTime(0);
    ssl.rampOffTime(0);
    ssl.dimMinLevel(0);
    ssl.dimMaxLevel(200);
    ssl.dimStep(5);
    ssl.offDelayStep(10);
    ssl.offDelayNewTime(5);
    ssl.offDelayOldTime(5);
    ssl.elseActionType(AS_CM_ACTIONTYPE_INACTIVE);
    ssl.elseOffTimeMode(false);
    ssl.elseOnTimeMode(false);
    ssl.elseJtOn(AS_CM_JT_ONDELAY);
    ssl.elseJtOff(AS_CM_JT_ONDELAY);
    ssl.elseJtDlyOn(AS_CM_JT_ONDELAY);
    ssl.elseJtDlyOff(AS_CM_JT_ONDELAY);
    ssl.elseJtRampOn(AS_CM_JT_ONDELAY);
    ssl.elseJtRampOff(AS_CM_JT_ONDELAY);


    ssl = lg();
    ssl.ctRampOn(0);
    ssl.ctRampOff(0);
    ssl.ctDlyOn(0);
    ssl.ctDlyOff(0);
    ssl.ctOn(0);
    ssl.ctOff(0);
    ssl.ctValLo(0x32);
    ssl.ctValHi(0x64);
    ssl.onDly(0);
    ssl.onTime(0xff);
    ssl.offDly(0);
    ssl.offTime(0xff);
    ssl.actionType(AS_CM_ACTIONTYPE_JUMP_TO_TARGET);
    ssl.multiExec(true);
    ssl.offTimeMode(false);
    ssl.onTimeMode(false);
    ssl.offDelayBlink(true);
    ssl.offLevel(0);
    ssl.onMinLevel(20);
    ssl.onLevel(200); // 201 ???
    ssl.rampStartStep(10);
    ssl.rampOnTime(0);
    ssl.rampOffTime(0);
    ssl.dimMinLevel(0);
    ssl.dimMaxLevel(200);
    ssl.dimStep(5);
    ssl.offDelayStep(10);
    ssl.offDelayNewTime(5);
    ssl.offDelayOldTime(5);
    ssl.elseActionType(AS_CM_ACTIONTYPE_INACTIVE);
    ssl.elseOffTimeMode(false);
    ssl.elseOnTimeMode(false);
    ssl.elseJtOn(AS_CM_JT_ONDELAY);
    ssl.elseJtOff(AS_CM_JT_ONDELAY);
    ssl.elseJtDlyOn(AS_CM_JT_ONDELAY);
    ssl.elseJtDlyOff(AS_CM_JT_ONDELAY);
    ssl.elseJtRampOn(AS_CM_JT_ONDELAY);
    ssl.elseJtRampOff(AS_CM_JT_ONDELAY);
  }

  void odd() {
    defaults();
    DimmerPeerList ssl = sh();
    ssl.jtOn(AS_CM_JT_OFFDELAY);
    ssl.jtOff(AS_CM_JT_OFF);
    ssl.jtDlyOn(AS_CM_JT_RAMPOFF);
    ssl.jtDlyOff(AS_CM_JT_RAMPOFF);
    ssl.jtRampOn(AS_CM_JT_RAMPOFF);
    ssl.jtRampOff(AS_CM_JT_OFF);
    ssl = lg();
    ssl.actionType(AS_CM_ACTIONTYPE_DOWNDIM);
//    ssl.jtOn(AS_CM_JT_OFFDELAY);
//    ssl.jtOff(AS_CM_JT_OFF);
//    ssl.jtDlyOn(AS_CM_JT_RAMPOFF);
//    ssl.jtDlyOff(AS_CM_JT_RAMPOFF);
//    ssl.jtRampOn(AS_CM_JT_RAMPOFF);
//    ssl.jtRampOff(AS_CM_JT_OFF);
  }

  void even() {
    defaults();
    DimmerPeerList ssl = sh();
    ssl.jtOn(AS_CM_JT_ON);
    ssl.jtOff(AS_CM_JT_ONDELAY);
    ssl.jtDlyOn(AS_CM_JT_RAMPON);
    ssl.jtDlyOff(AS_CM_JT_RAMPON);
    ssl.jtRampOn(AS_CM_JT_ON);
    ssl.jtRampOff(AS_CM_JT_RAMPON);
    ssl = lg();
    ssl.actionType(AS_CM_ACTIONTYPE_UPDIM);
//    ssl.jtOn(AS_CM_JT_ON);
//    ssl.jtOff(AS_CM_JT_ONDELAY);
//    ssl.jtDlyOn(AS_CM_JT_RAMPON);
//    ssl.jtDlyOff(AS_CM_JT_RAMPON);
//    ssl.jtRampOn(AS_CM_JT_ON);
//    ssl.jtRampOff(AS_CM_JT_RAMPON);
  }

  void single() {
    defaults();
    DimmerPeerList ssl = sh();
    ssl.jtOn(AS_CM_JT_OFFDELAY);
    ssl.jtOff(AS_CM_JT_ONDELAY);
    ssl.jtDlyOn(AS_CM_JT_RAMPON);
    ssl.jtDlyOff(AS_CM_JT_RAMPOFF);
    ssl.jtRampOn(AS_CM_JT_ON);
    ssl.jtRampOff(AS_CM_JT_OFF);
    ssl = lg();
    ssl.actionType(AS_CM_ACTIONTYPE_TOGGLEDIM_TO_COUNTER);
//    ssl.jtOn(AS_CM_JT_OFFDELAY);
//    ssl.jtOff(AS_CM_JT_ONDELAY);
//    ssl.jtDlyOn(AS_CM_JT_RAMPON);
//    ssl.jtDlyOff(AS_CM_JT_RAMPOFF);
//    ssl.jtRampOn(AS_CM_JT_ON);
//    ssl.jtRampOff(AS_CM_JT_OFF);
  }
};


class DimmerStateMachine {

#define DELAY_NO 0x00
#define DELAY_INFINITE 0xffffffff

  class StateAlarm : public Alarm {
    DimmerStateMachine& sm;
    DimmerPeerList      lst;
  public:
    StateAlarm(DimmerStateMachine& m) : Alarm(0), sm(m), lst(0) {}
    void list(DimmerPeerList l) {lst=l;}
    virtual void trigger (AlarmClock& clock) {
      uint8_t next = sm.getNextState();
      uint32_t dly = sm.getDelayForState(next,lst);
      sm.setState(next,dly,lst);
    }
  };

  void setState (uint8_t next,uint32_t delay,const DimmerPeerList& lst=DimmerPeerList(0),uint8_t deep=0) {
    // check deep to prevent infinite recursion
    if( next != AS_CM_JT_NONE && deep < 4) {
      // first cancel possible running alarm
      aclock.cancel(alarm);
      // if state is different
      if (state != next) {
        switchState(state, next);
        state = next;
      }
      if (delay == DELAY_NO) {
        // go immediately to the next state
        next = getNextState();
        delay = getDelayForState(next,lst);
        setState(next, delay, lst, ++deep);
      }
      else if (delay != DELAY_INFINITE) {
        alarm.list(lst);
        alarm.set(delay);
        aclock.add(alarm);
      }
    }
  }

protected:
  uint8_t    state;
  uint8_t    level;
  StateAlarm alarm;

public:
  DimmerStateMachine() : state(AS_CM_JT_NONE), level(0), alarm(*this) {}
  virtual ~DimmerStateMachine () {}

  virtual void switchState(uint8_t oldstate,uint8_t newstate) {
    DPRINT("Switch State: ");DHEX(oldstate);DPRINT(" -> ");DHEXLN(newstate);
  }

  void jumpToTarget(const DimmerPeerList& lst) {
    uint8_t next = getJumpTarget(state,lst);
    if( next != AS_CM_JT_NONE ) {
      // get delay
      uint32_t dly = getDelayForState(next,lst);
      // switch to next
      setState(next,dly,lst);
    }
  }

  void toggleState () {
    setState( state == AS_CM_JT_ON ? AS_CM_JT_OFF : AS_CM_JT_ON, DELAY_INFINITE);
  }

  uint8_t getNextState () {
    switch( state ) {
      case AS_CM_JT_ONDELAY:  return AS_CM_JT_RAMPON;
      case AS_CM_JT_RAMPON:   return AS_CM_JT_ON;
      case AS_CM_JT_ON:       return AS_CM_JT_OFFDELAY;
      case AS_CM_JT_OFFDELAY: return AS_CM_JT_RAMPOFF;
      case AS_CM_JT_RAMPOFF:  return AS_CM_JT_OFF;
      case AS_CM_JT_OFF:      return AS_CM_JT_ONDELAY;
    }
    return AS_CM_JT_NONE;
  }

  uint8_t getJumpTarget(uint8_t stat,const DimmerPeerList& lst) const {
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  return lst.jtDlyOn();
      case AS_CM_JT_RAMPON:   return lst.jtRampOn();
      case AS_CM_JT_ON:       return lst.jtOn();
      case AS_CM_JT_OFFDELAY: return lst.jtDlyOff();
      case AS_CM_JT_RAMPOFF:  return lst.jtRampOff();
      case AS_CM_JT_OFF:      return lst.jtOff();
    }
    return AS_CM_JT_NONE;
  }

  uint8_t getConditionForState(uint8_t stat,const DimmerPeerList& lst) const {
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  return lst.ctDlyOn();
      case AS_CM_JT_RAMPON:   return lst.ctRampOn();
      case AS_CM_JT_ON:       return lst.ctOn();
      case AS_CM_JT_OFFDELAY: return lst.ctDlyOff();
      case AS_CM_JT_RAMPOFF:  return lst.ctRampOff();
      case AS_CM_JT_OFF:      return lst.ctOff();
    }
    return AS_CM_CT_X_GE_COND_VALUE_LO;
  }

  uint32_t getDelayForState(uint8_t stat,const DimmerPeerList& lst) const {
    if( lst.valid() == false ) {
      return getDefaultDelay(stat);
    }
    uint8_t value = 0;
    switch( stat ) {
      case AS_CM_JT_ONDELAY:  value = lst.onDly(); break;
      case AS_CM_JT_RAMPON:   value = lst.rampOnTime(); break;
      case AS_CM_JT_ON:       value = lst.onTime(); break;
      case AS_CM_JT_OFFDELAY: value = lst.offDly(); break;
      case AS_CM_JT_RAMPOFF:  value = lst.rampOffTime(); break;
      case AS_CM_JT_OFF:      value = lst.offTime(); break;
    }
    return byteTimeCvt(value);
  }

  uint32_t getDefaultDelay(uint8_t stat) const {
    switch( stat ) {
      case AS_CM_JT_ON:
      case AS_CM_JT_OFF:
        return DELAY_INFINITE;
    }
    return DELAY_NO;
  }

  bool delayActive () const { return aclock.get(alarm) > 0; }

  // get timer count in ticks
  static uint32_t byteTimeCvt(uint8_t tTime) {
    if( tTime == 0xff ) return 0xffffffff;
    const uint16_t c[8] = {1,10,50,100,600,3000,6000,36000};
    return decis2ticks( (uint32_t)(tTime & 0x1F) * c[tTime >> 5] );
  }

  // get timer count in ticks
  static uint32_t intTimeCvt(uint16_t iTime) {
    if (iTime == 0x00) return 0x00;
    if (iTime == 0xffff) return 0xffffffff;

    uint8_t tByte;
    if ((iTime & 0x1F) != 0) {
      tByte = 2;
      for (uint8_t i = 1; i < (iTime & 0x1F); i++) tByte *= 2;
    } else tByte = 1;

    return decis2ticks( (uint32_t)tByte*(iTime>>5) );
  }

  void remote (const DimmerPeerList& lst,uint8_t counter) {
    // perform action as defined in the list
    switch (lst.actionType()) {
    case AS_CM_ACTIONTYPE_JUMP_TO_TARGET:
      jumpToTarget(lst);
      break;
    case AS_CM_ACTIONTYPE_TOGGLE_TO_COUNTER:
//      setState((counter & 0x01) == 0x01 ? AS_CM_JT_ON : AS_CM_JT_OFF, DELAY_INFINITE);
      break;
    case AS_CM_ACTIONTYPE_TOGGLE_INVERSE_TO_COUNTER:
//      setState((counter & 0x01) == 0x00 ? AS_CM_JT_ON : AS_CM_JT_OFF, DELAY_INFINITE);
      break;
    case AS_CM_ACTIONTYPE_UPDIM:
      break;
    case AS_CM_ACTIONTYPE_DOWNDIM:
      break;
    case AS_CM_ACTIONTYPE_TOGGLEDIM:
      break;
    case AS_CM_ACTIONTYPE_TOGGLEDIM_TO_COUNTER:
      break;
    case AS_CM_ACTIONTYPE_TOGGLEDIM_TO_COUNTER_INVERSE:
      break;
    }

  }

  void sensor (const DimmerPeerList& lst,uint8_t counter,uint8_t value) {
    uint8_t cond = getConditionForState(state,lst);
    bool doit = false;
    switch( cond ) {
    case AS_CM_CT_X_GE_COND_VALUE_LO:
      doit = (value >= lst.ctValLo());
      break;
    case AS_CM_CT_X_GE_COND_VALUE_HI:
      doit = (value >= lst.ctValHi());
      break;
    case AS_CM_CT_X_LT_COND_VALUE_LO:
      doit = (value < lst.ctValLo());
      break;
    case AS_CM_CT_X_LT_COND_VALUE_HI:
      doit = (value < lst.ctValHi());
      break;
    case AS_CM_CT_COND_VALUE_LO_LE_X_LT_COND_VALUE_HI:
      doit = ((lst.ctValLo() <= value) && (value < lst.ctValHi()));
      break;
    case AS_CM_CT_X_LT_COND_VALUE_LO_OR_X_GE_COND_VALUE_HI:
      doit =((value < lst.ctValLo()) || (value >= lst.ctValHi()));
      break;
    }
    if( doit == true ) {
      remote(lst,counter);
    }
    else {
      // TODO use else jump table
    }
  }

  void status (uint8_t stat, uint16_t delay) {
    setState( stat == 0 ? AS_CM_JT_OFF : AS_CM_JT_ON, intTimeCvt(delay) );
  }

  uint8_t status () const {
    return level;
  }

  uint8_t flags () const {
    return delayActive() ? 0x40 : 0x00;
  }
};

}

#endif
