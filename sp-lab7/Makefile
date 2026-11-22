CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Wconversion -O2 -g
SAN_FLAGS = -fsanitize=address,undefined

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

TARGET = current_time_80_6
SRC = current_time_80_6.c

.PHONY: all debug install uninstall clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

debug: $(SRC)
	$(CC) $(CFLAGS) $(SAN_FLAGS) $< -o $(TARGET)_debug $(SAN_FLAGS)

install: $(TARGET)
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 0755 "$(TARGET)" "$(DESTDIR)$(BINDIR)/$(TARGET)"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/$(TARGET)"

clean:
	rm -f $(TARGET) $(TARGET)_debug
