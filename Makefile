
OBJS = src/drunkard.o
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -pedantic -I./include -I/usr/include/libtcod
LDFLAGS = -lm

all: libdrunkard.a examples

$(OBJS): src/drunkard.c

libdrunkard.a: $(OBJS)
	ar rcs "lib/libdrunkard.a" $(OBJS)
	ranlib lib/libdrunkard.a

examples: libdrunkard.a
	$(CC) $(CFLAGS) examples/cave.c $(LDFLAGS) -L./lib -ldrunkard -o examples/cave
	$(CC) $(CFLAGS) examples/dung.c $(LDFLAGS) -L./lib -ldrunkard -o examples/dung

install:
	mv include/drunkard.h /usr/include
	mv lib/libdrunkard.a /usr/lib

clean:
	@rm -f lib/*.a
	@rm -f src/*.o
	@rm -f examples/cave
