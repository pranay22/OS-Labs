#!/bin/bash
## Test script, please execute with root privileges. We promise we won't harm your system.
## At least not deliberately... ;)

echo 'Installing kernel module'
cd ../1.1
make
insmod fifo.ko major=240
mknod -mode=666 /dev/deeds_fifo c 240 1
cd ../1.3
echo 'Kernel module installed'
echo 'Filling fifo with 32 items'
echo –n "0,1451001600,Merry Christmas!" > /dev/deeds_fifo
cat /dev/deeds_fifo
