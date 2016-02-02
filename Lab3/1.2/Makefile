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