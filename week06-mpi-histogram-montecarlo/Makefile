CC      = mpicc
CFLAGS  = -O2 -Wall -Wextra -std=c11
LDFLAGS =

BINARIES = histogram monte_carlo_pi

.PHONY: all clean

all: $(BINARIES)

histogram: histogram.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

monte_carlo_pi: monte_carlo_pi.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) -lm

clean:
	rm -f $(BINARIES) *.o
