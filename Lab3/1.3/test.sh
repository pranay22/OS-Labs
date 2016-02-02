#!/bin/bash
## Test script, please execute with root privileges. We promise we won't harm your system.
## At least not deliberately... ;)
#set -x
echo
echo 'Installing kernel module'
cd ../1.2
echo 'Kernel module installed'
echo 'Trying simple read and write'
echo "0,1,Merry Christmas!" > /dev/deeds_fifo
cat /dev/deeds_fifo
(
	echo 'Reading form an empty FIFO'
	echo 'This will blockk'
	cat /dev/deeds_fifo &
	cat /dev/deeds_fifo &
	cat /proc/deeds_fifo_stats
)
(
	sleep 5
	echo 'Deblocking'
	echo '1,1,Thank me for letting you out' > /dev/deeds_fifo
	sleep 5
	echo '1,2,Thank me for letting you out' > /dev/deeds_fifo 
)
(
	echo 'Filling in the FIFO'
    COUNTER=0
   	while [  $COUNTER -lt 32 ]; do
		echo "0,1,Message $COUNTER" > /dev/deeds_fifo
        let COUNTER=COUNTER+1 
   	done
	echo "Current statistics"
	cat /proc/deeds_fifo_stats
)
(
	echo "Writing to a full FIFO (Blocking)"
	echo 
)

