CFLAGS=-g -DLOONGSON
all: libdune.a libdune.so

dune.o:dune.c
	gcc $(CFLAGS) $^ -c -o $@

exception.o:exception.S
	gcc $(CFLAGS) $^ -c -o $@

libdune.a: exception.o dune.o
	ar -rcs $@ $^ 

libdune.so: exception.o dune.o
	gcc -shared $^ -o $@

dune.out:
	echo "oops"
	# gcc -static -g -DLOONGSON dune.c exception.S -o dune.out
	# gcc -g -DLOONGSON dune.c exception.S -o dune.out
	# gcc -L/home/loongson/musl/lib/libc.a -g -DLOONGSON dune.c exception.S -o dune.out
	# /usr/local/musl/bin/musl-gcc -static -g -DLOONGSON dune.c exception.S -o dune.out
	
	# /usr/local/musl/bin/musl-gcc -I/usr/local/musl/include -static -g -DLOONGSON dune.c test_clone.c exception.S -o dune.out -Wl,-Map=a.map

# ass:
	# mips64el-loongson-linux-gcc -c hello.c -o hello.o
	# mips64el-loongson-linux-objdump -d hello.o

gdb: dune.out
	gdb -x debug.txt dune.out
