CC       = gcc
CFLAGS   = -O2 -Wall -Wextra -std=c11 -fopenmp
LDFLAGS  = -fopenmp

BINARIES = openmp_histogram openmp_monte_carlo_pi

.PHONY: all clean

all: $(BINARIES)

openmp_histogram: openmp_histogram.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

openmp_monte_carlo_pi: openmp_monte_carlo_pi.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) -lm

clean:
	rm -f $(BINARIES) *.o
