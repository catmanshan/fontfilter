CC := gcc
CFLAGS = $(WFLAGS) $(OPTIM)

WFLAGS  := -Wall -Wextra -Wpedantic --std=c99

DEPS_LIBS := `pkgconf --libs fontconfig` -ltyrant
DEPS_CFLAGS := `pkgconf --cflags fontconfig | sed 's/-I/-isystem/g'` -Ityrant/src

OUT_DIR := .
OBJ_DIR = $(OUT_DIR)/obj
LIB_DIR = $(OUT_DIR)/lib
BIN_DIR = $(OUT_DIR)/bin

.PHONY: all
all: debug

.PHONY: debug
debug: DEBUG = -fsanitize=address,undefined
debug: TYRANT_TARGET = debug
debug: OPTIM := -g
debug: dirs $(BIN_DIR)/test

.PHONY: release
release: OPTIM := -O3
release: DEFINES += -DNDEBUG
release: TYRANT_TARGET = release
release: dirs $(BIN_DIR)/test

.PHONY: library
library: library-release

.PHONY: library-debug
library-debug: DEBUG = -fsanitize=address,undefined
library-debug: TYRANT_TARGET = debug
library-debug: OPTIM := -g
library-debug: dirs $(LIB_DIR)/libfontfilter.a

.PHONY: library-release
library-release: OPTIM := -O3
library-release: DEFINES += -DNDEBUG
library-release: TYRANT_TARGET = release
library-release: dirs $(LIB_DIR)/libfontfilter.a

# test:

$(BIN_DIR)/test: $(OBJ_DIR)/test.o $(LIB_DIR)/libfontfilter.a $(LIB_DIR)/libtyrant.a
	$(CC) -o $@ $^ -L$(LIB_DIR) -lfontfilter $(DEPS_LIBS) $(DEBUG) $(DEFINES)

$(OBJ_DIR)/test.o: src/test.c $(LIB_HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEPS_CFLAGS) $(DEBUG) $(DEFINES)

# fontfilter

LIB_HEADERS = src/fontfilter.h tyrant/src/tyrant.h
LIB_OBJS = $(OBJ_DIR)/fontfilter.o

$(LIB_DIR)/libfontfilter.a: $(LIB_OBJS) Makefile
	[ -f $@ -a Makefile -nt $@ ] \
		&& (rm $@ ; ar cqs $@ $(LIB_OBJS)) \
		|| ar crs $@ $(LIB_OBJS)

$(OBJ_DIR)/fontfilter.o: src/fontfilter.c $(LIB_HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEPS_CFLAGS) $(DEBUG) $(DEFINES)

# tyrant

$(LIB_DIR)/libtyrant.a: tyrant/src/*
	make -C tyrant $(TYRANT_TARGET) \
		OBJ_DIR=../$(OBJ_DIR) \
		LIB_DIR=../$(LIB_DIR)

# dirs

.PHONY: dirs
dirs: $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $@

$(LIB_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

# clean

.PHONY: clean
clean:
	rm -rf obj/* bin/* lib/*
	make -C tyrant clean
