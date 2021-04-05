CC=gcc
CFLAGS= -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11

bin/server: clean bin/client
	$(CC) $(CFLAGS) src/server.c -o bin/server -lreadline
bin/client:
	$(CC) $(CFLAGS) src/client.c -o bin/client -lreadline
clean:
	rm -f bin/*
	rm -f obj/*