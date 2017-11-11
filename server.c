#include <netdb.h>
#include "server.h"


int main(int argc, char** argv){
    int port;
    if (argc < 3 || argc > 4){
        printf("Error: wrong number of args\n");
        return 1;
    }
    if (argc == 4){
        port = atoi(argv[3]); //todo: check if atoi succeeded
    } else {
        port = DEFUALT_PORT;
    }
    if (makeUsersList(argv[1]) == ERR_RETURN_CODE){
        return ERR_RETURN_CODE;
    };
    char* dir_path = argv[2];
    if (openDirectories(dir_path) == ERR_RETURN_CODE){
        return ERR_RETURN_CODE;
    };

    int sock;
    struct addrinfo hints, *servinfo, *p;
    int res;
    char port_str[MAX_STR_LEN];
    sprintf(port_str, "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((res = getaddrinfo(NULL, port_str, &hints, &servinfo)) != 0) {
        printf("ERRRRRROROROR"); //TODO
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            printf("EROROROROR"); //TODO
            continue;
        }
        if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            printf("ERORORORO"); //TODO
            continue;
        }
        break; // if successful, stop the loop.
    }

    if (p == NULL) {
        printf("Failed to bind socket");
        return ERR_RETURN_CODE;
    }

    freeaddrinfo(servinfo);
    char buffer[MAX_FILE_LENGTH];
    ssize_t sent_num;

    listen(sock, 1); //TODO: change 1 to many?

    int new_sock;
    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    addr_size = sizeof(client_addr);

    while (1){

        new_sock = accept(sock, (struct sockaddr *) &client_addr, &addr_size);

        sent_num = recv(new_sock, buffer, BUFFER_SIZE, 0);
        buffer[sent_num] = '\0';
        printf("buffer received: %s\n", buffer);
        if (sent_num == 0) {
            printf("Received 0");
            break;
        }

        //listen for a client request, and handle when arrives (print welcome, check UN+pass,
        // if correct print client name and num of files).
        // start another loop handling client commands, untill client quits.
    }

    //TODO: delete all created folders on exit.

    return 0;
}

int makeUsersList(char* userFilePath){
    FILE* users_file;
    if ((users_file = fopen(userFilePath,"r")) == NULL){
        strerror(errno);
        return ERR_RETURN_CODE;
    }
    User user;
    char str[2*MAX_NAME_LEN +2];
    int i = 0;
    while (fgets(str,2*MAX_NAME_LEN +2,users_file) != NULL){
        strcpy(user.username, strtok(str,"\t"));
        strcpy(user.password, strtok(NULL,"\t"));
        users[i] = user;
        i++;
    }
    numUsers = i;
    return SUCCES_RETURN_CODE;
}

int openDirectories(char* dirpath){
    for (int i=0; i< numUsers; i++){
        char tempstr[MAX_DIRPATH_LEN];
        strcpy(tempstr, dirpath);
        const char* directoryToOpen = (const char*) strcat(tempstr,users[i].username);
        // TODO: check if directoryToOpen exosts in dirpath, if so continue.
        if (mkdir(directoryToOpen, 0777) != 0){
            return ERR_RETURN_CODE;
        };
    }
    return SUCCES_RETURN_CODE;
}