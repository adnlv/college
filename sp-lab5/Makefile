CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Wconversion -fsanitize=address,undefined -g
LDFLAGS ?= -fsanitize=address,undefined

TARGETS = pwd sigquit sigurs1

.PHONY: all clean

all: $(TARGETS)

pwd: pwd.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

sigquit: sigquit.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

sigurs1: sigusr1.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)
