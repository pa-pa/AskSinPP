#!/bin/bash

HEXFILE=$1
QUAL=`date +%Y%m%d%H%M`
EQ3=`basename $1 .hex`_$QUAL.eq3
TOHEX=`basename $1 .hex`_$QUAL.hex

crc=0xFFFF
function crc16 {
  local byte=$1
  for ((j=0;j<8;j++)); do
    local f=$(( ($crc & 0x8000) >> 15 ))
	crc=$(( ($crc << 1) & 0xFFFF ))
	if (( $(( $byte & 0x80 )) != 0 )); then
	  crc=$(( $crc | 1 ))
	fi
	if (( $f != 0 )); then
	  crc=$(( $crc ^ 0x1021 ))
	fi
	byte=$(( ($byte << 1) & 0xFF ))
  done
}

BLOCKSIZE=128
INBLOCK=0
function blockstart {
  if (( $INBLOCK == 0 )); then echo -n "0080"; fi
  INBLOCK=$(( ($INBLOCK+1) & ($BLOCKSIZE-1) ))
}

function toout {
  local HEX=$1
  local LEN=$2
  for (( OFF=0; $OFF < $LEN; OFF++ )); do
    local PART=${HEX:$(($OFF*2)):2}
    local NUM=$((16#$PART))
	crc16 $NUM
	blockstart
	echo -n $PART
  done
}

(
OUT=0

cp "$HEXFILE" "$TOHEX"
while read LINE
do
#  echo
#  echo $LINE
  LEN=$((16#${LINE:1:2}))
  ADR=${LINE:3:4}
  TYP=${LINE:7:2}
  DAT=${LINE:9:$(($LEN+$LEN))}
  CRC=${LINE:$((9+$LEN+$LEN)):2}
#  echo $LEN $ADR $TYP $DAT $CRC
  
  if (( $TYP == 0 )); then
#    echo $OUT
    while (( $OUT < 16#$ADR )); do toout "FF" 1; OUT=$(($OUT+1)); done
    toout $DAT $LEN
    OUT=$(($OUT+$LEN))
  fi
done < <(cat $HEXFILE)

for (( i=$OUT; i < 16#EFFE; i++ )); do toout "FF" 1; done
crc16 0
crc16 0
printf "%02X%02X" $(( $crc & 0xff )) $(( $crc >> 8 ))  
) > $EQ3
