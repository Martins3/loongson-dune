CFLAGS := -g -DLOONGSON -static
# CFLAGS += -pthread
CC=/usr/local/musl/bin/musl-gcc
# CFLAGS += -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
# $(info === $(CFLAGS))
all: libdune.a

dune.o:dune.c cp0.h
	gcc $(CFLAGS) $< -c -o $@

exception.o:exception.S cp0.h
	gcc $(CFLAGS) $< -c -o $@

libdune.a: exception.o dune.o
	ar -rcs $@ $^

dune.out:
	echo "oops"
	# gcc -static -g -DLOONGSON dune.c exception.S -o dune.out
	# gcc -g -DLOONGSON dune.c exception.S -o dune.out
	# gcc -L/home/loongson/musl/lib/libc.a -g -DLOONGSON dune.c exception.S -o dune.out
	# /usr/local/musl/bin/musl-gcc -static -g -DLOONGSON dune.c exception.S -o dune.out
	
	# /usr/local/musl/bin/musl-gcc -I/usr/local/musl/include -static -g -DLOONGSON dune.c test_clone.c exception.S -o dune.out -Wl,-Map=a.map

gdb: dune.out
	gdb -x debug.txt dune.out

clean:
	rm -f libdune.a dune.o exception.o
