# Simple Makefile for WSL/Unix (bash)
CC = mpicc
CFLAGS = -O2 -std=c11
LDFLAGS = -lm

all: sum_static sum_dynamic

sum_static: sum_static.c
	$(CC) $(CFLAGS) sum_static.c -o sum_static $(LDFLAGS)

sum_dynamic: sum_dynamic.c
	$(CC) $(CFLAGS) sum_dynamic.c -o sum_dynamic $(LDFLAGS)

clean:
	rm -f sum_static sum_dynamic
