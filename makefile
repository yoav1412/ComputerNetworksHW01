all: client server

client: auc.h client.h client.c
    gcc -Wall -g -o file_client client.c

server: aux.h server.h server.c
    gcc -Wall -g -o file_server server.c

clean:
    -rm *.o file_client file_server