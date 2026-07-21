CC          = mpicc
PTHREAD_CC  = gcc
CFLAGS      = -O2 -Wall -Wextra -std=c11
PTHREAD_LIB = -pthread
LDFLAGS     =

BINARIES = histogram monte_carlo_pi pthread_histogram pthread_monte_carlo_pi

.PHONY: all clean

all: $(BINARIES)

histogram: histogram.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

monte_carlo_pi: monte_carlo_pi.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) -lm

pthread_histogram: pthread_histogram.c
	$(PTHREAD_CC) $(CFLAGS) $< -o $@ $(PTHREAD_LIB)

pthread_monte_carlo_pi: pthread_monte_carlo_pi.c
	$(PTHREAD_CC) $(CFLAGS) $< -o $@ $(PTHREAD_LIB) -lm

clean:
	rm -f $(BINARIES) *.o
