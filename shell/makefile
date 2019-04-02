CC = gcc
CFLAGS = -Iinclude
LDLIBS = -ltermcap

INCLUDES = include/shell.h

OBJS = src/shell.o src/speccom.o

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c -o $@ $< $(LDLIBS)

bin/shell: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -fr src/*.o bin/*
