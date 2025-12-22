CC = gcc
CFLAGS = -I include -fPIC -g
LDFLAGS = -shared
LIBS =

OBJS = src/core.o src/file.o src/unicode.o src/stb_ds.o

SO = .so

.PHONY: all format clean
.SUFFIXES: .c .o

all: libxmllib$(SO)

format:
	clang-format --verbose -i `find src include -name "*.c" -or -name "*.h"` example.c

example: example.c libxmllib$(SO)
	$(CC) -o $@ example.c -I include -Wl,-R. -L. -lxmllib

libxmllib$(SO): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f src/*.o *.so *.dll example
