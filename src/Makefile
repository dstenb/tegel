CC = g++
CFLAGS = -std=c++0x -pedantic -Wall -Wextra
LDFLAGS =

all: tegel

tegel: main.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.cpp
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f *.o tegel

generate-tags:
	ctags -R --sort=yes --c++-kinds=+p --fields=+iaS --extra=+q
