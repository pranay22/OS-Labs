#obj-m := deeds_clock.o fifo.o
obj-m := fifo.o
PWD :=  $(shell pwd)
KDIR := $(shell uname -r)
SHELL := /bin/bash
#MODULE := deeds_clock.ko fifo.ko
MODULE := fifo.ko
default:	
	make clean
	make -C /lib/modules/$(KDIR)/build M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(KDIR)/build M=$(PWD) clean
	
load:
	@echo "Loading modules: $(MODULE)"
	make remove
	sudo insmod $(MODULE)
	@echo "$(MODULE) loaded"

remove:
	@echo "removing modules $(MODULE)"; \
	if [ -e "/proc/deeds_clock" ]; \
	then \
		echo "removing module deeds_clock"; \
		sudo rmmod deeds_clock; \
	fi; \
	if [ -e "/proc/fifo_config" ]; \
	then \
		echo "removing module fifo.ko"; \
		sudo rmmod fifo; \
	fi \
	