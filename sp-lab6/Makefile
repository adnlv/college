CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Wconversion -fsanitize=address,undefined -g
LDFLAGS ?= -fsanitize=address,undefined

TARGETS = tree_view

.PHONY: all clean

all: $(TARGETS)

tree_view: tree_view.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)