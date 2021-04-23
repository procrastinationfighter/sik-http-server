CC = g++
CFLAGS = -Wall -Wextra -std=c++17 -O2

.PHONY: all clean

all: serwer

serwer: main.o Server.o server_utilities.o exceptions.h HttpRequest.o
	$(CC) $(CFLAGS) -o $@ $^

HttpRequest.o: HttpRequest.cpp HttpRequest.h server_utilities.h exceptions.h
	$(CC) $(CFLAGS) -c $<

server_utilities.o: server_utilities.cpp server_utilities.h
	$(CC) $(CFLAGS) -c $<

Server.o: Server.cpp Server.h HttpRequest.h server_utilities.h
	$(CC) $(CFLAGS) -c $<

main.o: main.cpp Server.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o main serwer
