all: client server

client: auxiliary.h client.h client.c
	gcc -Wall -g -o file_client client.c

server: auxiliary.h server.h server.c
	gcc -Wall -g -o file_server server.c

clean:
	-rm -f *.o file_client file_server