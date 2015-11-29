obj-m := deeds_clock.o
PWD :=  $(shell pwd)
KDIR := $(shell uname -r)

default:
	make clean
	make -C /lib/modules/$(KDIR)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KDIR)/build M=$(PWD) clean
