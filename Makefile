#  Compiler Flags
CC 			= g++
CFLAGS      = -g -std=c++11 -pedantic -Wall -Werror -Wextra \
              -Wno-overlength-strings -Wfatal-errors -pedantic
LDFLAGS     = -g
CPPFLAGS    = -I.
RM          = rm -f 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean: 
	rm -rf src/*.o tests/*.o core* main *~ tests/test-*

main: src/main.o
	$(CC) $(CFLAGS) -o main src/main.o
