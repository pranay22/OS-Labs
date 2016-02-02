obj-m := fifo.o
#ccflags-y := -I.
PWD :=  $(shell pwd)
KDIR := $(shell uname -r)
SHELL := /bin/bash

default:	
	make clean
	make -C /lib/modules/$(KDIR)/build M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(KDIR)/build M=$(PWD) clean

load:
	sudo dmesg --clear;\
	sudo insmod fifo.ko major=240 ;\
	sudo mknod /dev/deeds_fifo --mode=666 c 240 0;\

remove:
	sudo rmmod fifo;\
	sudo rm /dev/deeds_fifo