#!/bin/bash

BOOTLOADER=$1

AVRDUDE=`which avrdude`

if [ ! -f $AVRDUDE ]; then
  echo Error can not find avrdude
  exit 5
fi

if [ "$BOOTLOADER" == "" ]; then
  echo Usage: flash.sh bootloader.hex
  exit 5
fi
if [ ! -f $BOOTLOADER ]; then
  echo Error $BOOTLOADER missing
  exit 5
fi

# Int. RC 8MHz, Brown-out detection disabled
avrdude -p m328p -P usb -c usbasp -B 10 -U lfuse:w:0xE2:m -U hfuse:w:0xD0:m -U efuse:w:0xFF:m -U lock:w:0xFF:m
avrdude -p m328p -P usb -c usbasp -V -U flash:w:$BOOTLOADER

