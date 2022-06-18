CC := gcc
CFLAGS = $(WFLAGS) $(OPTIM)

WFLAGS  := -Wall -Wextra -Wpedantic --std=c99

DEPS_LIBS := `pkgconf --libs fontconfig` -Ltyrant/lib -ltyrant
DEPS_CFLAGS := `pkgconf --cflags fontconfig | sed 's/-I/-isystem/g'` -Ityrant/src

.PHONY: all
all: debug

.PHONY: debug
debug: debug-sanitize

.PHONY: debug-sanitize
debug-sanitize: DEBUG = -fsanitize=address,undefined
debug-sanitize: TYRANT_TARGET = debug-sanitize
debug-sanitize: debug-common

.PHONY: debug-no-sanitize
debug-no-sanitize: TYRANT_TARGET = debug-no-sanitize
debug-no-sanitize: debug-common

.PHONY: debug-common
debug-common: OPTIM := -g
debug-common: dirs bin/test

.PHONY: release
release: OPTIM := -O3
release: DEFINES += -DNDEBUG
release: TYRANT_TARGET = release
release: dirs bin/test

# test:

bin/test: obj/test.o tyrant/lib/libtyrant.a
	$(CC) -o $@ $^ $(DEPS_LIBS) $(DEBUG) $(DEFINES)

obj/test.o: src/test.c
	$(CC) -c -o $@ $< $(CFLAGS) $(DEPS_CFLAGS) $(DEBUG) $(DEFINES)

# tyrant

tyrant/lib/libtyrant.a: tyrant/src/*
	make -C tyrant $(TYRANT_TARGET)

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
	make -C tyrant clean
