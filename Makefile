CC := gcc
CFLAGS = $(WFLAGS) $(OPTIM)

WFLAGS  := -Wall -Wextra -Wpedantic --std=c99

DEPS_LIBS := `pkgconf --libs fontconfig` -Ltyrant/lib -ltyrant
DEPS_CFLAGS := `pkgconf --cflags fontconfig | sed 's/-I/-isystem/g'` -Ityrant/src

.PHONY: all
all: debug

.PHONY: debug
debug: DEBUG = -fsanitize=address,undefined
debug: TYRANT_TARGET = debug
debug: OPTIM := -g
debug: dirs bin/test

.PHONY: release
release: OPTIM := -O3
release: DEFINES += -DNDEBUG
release: TYRANT_TARGET = release
release: dirs bin/test

# test:

bin/test: obj/test.o lib/libfontfilter.a tyrant/lib/libtyrant.a
	$(CC) -o $@ $^ -Llib -lfontfilter $(DEPS_LIBS) $(DEBUG) $(DEFINES)

obj/test.o: src/test.c $(LIB_HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEPS_CFLAGS) $(DEBUG) $(DEFINES)

# fontfilter

LIB_HEADERS = src/fontfilter.h tyrant/src/tyrant.h
LIB_OBJS = obj/fontfilter.o

lib/libfontfilter.a: $(LIB_OBJS) Makefile
	[ -f $@ -a Makefile -nt $@ ] \
		&& (rm $@ ; ar cqs $@ $(LIB_OBJS)) \
		|| ar crs $@ $(LIB_OBJS)

obj/fontfilter.o: src/fontfilter.c $(LIB_HEADERS)
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
