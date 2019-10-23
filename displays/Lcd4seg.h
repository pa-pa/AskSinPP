//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// LCD4SEG class
// 2019-10-20 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

//designed for 4 digit lcd from
//https://de.aliexpress.com/item/1000002117470.html

#include <AlarmClock.h>
#ifndef __LCD4SEGHT1621_H__
#define __LCD4SEGHT1621_H__

#define LCD4SEG_BUFSIZE 4

//============DEFINE CONTROL CODES=============
#define  BIAS     0x52             //0b1000 0101 0010
#define  SYSDIS   0X00             //0b1000 0000 0000
#define  SYSEN    0X02             //0b1000 0000 0010
#define  LCDOFF   0X04             //0b1000 0000 0100
#define  LCDON    0X06             //0b1000 0000 0110
#define  XTAL     0x28             //0b1000 0010 1000
#define  RC256    0X30             //0b1000 0011 0000
#define  TONEON   0X12             //0b1000 0001 0010
#define  TONEOFF  0X10             //0b1000 0001 0000
#define  WDTDIS1  0X0A             //0b1000 0000 1010

namespace as {

template<uint8_t CS, uint8_t WR, uint8_t DT>
class LCD4SEG {
private:
  uint8_t _cs, _wr, _data, _bufferMem;
  char _localBuffer[LCD4SEG_BUFSIZE]; //store the mapping segment number
  uint8_t _lcdbuffer[LCD4SEG_BUFSIZE]; //store the data that will use to send to LCD RAM

  void sendCmd(byte command) {
    digitalWrite(_cs, LOW);
    uint16_t cmd = 0x800;
    cmd = cmd | command;

    for (uint8_t i = 0; i <12; i ++){
      digitalWrite(_wr, LOW);
      digitalWrite(_data, (cmd & 0x800) ? HIGH : LOW);
      cmd <<=1;
      digitalWrite(_wr, HIGH);
    }
    digitalWrite(_cs, HIGH);
  }

  void update(uint8_t type) {
    memset(_lcdbuffer, 0, LCD4SEG_BUFSIZE);

    //maping number to lcd segment.
    for (int i = 0 ; i < LCD4SEG_BUFSIZE; i ++){
      switch(_localBuffer[i]){
      /* segments bitorder
      *       a
      *       -
      *    f | | b
      *       -  <-g
      *    e | | c
      *       - . <-h
      *       d
      *
      *   a b c d e f g h
      */

        case '0':
          _lcdbuffer[i] = 0b11111100;
        break;
        case '1':
          _lcdbuffer[i] = 0b01100000;
        break;
        case '2':
          _lcdbuffer[i] = 0b11011010;
        break;
        case '3':
          _lcdbuffer[i] = 0b11110010;
        break;
        case '4':
          _lcdbuffer[i] = 0b01100110;
        break;
        case '5':
          _lcdbuffer[i] = 0b10110110;
        break;
        case '6':
          _lcdbuffer[i] = 0b10111110;
        break;
        case '7':
          _lcdbuffer[i] = 0b11100000;
        break;
        case '8':
          _lcdbuffer[i] = 0b11111110;
        break;
        case '9':
          _lcdbuffer[i] = 0b11110110;
        break;
        case '-':
          _lcdbuffer[i] = 0b00000010;
        break;
        case ' ':
          _lcdbuffer[i] = 0b00000000;
        break;
      }
    }

    switch (type){
      case 1: //for _h
        _lcdbuffer[2] = 0b00001010; // r
        _lcdbuffer[3] = 0b01101110; // H
      break;

      case 2: //for temp C format
        _lcdbuffer[1]|= 0b00000001; //dot
        _lcdbuffer[3] = 0b10011101; //print *C
      break;

      case 3: //print b.Lo
        _lcdbuffer[0] = 0b00111111; // b.
        _lcdbuffer[1] = 0b00011100; // L
        _lcdbuffer[2] = 0b00111010; // o
        _lcdbuffer[3] = 0b00000000; //
      break;
    }

    //sending 101 to inform HT1621 that MCU send data then follow by address 000000.
    uint16_t cmd =0x140;
    digitalWrite(_cs, LOW);
    for(int i = 0 ;i < 9;i ++){
      digitalWrite(_wr, LOW);
      digitalWrite(_data, (cmd & 0x100) ? HIGH : LOW);
      cmd <<=1;
      digitalWrite(_wr, HIGH);
    }

    //sending 32 bit of segment to LCD RAM.
    for (int i =0; i < 8; i++){
      for (int j =0; j<4;j++){
        digitalWrite(_wr,LOW);
        _bufferMem =_lcdbuffer[j]<<i;
        digitalWrite(_data, (_bufferMem & 0x80) ? HIGH : LOW);
        digitalWrite(_wr, HIGH);
      }
    }
    digitalWrite(_cs, HIGH);
  }
  
public:
  LCD4SEG () : _cs(0), _wr(0), _data(0),_bufferMem(0) {}
  virtual ~LCD4SEG() {}

  void init(){
    _cs = CS;
    _wr = WR;
    _data = DT;
    pinMode(_cs, OUTPUT);
    pinMode(_wr, OUTPUT);
    pinMode(_data, OUTPUT);

    sendCmd(BIAS);
    sendCmd(SYSDIS);
    sendCmd(SYSEN);
    sendCmd(RC256);
    sendCmd(WDTDIS1);
    sendCmd(LCDON);
  }

  void clear(){
    uint16_t cmd = 0x140;

    digitalWrite(_cs, LOW);
    for (uint8_t i =0; i<9; i++){
      digitalWrite(_wr, LOW);
      digitalWrite(_data, (cmd & 0x100) ? HIGH : LOW);
      cmd <<=1;
      digitalWrite(_wr, HIGH);
    }

    for (uint8_t i =0 ; i<32; i++){
      digitalWrite(_wr, LOW);
      digitalWrite(_data, LOW);
      digitalWrite(_wr, HIGH);
    }
    digitalWrite(_cs, HIGH);
  }
 
  void printH(uint8_t number){
    if (number > 99){ number = 99; }
    snprintf(_localBuffer, 3, "%2i", number);
    update(1);
  }

  void printC(int16_t number){
    snprintf(_localBuffer, 4, "%3i", number);
    if (number > -10) {
      if (number < 0)  _localBuffer[0] = '-';
      if (number < 10) _localBuffer[1] = '0';
    }
    update(2);
  }

  void printLowBat() {
    update(3);
  }
};

template<class LCDTYPE>
class LCDToggleTH : public Alarm {
public:
    typedef LCDTYPE LcdType;
    LcdType lcd;
private:
    uint8_t         _screenNum;
    int16_t         _t;
    uint8_t         _h;
    uint8_t         _toggleTimeSecs;
    bool            _lowBat;
public:
  LCDToggleTH () :  Alarm(0), _screenNum(0), _t(0), _h(0), _toggleTimeSecs(10), _lowBat(false) { async(true); }
  virtual ~LCDToggleTH () {}

  void init() {
    lcd.init();
    lcd.clear();
    sysclock.init();
    sysclock.add(*this);
  }

  void setValues(int16_t t, uint8_t h, bool b) {
    _t = t;
    _h = h;
    _lowBat = b;
    displayValues();
  }

  void setToggleTime(uint8_t time) {
    _toggleTimeSecs = time;
    sysclock.cancel(*this);
    set(seconds2ticks(_toggleTimeSecs));
    sysclock.add(*this);
  }

  void displayValues() {
    switch (_screenNum) {
      case 0:
        lcd.printH(_h);
        break;
      case 1:
        lcd.printC(_t);
        break;
      case 2:
        lcd.printLowBat();
        break;
    }
    _screenNum++;
    if (_screenNum > (_lowBat ?  2 : 1)) _screenNum = 0;
  }

  virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
    set(seconds2ticks(_toggleTimeSecs));
    displayValues();
    sysclock.add(*this);
  }
};

}

/* example:
#include <displays/Lcd4seg.h>

using namespace as;

LCDToggleTH<LCD4SEG<6, 3, 7>> lcd;

void setup(){
  lcd.init();
  lcd.setToggleTime(3);          //toggle display every 3 seconds
  lcd.setValues(256, 38, false); //25.6°C, 38rH, no LowBat
}

void loop(){}

 */
#endif

