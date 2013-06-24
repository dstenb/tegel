CC = g++
CFLAGS = -std=c++0x -pedantic -Wall -Wextra
LDFLAGS =

all: tegel

tegel: main.o parser.o lexical.o type.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.cpp
	$(CC) -c $(CFLAGS) $<


parser.o: parser.cpp
	$(CC) -c $(CFLAGS) $<

lexical.o: lexical.cpp
	$(CC) -c $(CFLAGS) $<

type.o: type.cpp
	$(CC) -c $(CFLAGS) $<

parser.cpp: parser.y type.hpp
	bison -d -o $@ parser.y

parser.hpp: parser.cpp

lexical.cpp: lexical.l parser.hpp type.hpp
	flex -o $@ $^

clean:
	rm -f *.o tegel

generate-tags:
	ctags -R --sort=yes --c++-kinds=+p --fields=+iaS --extra=+q
