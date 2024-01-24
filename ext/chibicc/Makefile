CFLAGS=-g -Wall -Wextra -pedantic -Wfatal-errors -Werror

SRCS=$(wildcard *.cc)

chibicc: $(SRCS) chibicc.h
	gcc $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -rf chibicc 

.PHONY: test clean
