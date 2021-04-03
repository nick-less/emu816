#!/bin/bash
IMAGE=flashImage.bin

function dumpFile {

# target address
echo -n -e  \\x$2\\x$3\\$4 >>$IMAGE

FILESIZE=$(stat -f%z $1)

BYTE=$(((FILESIZE >> 8) & 0xff))
BYTE=$( printf "%x" $BYTE ) 
echo  -n -e  \\x$BYTE >>$IMAGE
BYTE=$((FILESIZE & 0xff))
BYTE=$( printf "%x" $BYTE ) 
echo -n -e \\x$BYTE >>$IMAGE

# starter
cat $1 >>$IMAGE
echo dumped $1  $FILESIZE bytes to address 0x$2$3$4
}

cd loader
make clean 
make
cd ../starter
make clean starter
cd ..

if [ -e $IMAGE ]
then
rm -f $IMAGE
fi
# first 256 bytes loader 
#cat loader/loader >$IMAGE
#cat loader/loader >>$IMAGE
# after


dumpFile starter/starter 00 10 00
#dumpFile ../../hardware/mini-65816/rom/osi_bas 00 c0 00

dumpFile ../../6502-Monitor/monitor 00 E0 00

#dumpFile basic4.b000 00 b0 00
#dumpFile edit4b80e000.bin 00 e0 00
#dumpFile kernal4.f000 00 f0 00


#dumpFile basic20.bin 00 a0 00
#dumpFile kernal-64.bin 00 e0 00
#dumpFile charset.bin 00 d0 00
#dumpFile 'Galaga_(None).prg' 00 08 01
