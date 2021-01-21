all: dune.out
	./dune.out
	sudo dmesg -c

dune.out: sync 
	# gcc -static -g -DLOONGSON dune.c exception.S -o dune.out
	# gcc -L/home/loongson/musl/lib/libc.a -g -DLOONGSON dune.c exception.S -o dune.out
	# /usr/local/musl/bin/musl-gcc -static -g -DLOONGSON dune.c exception.S -o dune.out
	
	/usr/local/musl/bin/musl-gcc -I/usr/local/musl/include -static -g -DLOONGSON dune.c exception.S -o dune.out -Wl,-Map=a.map
# ass:
	# mips64el-loongson-linux-gcc -c hello.c -o hello.o
	# mips64el-loongson-linux-objdump -d hello.o

sync:
	rsync --delete -avzh --exclude='/.git' --filter="dir-merge,- .gitignore" maritns3@10.90.50.149:/home/maritns3/core/loongson-dune/3day/ /home/loongson/dune/

gdb: dune.out
	gdb -x debug.txt dune.out

ass: sync
	gcc -g hello.c -o hello.out
	./hello.out
