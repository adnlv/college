CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Wconversion -fsanitize=address,undefined -g
LDFLAGS ?= -fsanitize=address,undefined

TARGETS = pwd sigquit

.PHONY: all clean

all: $(TARGETS)

pwd: pwd.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

sigquit: sigquit.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)
