CC = g++
CFLAGS = -g -std=c++0x -pedantic -Wall -Wextra -Weffc++ \
	 -Woverloaded-virtual -Wc++11-compat
LDFLAGS =
OBJFILES := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: tegel

tegel: main.o $(OBJFILES)
	$(CC) -o $@ $^ $(LDFLAGS)

lexical.cpp: lexical.l parser.hpp type.hpp symbol.hpp constant.hpp ast.hpp ast_factories.hpp
	flex -o $@ lexical.l

%.o: %.cpp
	$(CC) -c $(CFLAGS) -o $@ $<

parser.cpp: parser.y type.hpp
	bison -d -o $@ parser.y

parser.hpp: parser.cpp

clean:
	rm -f *.o tegel parser.cpp parser.hpp lexical.cpp *.gch

generate-tags:
	ctags -R --sort=yes --c++-kinds=+p --fields=+iaS --extra=+q

doc:
	doxygen doxygen

indent:
	astyle -C -N -xC79 *.hpp *.cpp && rm *.orig
