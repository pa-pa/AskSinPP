#!/usr/bin/env bash
source _functions.sh

# Libs included in Lib-Manager
LIBS=(
  "OneWire"
)

# Libs from Git-Repositories
GIT_LIBS=(
  "https://github.com/GreyGnome/EnableInterrupt"
  "https://github.com/rocketscream/Low-Power.git"
  "https://github.com/adafruit/Adafruit_Sensor"
  "https://github.com/adafruit/DHT-sensor-library"
  "https://github.com/spease/Sensirion.git"
  "https://github.com/adafruit/TSL2561-Arduino-Library"
  "https://github.com/adafruit/Adafruit-BMP085-Library"
  "https://github.com/finitespace/BME280.git"
  "https://github.com/claws/BH1750"
  "https://github.com/xoseperez/hlw8012.git"
  "https://github.com/bogde/HX711.git"
  "https://github.com/ZinggJM/GxEPD.git"
  "https://github.com/ZinggJM/GxEPD2.git"
  "https://github.com/adafruit/Adafruit-GFX-Library.git"
  "https://github.com/adafruit/Adafruit_BusIO.git"
  "https://github.com/adafruit/Adafruit_SHT31.git"
  "https://github.com/olikraus/U8g2_for_Adafruit_GFX.git"
  "https://github.com/FastLED/FastLED.git"
  "https://github.com/PaulStoffregen/SoftwareSerial.git"
  "https://github.com/DFRobot/DFRobotDFPlayerMini.git"
  "https://github.com/miguelbalboa/rfid.git"
  "https://github.com/jayjayuk/Si7021-Humidity-And-Temperature-Sensor-Library"
  "https://github.com/adafruit/Adafruit-ST7735-Library"
  "https://github.com/milesburton/Arduino-Temperature-Control-Library"
  "https://github.com/pololu/vl53l0x-arduino"
  "https://github.com/lewapek/sds-dust-sensors-arduino-library"
  "https://github.com/adafruit/Adafruit_ADS1X15"
  "https://github.com/marcoschwartz/LiquidCrystal_I2C.git"
  "https://github.com/olikraus/U8g2_Arduino.git"
  "https://github.com/laurb9/StepperDriver.git"
  "https://github.com/jp112sdl/INA219_WE"
  "https://github.com/kitesurfer1404/WS2812FX"
  "https://github.com/PaulStoffregen/Time"
  "https://github.com/adafruit/Adafruit_NeoPixel"
  "https://github.com/sparkfun/BMP180_Breakout_Arduino_Library"
  "https://github.com/khoih-prog/RPI_PICO_TimerInterrupt"
)

# Install Libs
arduino-cli lib install ${LIBS[*]}

# Install Libs from GIT
for REPO_URL in ${GIT_LIBS[*]}; do
  REPO="$(basename $REPO_URL | cut -d. -f1)"
  echo "Install $REPO from Git"
  ( arduino-cli lib install --git-url ${REPO_URL} 1>/dev/null )&
done
wait

# Symlink AskSinPP lib
echo Symlinking AskSinPP library
ln -sf $BASEDIR $(realpath $DIR/Arduino/libraries/AskSinPP)

