//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2020-04-12 stan23 Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
// datasheet:
// https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf

#ifndef ACTORS_PCA9685_H_
#define ACTORS_PCA9685_H_

#include <Wire.h>


#define PCA9685_ADDRESS           0x40    /* default address if A5-A0 are open */

/* PCA9685 Registers */
#define PCA9685_REG_MODE1         0x00    /* Mode Register 1 */
#define PCA9685_REG_MODE2         0x01    /* Mode Register 2 */
#define PCA9685_REG_SUBADR1       0x02    /* I2C-bus subaddress 1 */
#define PCA9685_REG_SUBADR2       0x03    /* I2C-bus subaddress 2 */
#define PCA9685_REG_SUBADR3       0x04    /* I2C-bus subaddress 3 */
#define PCA9685_REG_ALLCALLADR    0x05    /* LED All Call I2C-bus address */
#define PCA9685_REG_LED0_ON_L     0x06    /* LED0 on tick, low byte*/
#define PCA9685_REG_LED0_ON_H     0x07    /* LED0 on tick, high byte*/
#define PCA9685_REG_LED0_OFF_L    0x08    /* LED0 off tick, low byte */
#define PCA9685_REG_LED0_OFF_H    0x09    /* LED0 off tick, high byte */
#define PCA9685_REG_LEDx_ON_L(x)  (PCA9685_REG_LED0_ON_L  + 4*x)
#define PCA9685_REG_LEDx_ON_H(x)  (PCA9685_REG_LED0_ON_H  + 4*x)
#define PCA9685_REG_LEDx_OFF_L(x) (PCA9685_REG_LED0_OFF_L + 4*x)
#define PCA9685_REG_LEDx_OFF_H(x) (PCA9685_REG_LED0_OFF_H + 4*x)
// etc all 16:  LED15_OFF_H 0x45
#define PCA9685_REG_ALLLED_ON_L   0xFA    /* load all the LEDn_ON registers, low */
#define PCA9685_REG_ALLLED_ON_H   0xFB    /* load all the LEDn_ON registers, high */
#define PCA9685_REG_ALLLED_OFF_L  0xFC    /* load all the LEDn_OFF registers, low */
#define PCA9685_REG_ALLLED_OFF_H  0xFD    /* load all the LEDn_OFF registers,high */
#define PCA9685_REG_PRESCALE      0xFE    /* Prescaler for PWM output frequency */
#define PCA9685_REG_TESTMODE      0xFF    /* defines the test mode to be entered */

/* PCA9685 MODE1 register bits */
#define PCA9685_MODE1_ALLCAL      0x01    /* respond to LED All Call I2C-bus address */
#define PCA9685_MODE1_SUB3        0x02    /* respond to I2C-bus subaddress 3 */
#define PCA9685_MODE1_SUB2        0x04    /* respond to I2C-bus subaddress 2 */
#define PCA9685_MODE1_SUB1        0x08    /* respond to I2C-bus subaddress 1 */
#define PCA9685_MODE1_SLEEP       0x10    /* Low power mode. Oscillator off */
#define PCA9685_MODE1_AI          0x20    /* Auto-Increment enabled */
#define PCA9685_MODE1_EXTCLK      0x40    /* Use EXTCLK pin clock */
#define PCA9685_MODE1_RESTART     0x80    /* Restart enabled */
/* PCA9685 MODE2 register bits */
#define PCA9685_MODE2_OUTNE_0     0x01    /* Active LOW output enable input */
#define PCA9685_MODE2_OUTNE_1     0x02    /* Active LOW output enable input - high impedience */
#define PCA9685_MODE2_OUTDRV      0x04    /* totem pole structure vs open-drain */
#define PCA9685_MODE2_OCH         0x08    /* Outputs change on ACK vs STOP */
#define PCA9685_MODE2_INVRT       0x10    /* Output logic state inverted */
/* PCA9685 LEDx_ON_H register bits */
#define PCA9685_LEDx_FULL_ON      0x10    /* LEDx full ON */
/* PCA9685 LEDx_OFF_H register bits */
#define PCA9685_LEDx_FULL_OFF     0x10    /* LEDx full OFF */


namespace as {

template <uint8_t ADDRESS=PCA9685_ADDRESS>
class PCA9685 {
  private:
    static void writeRegister (uint8_t reg, uint8_t val) {
      Wire.beginTransmission(ADDRESS);
      Wire.write(reg);
      Wire.write(val);
      Wire.endTransmission();
    }
    static uint8_t readRegister (uint8_t reg) {
      Wire.beginTransmission(ADDRESS);
      Wire.write(reg);
      Wire.endTransmission();
      Wire.requestFrom(ADDRESS, 1);

      int ret = -1;
      if(Wire.available() <=1) {
        ret = Wire.read();
      }
      return ret;
    }

  public:

    void init () {
      Wire.begin();
    }

    void set (uint8_t channel, uint16_t value) {
      /* LEDx_ON contains the timeslot where LED is turned ON */
      /* LEDx_OFF contains the timeslot where LED is turned OFF */
      /* timeslots count from 0 to 4095 */
      if (value == 0) {
        writeRegister (PCA9685_REG_LEDx_OFF_H(channel), PCA9685_LEDx_FULL_OFF);
        //writeRegister (PCA9685_REG_LEDx_OFF_L(channel), 0);
        //writeRegister (PCA9685_REG_LEDx_ON_H(channel), 0);
        //writeRegister (PCA9685_REG_LEDx_ON_L(channel), 0);
      } else if (value == 0x0FFF) {
        writeRegister (PCA9685_REG_LEDx_ON_H(channel), PCA9685_LEDx_FULL_ON);
        //writeRegister (PCA9685_REG_LEDx_ON_L(channel), 0);
        writeRegister (PCA9685_REG_LEDx_OFF_H(channel), 0);
        //writeRegister (PCA9685_REG_LEDx_OFF_L(channel), 0);
      } else {
        /* no delay, turn LED on in the first timeslot */
        writeRegister (PCA9685_REG_LEDx_ON_H(channel), 0);
        writeRegister (PCA9685_REG_LEDx_ON_L(channel), 0);

        writeRegister (PCA9685_REG_LEDx_OFF_H(channel), (value >> 8) & 0x0F);
        writeRegister (PCA9685_REG_LEDx_OFF_L(channel), value & 0xFF);
      }
    }

    /* send reset command */
    void reset () {
      writeRegister (PCA9685_REG_MODE1, PCA9685_MODE1_RESTART);
      delay(10);
    }
};


}



#endif /* ACTORS_PCA9685_H_ */
