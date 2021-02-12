#!/usr/bin/env bash
source _functions.sh
uname -a

# Directories to search in for sketches
SKETCH_PATHES=(
  $BASEDIR/examples
  sketches
  $BASEDIR/Sketch
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
  runTests "${FQBN}" false "${SKETCHES[@]}"
  RES1=$?
  echo "::warning ::warning Compiled ${#SKETCHES[@]} Sketches for ${BOARD}. Average space consumption ${AVG_BYTES} Bytes"
fi

# Run tests with AES
RES2=0
if [ ${#SKETCHES_AES[@]} -gt 0 ]; then
  runTests "${FQBN}" true "${SKETCHES_AES[@]}"
  RES2=$?
  echo "::warning ::warning Compiled ${#SKETCHES_AES[@]} Sketches with AES supprt for ${BOARD}. Average space consumption ${AVG_BYTES_AES} Bytes"
fi

# Return error
if [ $RES1 -gt 0 ] || [ $RES2 -gt 0 ]; then
  >&2 echo "Errors occurred!"
  exit 1
fi
