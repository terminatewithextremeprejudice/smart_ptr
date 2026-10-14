.PHONY: test

CC=clang
OUTEXT=
CFLAGS=-O2 -g -std=gnu99
#EXTRA_CFLAGS=-Wall -Wextra -I..
#EXTRA_CFLAGS=-I.. -Wextra -Wstrict-prototypes -Wno-unused-parameter -Wno-macro-redefined -Wno-format -fno-strict-aliasing
EXTRA_CLAGS=-I..
RELEASE_CFLAGS=-O2 -march=native

all: smart_ptr_test

test: smart_ptr_test
	$(RUNNER) ./smart_ptr_test$(OUTEXT)

test-all: test 

smart_ptr_test: smart_ptr_test.c ../smart_ptr.h Makefile
	$(CC) smart_ptr_test.c -o smart_ptr_test $(EXTRA_CFLAGS) $(CFLAGS)


clean:
	rm -f smart_ptr_test$(OUTEXT)

