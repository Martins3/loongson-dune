all: sync
	gcc -DLOONGSON dune.c -o dune.out
	./dune.out
	sudo dmesg -c

# ass:
	# mips64el-loongson-linux-gcc -c hello.c -o hello.o
	# mips64el-loongson-linux-objdump -d hello.o

sync:
	rsync --delete -avzh --exclude='/.git' --filter="dir-merge,- .gitignore" maritns3@10.90.50.149:/home/maritns3/core/loongson-dune/3day/ /home/loongson/dune/

ass: sync
	gcc -g hello.c -o hello.out
	./hello.out
