CFLAGS := -g -DLOONGSON -static
CFLAGS += -pthread

# CC=/usr/local/musl/bin/musl-gcc
# CFLAGS += -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
# $(info === $(CFLAGS))
all: libdune.a

ARCH=loongarch
HEADERS=$(ARCH)/arch.h $(ARCH)/internal.h dune.h config.h

$(ARCH)/arch.o: $(ARCH)/arch.c $(HEADERS)
	gcc $(CFLAGS) $< -c -o $@

$(ARCH)/entry.o: $(ARCH)/entry.S $(HEADERS)
	gcc $(CFLAGS) $< -c -o $@

dune.o:dune.c $(HEADERS)
	gcc $(CFLAGS) $< -c -o $@

libdune.a: $(ARCH)/arch.o $(ARCH)/entry.o dune.o
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
	rm -f libdune.a $(ARCH)/arch.o $(ARCH)/entry.o dune.o
