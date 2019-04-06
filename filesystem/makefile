CC = gcc
CFLAGS = -Iinclude
LDFLAGS = -Llib
LDLIBS = -lfs

AR = ar
ARFLAGS = crv

INCLUDES = include/fs.h

OBJS = src/fs.o src/dir.o

EXAMPLES = examples/example.ct


%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c -o $@ $<

%.ct: %.c lib/libfs.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS)

lib/libfs.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $?

examples: $(EXAMPLES);

format: src/format.c
	$(CC) $(CFLAGS) -o bin/$@ $< $(LDFLAGS) $(LDLIBS)

clean:
	rm -fr src/*.o lib/* bin/* examples/*.ct
