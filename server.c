#include <netdb.h>
#include <stdbool.h>
#include "server.h"


int main(int argc, char** argv){
    int port = DEFAULT_PORT;
    int input_port;
    if (argc < 3 || argc > 4){
        printf("Error: wrong number of args\n");
        return 1;
    }
    if (argc == 4) {
        input_port = atoi(argv[3]);
        if (input_port == 0)
            printf("Error: setting of port number failed, continuing with default value %d\n", port);
        else
            port = input_port;
    }

    if (makeUsersList(argv[1]) == ERR_RETURN_CODE){
        return ERR_RETURN_CODE;
    };
    char* dir_path = argv[2];
    if (openDirectories(dir_path) == ERR_RETURN_CODE){
        return ERR_RETURN_CODE;
    };


    //Open socket and listen

    int sock = 0;
    struct addrinfo hints, *servinfo, *p;
    char port_str[MAX_STR_LEN];
    sprintf(port_str, "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((getaddrinfo(NULL, port_str, &hints, &servinfo)) != 0) {
        printf("Error in getaddrinfo\n");
        return ERR_RETURN_CODE;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        int enable = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int) );
        if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            continue;
        }
        break; // if successful, stop the loop.
    }

    if (p == NULL) {
        printf("Failed to connect.\n");
        return ERR_RETURN_CODE;
    }

    freeaddrinfo(servinfo);

    listen(sock, BACKLOG_SIZE);

    int new_sock;
    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    addr_size = sizeof(client_addr);

    bool logged_in = false;
    bool quit = false;
    User usr_from_client;
    User *logged_usr;

    while (1) {

        new_sock = accept(sock, (struct sockaddr *) &client_addr, &addr_size);

        // Send hello message to client
        if (sendStr(new_sock, HELLO_STR) == ERR_RETURN_CODE)
            return ERR_RETURN_CODE;

        logged_in = false;
        quit = false;

        // Login phase
        while (!quit && !logged_in) {
            if (recvStr(new_sock, usr_from_client.username) == ERR_RETURN_CODE)
                return ERR_RETURN_CODE;
            if (recvStr(new_sock, usr_from_client.password) == ERR_RETURN_CODE)
                return ERR_RETURN_CODE;

            if (checkCredentials(usr_from_client, &logged_usr)) {
                logged_in = true;
                send(new_sock, &LOGIN_SUCCESS_MSG, sizeof(int), 0);
            } else {
                send(new_sock, &LOGIN_FAILED_MSG, sizeof(int), 0);
            }
        }

        // Logged in successfully.
        // Send number of files for the logged in user
        int user_num_files = getNumOfFiles(*logged_usr);
        send(new_sock, &user_num_files, sizeof(int), 0);

        int usr_command;
        char file_name[MAX_NAME_LEN];
        char file_data[MAX_FILE_LENGTH];

        // Get commands from user until it quits

        while (!quit) {
            recv(new_sock, &usr_command, sizeof(int), 0);

            if (usr_command == QUIT_CMND) {
                quit = true;

            } else if (usr_command == LIST_OF_FILES_CMND) {
                char* files_list;
                files_list = getListOfFiles(logged_usr->folder_path);
                if (sendStr(new_sock, files_list) == ERR_RETURN_CODE)
                    return ERR_RETURN_CODE;
                free(files_list);

            } else if (usr_command == DELETE_FILE_CMND) {
                if (recvStr(new_sock, file_name) == ERR_RETURN_CODE) // receive the argument: file name
                    return ERR_RETURN_CODE;

                file_name[strlen(file_name)-1] = '\0';
                char full_file_path[MAX_DIRPATH_LEN + MAX_NAME_LEN];
                strcpy(full_file_path, logged_usr->folder_path);
                strcat(full_file_path, "/");
                strcat(full_file_path, file_name);

                int deleted = deleteFile(full_file_path);
                send(new_sock, &deleted, sizeof(int), 0);

            } else if (usr_command == ADD_FILE_CMND) {
                if (recvStr(new_sock, file_name) == ERR_RETURN_CODE) // Get required file name
                    return ERR_RETURN_CODE;
                if (recvStr(new_sock, file_data) == ERR_RETURN_CODE) // Get the file data
                    return ERR_RETURN_CODE;

                char full_file_path[MAX_DIRPATH_LEN + MAX_NAME_LEN];
                strcpy(full_file_path, logged_usr->folder_path);
                strcat(full_file_path, "/");
                strcat(full_file_path, file_name);

                int created = saveDataToFile(file_data, full_file_path);
                send(new_sock, &created, sizeof(int), 0);

            } else if (usr_command == GET_FILE_CMND) {
                if (recvStr(new_sock, file_name) == ERR_RETURN_CODE) // Get required file name
                    return ERR_RETURN_CODE;
                char full_file_path[MAX_DIRPATH_LEN + MAX_NAME_LEN];
                strcpy(full_file_path, logged_usr->folder_path);
                strcat(full_file_path, "/");
                strcat(full_file_path, file_name);

                char* data;
                int successful = OPERATION_SUCCESSFUL;
                data = fileToStr(full_file_path);
                if (data == NULL) {
                    successful = OPERATION_FAILED;
                    send(new_sock, &successful, sizeof(int), 0);
                    continue;
                }
                send(new_sock, &successful, sizeof(int), 0);
                if (sendStr(new_sock, data) == ERR_RETURN_CODE)
                    return ERR_RETURN_CODE;
                free(data);

            } else if (usr_command == QUIT_CMND) {
                close(new_sock);
                quit = true;
            }
        }

    }

    return SUCCESS_RETURN_CODE;
}

int getNumOfFiles(User user) {
    DIR* dirp;
    struct dirent* entry;
    int num_of_files = 0;

    dirp = opendir(user.folder_path);
    if (dirp == NULL)
        return 0;

    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG){
            num_of_files++;
        }
    }

    closedir(dirp);

    return num_of_files;

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
    while (fgets(str,2*MAX_NAME_LEN +2,users_file) != NULL) {
        strcpy(user.username, strtok(str,"\t"));
        strcpy(user.password, strtok(NULL,"\n"));
        users[i] = user;
        i++;
    }
    numUsers = i;
    return SUCCESS_RETURN_CODE;
}

int openDirectories(char* dirpath) {
    int i;
    for (i=0; i< numUsers; i++) {
        char tempstr[MAX_DIRPATH_LEN];
        strcpy(tempstr, dirpath);
        char* directoryToOpen = strcat(tempstr,users[i].username);
        strcpy(users[i].folder_path, directoryToOpen);
        if (folderExists(directoryToOpen)) { // If exists
            continue;
        }
        if (mkdir(directoryToOpen, 0777) != 0) {
            return ERR_RETURN_CODE;
        };
    }
    return SUCCESS_RETURN_CODE;
}

bool folderExists(char* dirpath) {
    DIR* dir = opendir(dirpath);
    if (dir != NULL) {
        closedir(dir);
        return true;
    }
    return false;
}

bool checkCredentials(User usr_from_client, User **logged_usr) {
    int i;
    for (i=0; i < numUsers; i++ ){
        if ((strcmp(users[i].username,usr_from_client.username) == 0) && (strcmp(users[i].password, usr_from_client.password) == 0)) {
            *logged_usr = &(users[i]);
            return true;
        }
    }
    return false;
}

char* getListOfFiles(char folder_path[MAX_DIRPATH_LEN]) {
    DIR* dirp;
    struct dirent* entry;
    char* list_of_files = (char*)calloc(sizeof(char)*MAX_NAME_LEN*MAX_NUM_OF_FILES, sizeof(char));

    dirp = opendir(folder_path);
    if (dirp == NULL)
        return NULL;

    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG){
            strcat(list_of_files, strcat(entry->d_name, "\n"));
        }
    }

    closedir(dirp);

    return list_of_files;
}

int deleteFile(char file_path[MAX_NAME_LEN]) {
    if (remove(file_path) != 0)
        return OPERATION_FAILED;
    return OPERATION_SUCCESSFUL;
}

