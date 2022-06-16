CC := gcc
CFLAGS = $(WFLAGS) $(OPTIM)

WFLAGS  := -Wall -Wextra -Wpedantic --std=c99

.PHONY: all
all: debug

.PHONY: debug
debug: debug-sanitize

.PHONY: debug-sanitize
debug-sanitize: DEBUG = -fsanitize=address,undefined
debug-sanitize: debug-common

.PHONY: debug-no-sanitize
debug-no-sanitize: debug-common

.PHONY: debug-common
debug-common: OPTIM := -g
debug-common: dirs bin/test

.PHONY: release
release: OPTIM := -O3
release: DEFINES += -DNDEBUG
release: dirs bin/test

# test:

bin/test: obj/test.o
	$(CC) -o $@ $^ $(DEBUG) $(DEFINES)

obj/test.o: src/test.c
	$(CC) -c -o $@ $< $(CFLAGS) $(DEBUG) $(DEFINES)

# dirs

.PHONY: dirs
dirs: obj lib bin

obj:
	mkdir -p $@

lib:
	mkdir -p $@

bin:
	mkdir -p $@

# clean

.PHONY: clean
clean:
	rm -rf obj/* bin/* lib/*
