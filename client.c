//
// Created by Yoav on 09-Nov-17.
//
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
    int sock = (int) socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1){
        return ERR_RETURN_CODE;
    }


    close(sock);
}