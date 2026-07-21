CFLAGS=-O2 -Wall -Wextra -pthread
RM := rm -f
CLEAN_TARGETS := pthreads_histogram pthreads_monte_carlo_pi pthreads_histogram.exe pthreads_monte_carlo_pi.exe

all: pthreads_histogram pthreads_monte_carlo_pi

pthreads_histogram: pthreads_histogram.c
	$(CC) $(CFLAGS) -o $@ $<

pthreads_monte_carlo_pi: pthreads_monte_carlo_pi.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	-$(RM) $(CLEAN_TARGETS)
