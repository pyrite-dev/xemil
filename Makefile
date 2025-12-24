CC = gcc
CFLAGS = -I include -fPIC -g
LDFLAGS = -shared
LIBS =

OBJS = src/core.o src/file.o src/unicode.o src/util.o src/array.o

SO = .so

.PHONY: all format clean
.SUFFIXES: .c .o

all: src/libxemil$(SO)

format:
	clang-format --verbose -i `find src include -name "*.c" -or -name "*.h"` example.c

example: example.c src/libxemil$(SO)
	$(CC) -o $@ example.c -I include -Wl,-R./src -L./src -lxemil

src/libxemil$(SO): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f src/*.o src/*.so src/*.dll example
