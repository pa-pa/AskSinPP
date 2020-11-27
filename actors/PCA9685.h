//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-11-01 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2020-11-17 trilu2000 Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __PCA9685PWM_H__
#define __PCA9685PWM_H__

#include <Arduino.h>
#include <Wire.h>
#include <PCA9685.h>

// REGISTER ADDRESSES
#define PCA9685_MODE1          0x00 
#define PCA9685_MODE2          0x01 
#define PCA9685_LED0_ON_L_REG  0x06 
#define PCA9685_LED0_ON_H_REG  0x07 
#define PCA9685_LED0_OFF_L_REG 0x08 
#define PCA9685_LED0_OFF_H_REG 0x09

// MODE1 REGISTERS
#define PCA9685_RESTART        0x80
#define PCA9685_EXTCLK         0x40
#define PCA9685_AUTOINCR       0x20
#define PCA9685_SLEEP          0x10
#define PCA9685_SUB1           0x08
#define PCA9685_SUB2           0x04
#define PCA9685_SUB3           0x02
#define PCA9685_ALLCALL        0x01

// MODE2 REGISTERS
#define PCA9685_INVERT         0x10
#define PCA9685_OCH            0x08
#define PCA9685_OUTDRV         0x04
#define PCA9685_OUTNE          0x03

#define PCA9685_MODE1_RESTART 0x80  
#define GENERAL_CALL  0x00
#define PCA9685_SWRST 0b110

namespace as {

  // reduced 12 bit PWM table, holds only every second value
  static const uint16_t pwm_12bit_table[101] PROGMEM = {
    0, 2, 18, 28, 37, 47, 58, 68, 79, 90, 101, 113, 125, 137, 149, 162, 175, 189,203, 217, 231, 246, 262,
    277, 293, 310, 327, 344, 362, 381, 399, 419, 438, 459, 479, 501, 523, 545, 568, 592, 616, 641, 666,
    693, 719, 747, 775, 804, 834, 864, 896, 928, 961, 994, 1029, 1064, 1101, 1138, 1177, 1216, 1256, 
    1298, 1340, 1384, 1428, 1474, 1521, 1569, 1619, 1669, 1721, 1775, 1830, 1886, 1943, 2002, 2063, 2125, 
    2189, 2254, 2322, 2390, 2461, 2534, 2608, 2684, 2762, 2843, 2925, 3009, 3096, 3185, 3276, 3369, 3465, 
    3563, 3664, 3768, 3874, 3983, 4095
  };


  static uint8_t _init_done = 0;

  template<uint8_t STEPS = 200, bool LINEAR = false, bool INVERSE = false, byte I2C_ADDRESS = 0x40>
  class PCA9685PWM {
    uint8_t _channel;

    uint8_t read_register(uint8_t address) {												// read register byte
      Wire.beginTransmission(I2C_ADDRESS);
      Wire.write(address);
      Wire.endTransmission(false);
      Wire.requestFrom(I2C_ADDRESS, (uint8_t)1);
      if (Wire.available() < 1)
        return 0;
      return (Wire.read());
    }
    void write_register(uint8_t address, uint8_t value) {								// writes a byte to a register
      Wire.beginTransmission(I2C_ADDRESS);
      Wire.write((uint8_t)address);
      Wire.write((uint8_t)(value));
      Wire.endTransmission();
    }
    void reset() {
      Wire.beginTransmission(GENERAL_CALL);
      Wire.write(PCA9685_SWRST);
      Wire.endTransmission();
      delay(10);
    }
    void set_channel_value(uint8_t channel, uint16_t value) {
      //DPRINT(F("set channel: ")); DPRINT(channel); DPRINT(F(" to ")); DPRINTLN(value);
      write_register(PCA9685_LED0_ON_L_REG + channel * 4, 0x00);
      write_register(PCA9685_LED0_ON_H_REG + channel * 4, 0x00);
      write_register(PCA9685_LED0_OFF_L_REG + channel * 4, (value & 0xFF));
      write_register(PCA9685_LED0_OFF_H_REG + channel * 4, (value >> 8));
    }

  public:
    PCA9685PWM() : _channel(0) {}
    ~PCA9685PWM() {}

    void init(uint8_t channel) {
      _channel = channel;
      if (_init_done == 0) {

        Wire.begin();
        reset();

        // set default values
        write_register(PCA9685_MODE1, PCA9685_ALLCALL | PCA9685_AUTOINCR);
        write_register(PCA9685_MODE2, PCA9685_OUTDRV);

        // check if the pca9685 is there
        uint8_t r = read_register(PCA9685_MODE1);
        DPRINT(F("PCA9685 "));
        if (r == 0x21) {
          _init_done = 2;
          DPRINT(F("found at 0x")); DHEXLN(I2C_ADDRESS);
        }
        else {
          _init_done = 1;
          DPRINTLN(F(" not found"));
        }
      }
    }

    void set(uint8_t value) {
      uint16_t pwm = 0;
      if (LINEAR) {
        // https://www.arduino.cc/reference/en/language/functions/math/map/
        pwm = map(value, 0, STEPS, 0, 4096);
      }
      else {
        // the table holds only every second value to save space, therefore 
        // we have to calculate the odd value as average of the word above and below 
        pwm = pgm_read_word(&pwm_12bit_table[value / 2]);

        if (value & 1) { //odd
          uint16_t x_high = pgm_read_word(&pwm_12bit_table[(value / 2) + 1]);
          pwm = (x_high + pwm) / 2;
        }
      }
      if (INVERSE) pwm = ~pwm ^ 0xF000;
      //DPRINT(F("PWM value ")); DPRINT(value); DPRINT(F(", pwm ")); DPRINTLN(pwm);
      set_channel_value(_channel, pwm);
    }
  };


}

#endif
