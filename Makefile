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
	rm -rf src/*.o tests/*.o core* peer index *~ test_*

peer: src/peer-driver.o src/peer.o src/tcp.o
	$(CC) $(CFLAGS) -o peer src/peer-driver.o src/peer.o src/tcp.o

index: src/index-driver.o src/tcp.o
	$(CC) $(CFLAGS) -o index src/index-driver.o src/tcp.o

test_messages: tests/test-messages.o src/messages.o
	$(CC) $(CFLAGS) -o test_messages tests/test-messages.o src/messages.o
