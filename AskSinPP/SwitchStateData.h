
#ifndef __SWITCHSTATEDATA_H__
#define __SWITCHSTATEDATA_H__

#include "ChannelList.h"
#include "cm.h"

class SwitchCtrlData {
public:
  uint8_t  ctDlyOn           :4;     // 0x02, s:0, e:4
  uint8_t  ctDlyOff          :4;     // 0x02, s:4, e:8
  uint8_t  ctOn              :4;     // 0x03, s:0, e:4
  uint8_t  ctOff             :4;     // 0x03, s:4, e:8
  uint8_t  ctValLo;                  // 0x04, s:0, e:0
  uint8_t  ctValHi;                  // 0x05, s:0, e:0
  uint8_t  onDly;                    // 0x06, s:0, e:0
  uint8_t  onTime;                   // 0x07, s:0, e:0
  uint8_t  offDly;                   // 0x08, s:0, e:0
  uint8_t  offTime;                  // 0x09, s:0, e:0
  uint8_t  actionType        :4;     // 0x0a, s:0, e:4
  uint8_t                    :1;     //
  uint8_t  multiExec         :1;     // 0x8a, s:5, e:6
  uint8_t  offTimeMode       :1;     // 0x0a, s:6, e:7
  uint8_t  onTimeMode        :1;     // 0x0a, s:7, e:8
  uint8_t  jtOn              :4;     // 0x0b, s:0, e:4
  uint8_t  jtOff             :4;     // 0x0b, s:4, e:8
  uint8_t  jtDlyOn           :4;     // 0x0c, s:0, e:4
  uint8_t  jtDlyOff          :4;     // 0x0c, s:4, e:8

  static uint8_t getRegister (uint8_t off) {
    if( off < sizeof(SwitchCtrlData) ) {
      return off + 2;
    }
    return 0xff;
  }

  static uint8_t getOffset (uint8_t reg) {
    if( reg >= 0x02 && reg <= 0x0c ) {
      return reg - 2;
    }
    return 0xff;
  }
};

class SwitchStateData {
public:
  SwitchCtrlData sh;
  SwitchCtrlData lg;

  static uint8_t getRegister (uint8_t off) {
    if( off < sizeof(SwitchCtrlData) ) {
      return SwitchCtrlData::getRegister(off);
    }
    else if( off < sizeof(SwitchStateData)) {
      return 0x80 + SwitchCtrlData::getRegister(off - sizeof(SwitchCtrlData));
    }
    return 0xff;
  }

  static uint8_t getOffset (uint8_t reg) {
    uint8_t off = ((reg & 0x80) == 0x80) ? sizeof(SwitchCtrlData) : 0;
    uint8_t result = SwitchCtrlData::getOffset(reg & ~0x80);
    if( result != 0xff ) {
      result += off;
    }
    return result;
  }

};

class SwitchCtrlList : public ChannelList {
public:
  SwitchCtrlList(uint16_t a) : ChannelList(a) {}
  virtual ~SwitchCtrlList() {}

  uint8_t ctDlyOn () const { return getByte(0,0x0f,0); }
  bool ctDlyOn (uint8_t value) const { return setByte(0,value,0x0f,0); }
  uint8_t ctDlyOff () const { return getByte(0,0xf0,4); }
  bool ctDlyOff (uint8_t value) const { return setByte(0,value,0xf0,4); }
  bool ctDlyOnOff (uint8_t onvalue, uint8_t offvalue) const { setByte(0,(onvalue & 0x0f) | (offvalue << 4)); return true; }

  uint8_t ctOn () const { return getByte(1,0x0f,0); }
  bool ctOn (uint8_t value) const { return setByte(1,value,0x0f,0); }
  uint8_t ctOff () const { return getByte(1,0xf0,4); }
  bool ctOff (uint8_t value) const { return setByte(1,value,0xf0,4); }
  bool ctOnOff (uint8_t onvalue, uint8_t offvalue) const { setByte(1,(onvalue & 0x0f) | (offvalue << 4)); return true; }

  uint8_t ctValLo() const { return getByte(2); }
  bool ctValLo(uint8_t value) const { return setByte(2,value); }
  uint8_t ctValHi() const { return getByte(3); }
  bool ctValHi(uint8_t value) const { return setByte(3,value); }
  uint8_t onDly() const { return getByte(4); }
  bool onDly(uint8_t value) const { return setByte(4,value); }
  uint8_t onTime() const { return getByte(5); }
  bool onTime(uint8_t value) const { return setByte(5,value); }
  uint8_t offDly() const { return getByte(6); }
  bool offDly(uint8_t value) const { return setByte(6,value); }
  uint8_t offTime() const { return getByte(7); }
  bool offTime(uint8_t value) const { return setByte(7,value); }

  uint8_t actionType() const { return getByte(8,0x0f,0); }
  bool actionType(uint8_t type) const { return setByte(8,type,0x0f,0); }
  bool multiExec() const { return isBitSet(19,0x20); }
  bool multiExec(bool s) const { return setBit(19,0x20,s); }
  bool offTimeMode() const { return isBitSet(8,0x40); }
  bool offTimeMode(bool s) const { return setBit(8,0x40,s); }
  bool onTimeMode() const { return isBitSet(8,0x80); }
  bool onTimeMode(bool s) const { return setBit(8,0x80,s); }

  uint8_t jtOn() const { return getByte(9,0x0f,0); }
  bool jtOn (uint8_t value) const { return setByte(9,value,0x0f,0); }
  uint8_t jtOff() const { return getByte(9,0xf0,4); }
  bool jtOff (uint8_t value) const { return setByte(9,value,0xf0,4); }
  uint8_t jtDlyOn() const { return getByte(10,0x0f,0); }
  bool jtDlyOn (uint8_t value) const { return setByte(10,value,0x0f,0); }
  uint8_t jtDlyOff() const { return getByte(10,0xf0,4); }
  bool jtDlyOff (uint8_t value) const { return setByte(10,value,0xf0,4); }

  virtual uint8_t getOffset (uint8_t reg) const { return SwitchCtrlData::getOffset(reg); }
};

class SwitchStateList : public ChannelList {
protected:
public:
  SwitchStateList(uint16_t a) : ChannelList(a) {}
  virtual ~SwitchStateList() {}

  const SwitchCtrlList sh() const { return SwitchCtrlList(address()); }
  const SwitchCtrlList lg() const { return SwitchCtrlList(address() + sizeof(SwitchCtrlData)); }

  virtual uint8_t getOffset (uint8_t reg) const { return SwitchStateData::getOffset(reg); }

  void defaults ();
  void odd ();
  void even ();
  void single ();

  static uint8_t size () {
    return sizeof(SwitchStateData);
  }
};

#endif
