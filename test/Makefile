TESTSRCS ?= $(wildcard *.c)
TEST := $(TESTSRCS:.c=.out)

$(info == $(LIBSRCS))
$(info == $(TEST))

LIBDUNE := ../dune/libdune.a

%.out: %.c $(LIBDUNE)
	gcc $< $(LIBDUNE)  -o $@

all: $(TEST)

so.out: hello_world.c libdune.so
	gcc -L../dune -l:libdune.so $< -o $@ -Wl,-R../dune
	./$@

clean:
	rm -f $(TEST)

libdune.a:
	$(MAKE) -C ../dune 

libdune.so:
	$(MAKE) -C ../dune 
