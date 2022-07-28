CC := gcc
CFLAGS = $(WFLAGS) $(OPTIM)

WFLAGS := -Wall -Wextra -Wpedantic -std=c99

LFLAGS = -L$(LIB_DIR) \
	  -lfontfilter \
	  -ltyrant \
	  `pkgconf --libs fontconfig`

DEPS_CFLAGS := -Ityrant/src \
	       `pkgconf --cflags fontconfig | sed 's/-I/-isystem/g'`

OUT_DIR := .
OBJ_DIR = $(OUT_DIR)/obj
LIB_DIR = $(OUT_DIR)/lib
BIN_DIR = $(OUT_DIR)/bin

OPTIM_debug := -g
OPTIM_release := -O3
OPTIM = $(OPTIM_$(TARGET))

DEBUG_debug = -fsanitize=address,undefined
DEBUG = $(DEBUG_$(TARGET))

.PHONY: all
all: debug

.PHONY: debug
debug: TARGET = debug
debug: dirs $(BIN_DIR)/examples

.PHONY: release
release: TARGET = release
release: DEFINES += -DNDEBUG
release: dirs $(BIN_DIR)/examples

.PHONY: library
library: library-release

.PHONY: library-debug
library-debug: TARGET = debug
library-debug: dirs $(LIB_DIR)/libfontfilter.a

.PHONY: library-release
library-release: TARGET = release
library-release: DEFINES += -DNDEBUG
library-release: dirs $(LIB_DIR)/libfontfilter.a

# examples:

$(BIN_DIR)/examples: $(OBJ_DIR)/examples.o $(LIB_DIR)/libfontfilter.a $(LIB_DIR)/libtyrant.a
	$(CC) -o $@ $^ $(LFLAGS) $(DEBUG) $(DEFINES)

$(OBJ_DIR)/examples.o: examples.c $(LIB_HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEPS_CFLAGS) $(DEBUG) $(DEFINES) -Isrc

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
	make -C tyrant $(TARGET) \
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
