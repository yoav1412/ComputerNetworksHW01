#include <direct.h>
#include "server.h"



int mainServer(int argc, char** argv){
    if (argc < 3 || argc > 4){
        printf("Error: wrong number of args\n");
        return 1;
    }
    if (argc == 4){
        int port = atoi(argv[3]); //todo: check if atoi succeeded
    } else {
        int port = DEFUALT_PORT;
    }
    if (makeUsersList(argv[1]) == ERR_RETURN_CODE){
        return ERR_RETURN_CODE;
    };
    char* dir_path = argv[2];
    if (openDirectories(dir_path) == ERR_RETURN_CODE){
        return ERR_RETURN_CODE;
    };
    while (1){
        //listen for a client request, and handle when arrives (print welcome, check UN+pass,
        // if correct print client name and num of files).
        // start another loop handling client commands, untill client quits.
    }

    //TODO: delete all created folders on exit.
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
        if (mkdir(directoryToOpen) != 0){
            return ERR_RETURN_CODE;
        };
    }
    return SUCCES_RETURN_CODE;
}