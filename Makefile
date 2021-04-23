CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++17 -O2
FILESYSTEM_FLAG = -lstdc++fs

.PHONY: all clean

all: serwer

serwer: main.o Server.o server_utilities.o exceptions.h HttpRequest.o
	$(CC) $(CFLAGS) -o $@ $^ $(FILESYSTEM_FLAG)

HttpRequest.o: HttpRequest.cpp HttpRequest.h server_utilities.h exceptions.h
	$(CC) $(CFLAGS) -c $< $(FILESYSTEM_FLAG)

server_utilities.o: server_utilities.cpp server_utilities.h
	$(CC) $(CFLAGS) -c $< $(FILESYSTEM_FLAG)

Server.o: Server.cpp Server.h HttpRequest.h server_utilities.h
	$(CC) $(CFLAGS) -c $< $(FILESYSTEM_FLAG)

main.o: main.cpp Server.h
	$(CC) $(CFLAGS) -c $< $(FILESYSTEM_FLAG)

clean:
	rm -f *.o main serwer
