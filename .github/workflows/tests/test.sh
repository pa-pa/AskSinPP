#!/usr/bin/env bash
set -x
source _functions.sh
uname -a

# Directories to search in for sketches
SKETCH_PATHES=(
  $BASEDIR/examples
  sketches
)

# First argument specifies the board
BOARD=$1

if [ -z "$BOARD" ] ; then
  >&2 echo "ERROR: Board argument missing"
  exit 1
fi

# Map for board-shorthand to specify CORE and full-qualifying-board-name
case "$BOARD" in
  328p)
    # arduino:avr is the default core and gets always installed
    CORE=""
    FQBN=arduino:avr:pro:cpu=8MHzatmega328
    ;;
  168p)
    # arduino:avr is the default core and gets always installed
    CORE=""
    FQBN=arduino:avr:pro:cpu=8MHzatmega168
    BUILD_PROPERTY="upload.maximum_size=16384"
    ;;
  mega128)
    CORE=MegaCore:avr
    FQBN=MegaCore:avr:128:BOD=disabled,LTO=Os_flto,clock=16MHz_external
    ;;
  644p)
    CORE=MightyCore:avr
    FQBN=MightyCore:avr:644:pinout=bobuino,variant=modelP,LTO=Os_flto,clock=8MHz_internal
    ;;
  1284pBOB)
    CORE=MightyCore:avr
    FQBN=MightyCore:avr:1284:pinout=bobuino,variant=modelP,LTO=Os_flto,clock=8MHz_internal
    ;;
  1284pSTD)
    CORE=MightyCore:avr
    FQBN=MightyCore:avr:1284:pinout=standard,variant=modelP,LTO=Os_flto,clock=8MHz_internal
    ;;
  bluepill)
    CORE=stm32duino:STM32F1
    FQBN=stm32duino:STM32F1:genericSTM32F103C
    ;;
  maplemini)
    CORE=stm32duino:STM32F1
    FQBN=stm32duino:STM32F1:mapleMini
    ;;
  esp32)
    CORE=esp32:esp32
    FQBN=esp32:esp32:esp32
    ;;
  rp2040)
    CORE=rp2040:rp2040
    FQBN=rp2040:rp2040:rpipico:flash=2097152_65536,freq=100,opt=Small,rtti=Disabled,dbgport=Serial,dbglvl=None,usbstack=picosdk
    ;;
  efm32)
    CORE=jp112sdl:EFM32
    FQBN=jp112sdl:EFM32:EFM32G200:cpu=64,clock_source=hfrc28,serial=serial1
    ;;
  *)
    >&2 echo "ERROR: Unknown board \"${BOARD}\""
    exit 1
esac

# Find sketches
findSketches ${BOARD}

# Install cores
echo Install arduino:avr core
arduino-cli core install arduino:avr
if [ -n "${CORE}" ] ; then
  echo Install ${CORE} core
  arduino-cli core install ${CORE}
fi

# Run tests without AES
RES1=0
if [ ${#SKETCHES[@]} -gt 0 ]; then
  runTests "${FQBN}" "${SKETCHES[@]}"
  RES1=$?
  echo "::warning ::warning Compiled ${#SKETCHES[@]} Sketches for ${BOARD}. Average space consumption ${AVG_BYTES} Bytes"
fi

# Return error
if [ $RES1 -gt 0 ] ; then
  >&2 echo "Errors occurred!"
  exit 1
fi

