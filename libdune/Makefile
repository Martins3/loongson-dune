CC      = gcc
CFLAGS  = -Wall -g -MD -ggdb3
LDFLAGS = -static -L. -ldune

OBJ     = kvm.o
TEST_OBJ = mintest.o

# NOFPU_OBJ = trap.o

# $(NOFPU_OBJ): EXTRA_FLAGS := -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -msoft-float

all: kvm

libdune.a: $(OBJ)
	$(AR) crD $(@) $(OBJ)

clean:
	rm -f *.o test *.d libdune.a

mintest.out: mintest.o $(OBJ)
	$(CC) $^ -o $@

test: mintest.out
	./$<
	echo "please don do any test, any run with a github issue!"

-include *.d

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) -o $@ -c $<
