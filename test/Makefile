TESTSRCS ?= $(wildcard *.c)
TEST := $(TESTSRCS:.c=)

$(info == $(TESTSRCS))
$(info == $(TEST))

LIBDIR := ../dune

LIBDUNE := $(LIBDIR)/libdune.a

DEPS_FILES := dune.c exception.S cp0.h
DEPS := $(addprefix $(LIBDIR)/,$(DEPS_FILES))

$(info $(DEPS))


all: $(TEST)

%: %.c $(DEPS)
	$(MAKE) -C ../dune 
	gcc $< $(LIBDUNE) -o $@

clean:
	rm -f $(TEST)
	$(MAKE) -C ../dune/ clean
