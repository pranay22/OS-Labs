#obj-m := deeds_clock.o fifo.o
obj-m := fifo.o
PWD :=  $(shell pwd)
KDIR := $(shell uname -r)
SHELL := /bin/bash

default:	
	make clean
	make -C /lib/modules/$(KDIR)/build M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(KDIR)/build M=$(PWD) clean