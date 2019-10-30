CC=gcc
CFLAGS=-I.

client: client.c requests.c helpers.c
		$(CC) -o client client.c requests.c helpers.c parson.c -Wall -Wextra -g
		
run: client
		./client

clean:
		rm -f *.o client
