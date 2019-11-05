//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// LCD6749 class
// 2019-10-20 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// https://www.pollin.de/p/lcd-modul-densitron-pc-6749-aaw-120818
// code based on https://www.mikrocontroller.net/topic/380929#4335859

#ifndef __LCD6749_H__
#define __LCD6749_H__

#define LCD6749_BUFSIZE 7

namespace as {

  const struct seg16fonts {
    char c;
    uint16_t s;
  } seg16font[65] PROGMEM  = {
        /* ABCDEFGHIJKLMNOP */
 #ifdef LCDREVERSED
  { '1', 0b0000011000000100 },
  { '2', 0b1101110000111000 },
  { '3', 0b1001111000111000 },
  { '4', 0b0010011000111000 },
  { '5', 0b1011101000111000 },
  { '6', 0b1111101000111000 },
  { '7', 0b0001100001111100 },
  { '8', 0b1111111000111000 },
  { '9', 0b1011111000111000 },
  { '0', 0b1111111001010100 },
  { '+', 0b0000000010111010 },
  { '-', 0b0000000000111000 },
  { '*', 0b0000000111111111 },
  { '/', 0b0000000001010100 },
  { '.', 0b0000100000000000 },    // no
  { ')', 0b0000000001000001 },
  { '$', 0b1011101010111010 },
  { '(', 0b0000000100000100 },
  { 'A', 0b0111111000111000 },
  { 'B', 0b1111100100011100 },
  { 'C', 0b1111100000000000 },
  { 'D', 0b1111111000000000 },  // bad = O
  { 'E', 0b1111100000011000 },
  { 'F', 0b0111100000011000 },
  { 'G', 0b1111101000100000 },
  { 'H', 0b0110011000111000 },
  { 'I', 0b1001100010010010 },
  { 'J', 0b1000111000000000 },
  { 'K', 0b0110000100011100 },
  { 'L', 0b1110000000000000 },
  { 'M', 0b0110011000010101 },
  { 'N', 0b0110011100010001 },
  { 'O', 0b1111111000000000 },
  { 'P', 0b0111110000111000 },
  { 'Q', 0b1111111100000000 },
  { 'R', 0b0111000100001100 },
  { 'S', 0b1011101000111000 },
  { 'T', 0b0001100010010010 },
  { 'U', 0b1110011000000000 },
  { 'V', 0b0110100001010100 },
  { 'W', 0b0110011101010000 },
  { 'X', 0b0000000101010101 },
  { 'Y', 0b0000000010010101 },
  { 'Z', 0b1001100001010100 },
  { '}', 0b0000000001011001 },
  { '_', 0b1000000000000000 },
  { '{', 0b0000000100110100 },
  { 'b', 0b1110001000111000 },
  { 'c', 0b1100000000111000 },
  { 'd', 0b1100011000111000 },
  { 'h', 0b0110001000111000 },
  { 'l', 0b0110000000000000 },
  { 'm', 0b0100001010111000 },
  { 'n', 0b0100001000111000 },
  { 'o', 0b1100001000111000 },
  { 'r', 0b0100000000011000 },
  { 't', 0b1110000000011000 },
  { 'u', 0b1100001000000000 },
  { 'v', 0b0100000001000000 },
  { 'w', 0b1100001010000000 },
  { '[', 0b1111100000000000 },
  { '|', 0b0000000010010010 },
  { ']', 0b1001111000000000 },
#else
  { '1', 0b0110000001000000 },
  { '2', 0b1101110000111000 },
  { '3', 0b1111100000111000 },
  { '4', 0b0000001010111010 },
  { '5', 0b1011101000111000 },
  { '6', 0b1011111000111000 },
  { '7', 0b1000000001111010 },
  { '8', 0b1111111000111000 },
  { '9', 0b1111101000111000 },
  { '0', 0b1111111001010100 },
  { '+', 0b0000000010111010 },
  { '-', 0b0000000000111000 },
  { '*', 0b0000000111111111 },
  { '/', 0b0000000001010100 },
  { '.', 0b0000100000000000 },
  { '(', 0b0000000001000001 },
  { '$', 0b1011101010111010 },
  { ')', 0b0000000100000100 },
  { 'A', 0b1110011000111000 },
  { 'B', 0b1111100010011010 },
  { 'C', 0b1001111000000000 },
  { 'D', 0b1111100010010010 },
  { 'E', 0b1001111000110000 },
  { 'F', 0b1000011000110000 },
  { 'G', 0b1011111000001000 },
  { 'H', 0b0110011000111000 },
  { 'I', 0b1001100010010010 },
  { 'J', 0b1000100010010010 },
  { 'K', 0b0000000011010011 },
  { 'L', 0b0001111000000000 },
  { 'M', 0b0110011101010000 },
  { 'N', 0b0110011100010001 },
  { 'O', 0b1111111000000000 },
  { 'P', 0b1100011000111000 },
  { 'Q', 0b1111111000000001 },
  { 'R', 0b1100011000110001 },
  { 'S', 0b1011101000111000 },
  { 'T', 0b1000000010010010 },
  { 'U', 0b0111111000000000 },
  { 'V', 0b0000011001010100 },
  { 'W', 0b0110011000010101 },
  { 'X', 0b0000000101010101 },
  { 'Y', 0b0000000101010010 },
  { 'Z', 0b1001100001010100 },
  { '{', 0b0000000001011001 },
  { '_', 0b0001100000000000 },
  { '}', 0b0000000100110100 },
  { 'b', 0b0011111000111000 },
  { 'c', 0b0001110000111000 },
  { 'd', 0b0111110000111000 },
  { 'h', 0b0010011000111000 },
  { 'l', 0b0000111000000000 },
  { 'm', 0b0010010000101010 },
  { 'n', 0b0010010000111000 },
  { 'o', 0b0011110000111000 },
  { 'r', 0b0000010000110000 },
  { 't', 0b0001000010111010 },
  { 'u', 0b0011110000000000 },
  { 'v', 0b0000010000000100 },
  { 'w', 0b0010010000000101 },
  { '[', 0b1001111000000000 },
  { '|', 0b0000000010010010 },
  { ']', 0b1111100000000000 },
#endif
  { ' ', 0b0000000000000000 },
  { 0, 0xFFFF }
  };

  const uint8_t segmap[3][16] PROGMEM= {
#ifdef LCDREVERSED
    { 0x60, 0x62, 0x63, 0x53, 0x47, 0x46, 0x44, 0x50,
      0x54, 0x61, 0x45, 0x51, 0x55, 0x52, 0x56, 0x57 },
    { 0x40, 0x42, 0x43, 0x33, 0x27, 0x26, 0x24, 0x30,
      0x34, 0x41, 0x25, 0x31, 0x35, 0x32, 0x36, 0x37 },
    { 0x14, 0x16, 0x17, 0x07, 0x03, 0x02, 0x00, 0x04,
      0x10, 0x15, 0x01, 0x05, 0x11, 0x06, 0x12, 0x13 }
#else
    { 0x14, 0x16, 0x17, 0x07, 0x03, 0x02, 0x00, 0x04,
      0x10, 0x15, 0x01, 0x05, 0x11, 0x06, 0x12, 0x13 },
    { 0x40, 0x42, 0x43, 0x33, 0x27, 0x26, 0x24, 0x30,
      0x34, 0x41, 0x25, 0x31, 0x35, 0x32, 0x36, 0x37 },
    { 0x60, 0x62, 0x63, 0x53, 0x47, 0x46, 0x44, 0x50,
      0x54, 0x61, 0x45, 0x51, 0x55, 0x52, 0x56, 0x57 }
#endif
  };

template<uint8_t CS, uint8_t WR, uint8_t DT>
class LCD6749 {

public:
  enum dots { NO_DOT, SINGLE_DOT, DOUBLE_DOT, BOTH_DOTS };

private:
  uint8_t _cs, _wr, _data, _lcdbuffer[LCD6749_BUFSIZE];

  void CS_low()          { digitalWrite(_cs, LOW); delayMicroseconds(1); }
  void CS_high()         { delayMicroseconds(1);   digitalWrite(_cs, HIGH); }
  void DATA(uint8_t x)   { if ((x)==0) digitalWrite(_data, LOW); else digitalWrite(_data, HIGH); }
  void WR_pulse()        { digitalWrite(_wr, LOW); delayMicroseconds(3); digitalWrite(_wr, HIGH); delayMicroseconds(3); }
  void clearBuffer(void) { memset(_lcdbuffer, 0, LCD6749_BUFSIZE); }


  uint16_t char2seg(char c) {
    for (uint8_t j=0; pgm_read_byte(&(seg16font[j]).c); j++) {
      if (pgm_read_byte(&(seg16font[j]).c) == c) {
        return (pgm_read_word(&(seg16font[j]).s));
      }
    }
    return 0;
  }

  void updateDisplay(uint8_t dot) {
    CS_low();
    DATA(1); WR_pulse();
    DATA(0); WR_pulse();
    DATA(1); WR_pulse();

    for (uint8_t i = 0; i < 6; i++) { DATA(0); WR_pulse(); }

#ifdef LCDREVERSED
    if (dot & 0x01) _lcdbuffer[2]|=0b00000010;
#else
    if (dot & 0x01) _lcdbuffer[2]|=0b00001000;
#endif
    if (dot & 0x02) _lcdbuffer[2]|=0b00000110;
    for (uint8_t i=0; i<LCD6749_BUFSIZE; i++) {
      uint8_t mask= 0x01;
      while (mask) {
        DATA(_lcdbuffer[i] & mask);
        WR_pulse();
        mask<<=1;
      }
    }
    DATA(1);
    CS_high();
  }

  void updateBuffer(uint8_t pos, char c) {
    uint8_t segno= 0;
    uint16_t mask= 0x8000;
    for ( ; segno<16; segno++, mask>>=1) {
      if (char2seg(c) & mask) {
        uint8_t code= pgm_read_byte(&(segmap[pos][segno]));
        _lcdbuffer[code>>4] |= 1<<(code&7);
      }
    }
  }

  void sendCmd(uint8_t cmd) {
    CS_low();
    DATA(1); WR_pulse();
    DATA(0); WR_pulse();
    DATA(0); WR_pulse();
    uint8_t mask= 0x80;
    while (mask) {
      DATA(cmd&mask);
      WR_pulse();
      mask>>=1;
    }
    DATA(1); WR_pulse();
    CS_high();
  }

public:
  LCD6749 () : _cs(0), _wr(0), _data(0) {}
  virtual ~LCD6749() {}

  void init() {
    _cs = CS;
    _wr = WR;
    _data= DT;
    pinMode(_cs,   OUTPUT);
    pinMode(_wr,   OUTPUT);
    pinMode(_data, OUTPUT);

    sendCmd(0b00000001); // system enable
    sendCmd(0b00101010); // 1/2 Bias, 4 commons
    sendCmd(0b00000011); // turn on LCD

    clearBuffer();
  }

  void clear() {
    clearBuffer();
    updateDisplay(NO_DOT);
  }

  void printChar(uint8_t pos, char c, uint8_t dot) {
    updateBuffer(pos, c);
    updateDisplay(dot);
  }

  void printV(uint8_t value) {
    printWithUnit(value, SINGLE_DOT, 'V');
  }

  void printWithUnit(uint8_t value, uint8_t dot, char c) {
    clearBuffer();
    char localBuffer[3];
    snprintf(localBuffer, 3, "%2i", value);
    updateBuffer(0, value < 10 ? '0' : localBuffer[0]);
    updateBuffer(1,localBuffer[1]);
    updateBuffer(2,c);
    updateDisplay(dot);
  }

  void printText(const char * t, uint8_t dot=NO_DOT) {
    clearBuffer();

#ifdef LCDREVERSED
    if (strlen(t) > 2) {
      if ( t[2]=='.' ) dot = SINGLE_DOT;
      if ( t[2]==':' ) dot = DOUBLE_DOT;
    }
#else
    if (strlen(t) > 1) {
      if ( t[1]=='.' ) dot = SINGLE_DOT;
      if ( t[1]==':' ) dot = DOUBLE_DOT;
    }
#endif

    uint8_t j = 0;
    for (uint8_t i = 0; i < min(strlen(t), dot > NO_DOT ? 4 : 3); i++) {
#ifdef LCDREVERSED
      if (dot > NO_DOT && i == 2) i++;
#else
      if (dot > NO_DOT && i == 1) i++;
#endif
      updateBuffer(j, t[i]);
      j++;
    }

    updateDisplay(dot);
  }

  void printNumberAsFloat(uint16_t number) {
    if (number > 999) {
      number = (number + 5) / 10;
      printNumber(number, true, NO_DOT);
    } else {
      printNumber(number, true, SINGLE_DOT);
    }
  }

  void printNumber(int16_t number, bool leadingZero) {
    printNumber(number, leadingZero, NO_DOT);
  }

  void printNumber(int16_t number, bool leadingZero, uint8_t dot) {
    clearBuffer();
    char localBuffer[4]={};
    snprintf(localBuffer, 4, "%3i", number);
    for (uint8_t i = 0; i < strlen(localBuffer); i++) {
      if (localBuffer[i] == 32 && leadingZero == true) localBuffer[i] = '0';
      updateBuffer(i, localBuffer[i]);
    }
    updateDisplay(dot);
  }
};

}

/*
  //printWithUnit(24, LCD6749::NO_DOT, 'G');     // 2 4 G
  //printWithUnit(24, LCD6749::SINGLE_DOT, 'G'); // 2.4 G
  //printText("4:15");                           // 4:1 5
  //printNumberAsFloat(720);                     // 7.2 0
  //printNumberAsFloat(4);                       // 0.0 4
  //printV(19);                                  // 1.9 V
  //printNumber(14);                             //   1 4
  //printNumber(14, true); //with leading zero   // 0 1 4
*/

#endif
