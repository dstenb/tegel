CC = g++
CFLAGS = -std=c++0x -pedantic -Wall -Wextra
LDFLAGS =
OBJFILES := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: tegel

tegel: main.o $(OBJFILES)
	$(CC) -o $@ $^ $(LDFLAGS)

lexical.cpp: lexical.l parser.hpp type.hpp symbol.hpp constant.hpp
	flex -o $@ lexical.l

%.o: %.cpp
	$(CC) -c $(CFLAGS) -o $@ $<

parser.cpp: parser.y type.hpp lexical.cpp
	bison -d -o $@ parser.y

parser.hpp: parser.cpp lexical.cpp

clean:
	rm -f *.o tegel parser.cpp parser.hpp lexical.cpp

generate-tags:
	ctags -R --sort=yes --c++-kinds=+p --fields=+iaS --extra=+q
