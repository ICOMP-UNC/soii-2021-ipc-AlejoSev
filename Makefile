CC=gcc
CFLAGS= -g -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11

bin/server: clean obj/list_lib.o bin/client bin/cli bin/productor1 bin/productor2 bin/productor3 bin/sysv
	$(CC) $(CFLAGS) src/server.c obj/list_lib.o -o bin/server
bin/client:
	$(CC) $(CFLAGS) src/client.c -o bin/client
bin/cli:
	$(CC) $(CFLAGS) src/cli.c -o bin/cli
bin/productor1:
	$(CC) $(CFLAGS) src/productor1.c -o bin/productor1
bin/productor2:
	$(CC) $(CFLAGS) src/productor2.c -o bin/productor2
bin/productor3:
	$(CC) $(CFLAGS) src/productor3.c -o bin/productor3
bin/sysv:
	$(CC) $(CFLAGS) src/sysv.c -o bin/sysv
obj/list_lib.o:
	$(CC) $(CFLAGS) -c src/list_lib.c -o obj/list_lib.o

clean:
	rm -f bin/*
	rm -f obj/*