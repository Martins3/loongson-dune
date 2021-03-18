TESTSRCS ?= $(wildcard *.c)
TEST := $(TESTSRCS:.c=)

LIBDIR := ../dune

LIBDUNE := $(LIBDIR)/libdune.a

DEPS_FILES := dune.c exception.S cp0.h
DEPS := $(addprefix $(LIBDIR)/,$(DEPS_FILES))

# LDLIBS			+= -lpthread -lrt
# LDLIBS			+= -static
# CC=/usr/local/musl/bin/musl-gcc
# CFLAGS      += -g -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
CFLAGS      += -g 

all: $(TEST)

%: %.c $(DEPS)
	$(MAKE) -C $(LIBDIR)
	$(CC) $(CFLAGS) -I$(LIBDIR) $(LDLIBS) $< $(LIBDUNE) -o $@

clean:
	rm -f $(TEST)
	rm -f *.txt
	$(MAKE) -C ../dune/ clean
