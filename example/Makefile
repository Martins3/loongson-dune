# TESTSRCS ?= $(wildcard *.c)
TESTSRCS ?= clone.c
TARGET := $(patsubst %.c, %.out, $(TESTSRCS))

LIBDIR := ../dune

LIBDUNE := $(LIBDIR)/libdune.a

ARCH ?= loongarch

DEPS_FILES := config.h dune.h dune.c $(ARCH)/arch.c $(ARCH)/entry.S $(ARCH)/internal.h 
DEPS := $(addprefix $(LIBDIR)/,$(DEPS_FILES))

# LDLIBS			+= -lpthread -lrt
# LDLIBS			+= -static
# CC=/usr/local/musl/bin/musl-gcc
# CFLAGS      += -g -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
# CFLAGS      += -g -pthread -static
CFLAGS      += -g -pthread -ldl

# $(info $(DEPS) )
$(info $(TARGET) )

all:$(TARGET)

$(TARGET): %.out: %.c

%.out: %.c $(DEPS)
	$(MAKE) -C $(LIBDIR) ARCH=$(ARCH)
	$(CC) $(CFLAGS) -I$(LIBDIR) $(LDLIBS) $< $(LIBDUNE) -o $@

clean:
	rm -f $(TARGET)
	rm -f *.txt
	$(MAKE) -C ../dune/ clean ARCH=$(ARCH)
