CC=gcc
CFLAGS= -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11

bin/server: clean bin/client bin/cli
	$(CC) $(CFLAGS) src/server.c -o bin/server
bin/client:
	$(CC) $(CFLAGS) src/client.c -o bin/client
bin/cli:
	$(CC) $(CFLAGS) src/cli.c -o bin/cli

clean:
	rm -f bin/*
	rm -f obj/*