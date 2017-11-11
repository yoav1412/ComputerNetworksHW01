#include "client.h"

int main(int argc, char** argv){
    if (argc > 2){
        printf("Error: wrong number of args!\n");
        return ERR_RETURN_CODE;
    }
    char hostname[MAX_HOST_NAME] = DEFAULT_HOST_NAME;
    int port = DEFUALT_PORT;
    if (argc == 2){
        strcpy(hostname,strtok(argv[1]," "));
        char* portStr = strtok(NULL," ");
        if (portStr == NULL){
            printf("Error: wrong number of args!\n");
            return ERR_RETURN_CODE;
        }
        port = atoi(portStr);
    }

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return ERR_RETURN_CODE;
    }

//    struct sockaddr_in dest_address;
    struct addrinfo hints, *servinfo, *p;
    int res;
    char port_str[MAX_STR_LEN];
    sprintf(port_str, "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;

//    dest_address.sin_family = AF_INET;
//    dest_address.sin_port = htons((uint16_t) port);
    if ((res = getaddrinfo(hostname, port_str, &hints, &servinfo)) != 0) {
        printf("EROROROROROR"); //TODO
    }

    for (p = servinfo; p!= NULL; p = p->ai_next) {
        if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
            printf("EROROROROR"); //TODO
            continue;
        }
        break; // Connection succesful
    }

    if (p == NULL) {
        printf("Error: failed to connect\n");
        return ERR_RETURN_CODE;
    }

    freeaddrinfo(servinfo);

    char buffer[MAX_FILE_LENGTH] = "abc"; //TODO: change to const
    send(sock, buffer, BUFFER_SIZE, 0);

    close(sock);
}