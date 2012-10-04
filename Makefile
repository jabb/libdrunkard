
OBJS = src/drunkard.o
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -pedantic -I./include -I/usr/include/libtcod
LDFLAGS = -lm

all: libdrunkard.a examples

$(OBJS): src/drunkard.c

libdrunkard.a: $(OBJS)
	-@echo -n 'Building library...'
	@ar rcs "lib/libdrunkard.a" $(OBJS)
	@ranlib lib/libdrunkard.a
	-@echo 'success!'

examples: libdrunkard.a
	-@echo -n 'Building examples...'
	-@$(CC) $(CFLAGS) examples/cave.c $(LDFLAGS) -L./lib -ldrunkard -o examples/cave 2> /dev/null
	-@$(CC) $(CFLAGS) examples/dung.c $(LDFLAGS) -L./lib -ldrunkard -o examples/dung 2> /dev/null
	-@$(CC) $(CFLAGS) examples/atw.c $(LDFLAGS) -L./lib -ldrunkard -o examples/atw 2> /dev/null
	-@$(CC) $(CFLAGS) examples/screen_shotter.c $(LDFLAGS) -L./lib -ldrunkard -ltcod -o examples/screen_shotter 2> /dev/null
	-@echo 'success!'

install:
	mv include/drunkard.h /usr/include
	mv lib/libdrunkard.a /usr/lib

uninstall:
	rm /usr/include/drunkard.h
	rm /usr/lib/libdrunkard.a

clean:
	@rm -f lib/*.a
	@rm -f src/*.o
	@rm -f examples/cave
