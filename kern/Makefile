obj-m := dune.o
dune-objs := core.o

UNAME := $(shell uname -m)
KDIR := /home/maritns3/core/loongson-dune/cross

PWD := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
