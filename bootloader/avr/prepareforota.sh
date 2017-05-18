#!/bin/bash -ex

QUAL=`date +%Y%m%d%H%M`
FILE=$1
NAME=`basename $1`
BIN=`basename $1 .hex`.bin
EQ3=`basename $1 .hex`_$QUAL.eq3

cp $FILE $NAME
./srecord/srec_cat.exe $NAME -intel -fill 0xFF 0x0000 0x6FFE \
  -Cyclic_Redundancy_Check_16_Little_Endian 0x6FFE -o  $BIN -binary
  
./bin2eq3.php $BIN $EQ3 128
rm $NAME $BIN
