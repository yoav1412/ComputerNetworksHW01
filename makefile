all: client server

client: auxiliary.h auxiliary.c client.h client.c
	gcc -Wall -g -o file_client auxiliary.c client.c

server: auxiliary.h auxiliary.c server.h server.c
	gcc -Wall -g -o file_server auxiliary.c server.c

clean:
	-rm -f *.o file_client file_server