ARCH=arm
CROSS_COMPILE=arm-none-linux-gnueabi
obj-m := dune.o
dune-objs := \
			core.o \
			vz.o

KDIR := /home/maritns3/core/loongson-dune/cross
PWD := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
