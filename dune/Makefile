ARCH ?= loongarch
CC ?= gcc

CPPFLAGS += -DLOONGSON
CFLAGS += -g -static -pthread

ifeq ($(ARCH),mips)
ARCH_CPP := MIPS_ARCH
else ifeq ($(ARCH),loongarch)
ARCH_CPP := LOONGSON_ARCH
else ifeq ($(ARCH),x86_64)
ARCH_CPP := X86_64_ARCH
else
$(error Unsupported ARCH=$(ARCH))
endif

CPPFLAGS += -DARCH=$(ARCH_CPP)

# CC=/usr/local/musl/bin/musl-gcc
# CFLAGS += -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
# $(info === $(CFLAGS))
all: libdune.a

HEADERS=$(ARCH)/arch.h $(ARCH)/internal.h dune.h config.h

$(ARCH)/arch.o: $(ARCH)/arch.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -c -o $@

$(ARCH)/entry.o: $(ARCH)/entry.S $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -c -o $@

dune.o:dune.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -c -o $@

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
