# UNAME := $(shell uname -m)

# KDIR := /home/maritns3/core/loongson-dune/cross

KDIR = /lib/modules/`uname -r`/build

# https://stackoverflow.com/questions/28936199/why-is-pr-debug-of-the-linux-kernel-not-giving-any-output
# -DDEBUG is essential, demsg will give nothing otherwise.
#
obj-m        = mm.o

CFLAGS_mm.o := -DDEBUG -o0

kbuild:
	rsync --delete -avzh --exclude='/.git' --filter="dir-merge,- .gitignore" maritns3@10.90.50.149:/home/maritns3/core/loongson-dune/3day/ /home/loongson/loongson/dune/
	make -C $(KDIR) M=`pwd` && sudo insmod mm.ko && sudo rmmod mm && dmesg

clean:
	make -C $(KDIR) M=`pwd` clean
