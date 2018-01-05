#include <netdb.h>
#include <stdbool.h>
#include "server.h"


int main(int argc, char** argv){
    int port = DEFAULT_PORT;
    int input_port;
    int usr_command;
    int retval;
    fd_set master;
    fd_set read_fds;
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

    int listener_sock = 0;
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
        if ((listener_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        int enable = 1;
        setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int) );
        if (bind(listener_sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(listener_sock);
            continue;
        }
        break; // if successful, stop the loop.
    }

    if (p == NULL) {
        printf("Failed to connect.\n");
        return ERR_RETURN_CODE;
    }

    freeaddrinfo(servinfo);

    listen(listener_sock, BACKLOG_SIZE);

    int new_sock = 0;
    struct sockaddr_storage client_addr;
    socklen_t addr_size;

    User usr_from_client;
    User *logged_usr;

    // Prepare fd's set
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(listener_sock, &master);
    updateMax(listener_sock);

    while (1) {
        read_fds = master;
        if (select(max_fd+1, &read_fds, NULL, NULL, NULL) == -1)
            return ERR_RETURN_CODE;

        for (int i = 0; i <= max_fd; i++){
            // Find ready fd's
            if (FD_ISSET(i, &read_fds)) {

                // If this is the listener socket
                if (i == listener_sock) {
                    // Register new socket for this client
                    addr_size = sizeof(client_addr);
                    new_sock = accept(listener_sock, (struct sockaddr *) &client_addr, &addr_size);
                    if (new_sock == -1) return ERR_RETURN_CODE;
                    FD_SET(new_sock, &master);
                    updateMax(new_sock);

                    // Send hello message to client
                    if (sendStr(new_sock, HELLO_STR) == ERR_RETURN_CODE)
                        return ERR_RETURN_CODE;
                }

                // If this is a client in the login sequence
                else if (i != listener_sock && !isFdLoggedIn(i)) {
                    if (recvCredentials(i, &usr_from_client) == ERR_RETURN_CODE)
                        return ERR_RETURN_CODE;

                    if (checkCredentials(usr_from_client, &logged_usr)) {
                        // Logged in successfully.
                        logged_usr->logged_in = true;
                        logged_usr->sock_fd = i;
                        send(i, &LOGIN_SUCCESS_MSG, sizeof(int), 0);

                        // Send number of files for the logged in user
                        int user_num_files = getNumOfFiles(*logged_usr);
                        send(i, &user_num_files, sizeof(int), 0);
                    } else {
                        send(i, &LOGIN_FAILED_MSG, sizeof(int), 0);
                    }
                }

                // If this is an already logged in client
                else {
                    logged_usr = getUserFromFd(i);
                    if (logged_usr == NULL)
                        continue;

                    // Process user command
                    recv(i, &usr_command, sizeof(int), 0);
                    retval = processCommand(usr_command, logged_usr, i);

                    if (retval == ERR_RETURN_CODE)
                        return ERR_RETURN_CODE;

                    // If user quitted
                    if (retval == QUIT_CMND)
                        FD_CLR(i, &master);
                }
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
        user.logged_in = false;
        user.sock_fd = INVALID_FD;
        users[i] = user;
        i++;
    }
    numUsers = i;
    return SUCCESS_RETURN_CODE;
}

int openDirectories(char* dirpath) {
    int i;
    FILE *fp;
    for (i=0; i< numUsers; i++) {
        char tempstr[MAX_DIRPATH_LEN];
        strcpy(tempstr, dirpath);
        char* directoryToOpen = strcat(tempstr,users[i].username);
        strcpy(users[i].folder_path, directoryToOpen);
        if (folderExists(directoryToOpen)) { // If exists
            if ((fp = fopen(strcat(directoryToOpen,MESSAGE_FILE), "w")) == NULL)
                return ERR_RETURN_CODE;
            fclose(fp);
            continue;
        }

        // Make the directory and add a message buffer file
        if (mkdir(directoryToOpen, 0777) != 0) {
            return ERR_RETURN_CODE;
        }
        if ((fp = fopen(strcat(directoryToOpen,MESSAGE_FILE), "w")) == NULL)
            return ERR_RETURN_CODE;
        fclose(fp);
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

void updateMax(int new_fd) {
    max_fd = new_fd > max_fd ? new_fd : max_fd;
}

bool isFdLoggedIn(int fd) {
    User *user = getUserFromFd(fd);
    if (user != NULL)
        return user->logged_in;
    return false;
}

int recvCredentials(int new_sock, User *usr_from_client) {
    char buffer[CRED_LEN];
    if (recvStr(new_sock, buffer) == -1)
        return ERR_RETURN_CODE;
    char *username = strtok(buffer, "\n");
    char *password = strtok(NULL, "\n");

    strcpy(usr_from_client->username, username);
    strcpy(usr_from_client->password, password);
    return SUCCESS_RETURN_CODE;
}

User *getUserFromFd(int fd) {
    for (int i = 0; i < MAX_NUM_USERS; i++) {
        if (users[i].sock_fd == fd)
            return &(users[i]);
    }
    return NULL;
}

int processCommand(int usr_command, User *logged_usr, int fd) {
    char file_name[MAX_NAME_LEN];
    char file_data[MAX_FILE_LENGTH];

    if (usr_command == LIST_OF_FILES_CMND) {
        char* files_list;
        files_list = getListOfFiles(logged_usr->folder_path);
        if (sendStr(fd, files_list) == ERR_RETURN_CODE) {
            free(files_list);
            return ERR_RETURN_CODE;
        }
        free(files_list);
        return COMMAND_EXECUTED;
    }
    else if (usr_command == QUIT_CMND) {
        logged_usr->logged_in = false;
        logged_usr->sock_fd = INVALID_FD;
        close(fd);
        return QUIT_CMND;
    }
    else if (usr_command == DELETE_FILE_CMND) {
        if (recvStr(fd, file_name) == ERR_RETURN_CODE) // receive the argument: file name
            return ERR_RETURN_CODE;

        file_name[strlen(file_name)-1] = '\0';
        char full_file_path[MAX_DIRPATH_LEN + MAX_NAME_LEN];
        strcpy(full_file_path, logged_usr->folder_path);
        strcat(full_file_path, "/");
        strcat(full_file_path, file_name);

        int deleted = deleteFile(full_file_path);
        send(fd, &deleted, sizeof(int), 0);
        return COMMAND_EXECUTED;

    } else if (usr_command == ADD_FILE_CMND) {
        if (recvStr(fd, file_name) == ERR_RETURN_CODE) // Get required file name
            return ERR_RETURN_CODE;
        if (recvStr(fd, file_data) == ERR_RETURN_CODE) // Get the file data
            return ERR_RETURN_CODE;

        char full_file_path[MAX_DIRPATH_LEN + MAX_NAME_LEN];
        strcpy(full_file_path, logged_usr->folder_path);
        strcat(full_file_path, "/");
        strcat(full_file_path, file_name);

        int created = saveDataToFile(file_data, full_file_path);
        send(fd, &created, sizeof(int), 0);
        return COMMAND_EXECUTED;

    } else if (usr_command == GET_FILE_CMND) {
        if (recvStr(fd, file_name) == ERR_RETURN_CODE) // Get required file name
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
            send(fd, &successful, sizeof(int), 0);
            return COMMAND_EXECUTED;
        }
        send(fd, &successful, sizeof(int), 0);
        if (sendStr(fd, data) == ERR_RETURN_CODE) {
            free(data);
            return ERR_RETURN_CODE;
        }
        free(data);
        return COMMAND_EXECUTED;
    } else if (usr_command == MSG_CMND) {
        char user_name[MAX_NAME_LEN];
        char message[MAX_MSG_LEN];

        if (recvStr(fd, user_name) == ERR_RETURN_CODE) // Get required user name
            return ERR_RETURN_CODE;
        if (recvStr(fd, message) == ERR_RETURN_CODE) // Get required message
            return ERR_RETURN_CODE;

        char *final_message = (char*)calloc(FINAL_MSG_LEN, 1);
        User* dest_usr = getUserByName(user_name);
        bool online = isFdLoggedIn(dest_usr->sock_fd);
        formatMessage(logged_usr->username, message, online, final_message);
        if (online) {
            if (sendStr(dest_usr->sock_fd, final_message) == ERR_RETURN_CODE) {
                free(final_message);
                return ERR_RETURN_CODE;
            }
            free(final_message);
        }
        else {
            if (addToMessageFile(dest_usr, final_message) == ERR_RETURN_CODE) {
                free(final_message);
                return ERR_RETURN_CODE;
            }
            free(final_message);
        }
    } else if (usr_command == READ_CMND) {
        int curr_msg_size = FINAL_MSG_LEN;
        char *curr_message = (char *)malloc(curr_msg_size);
        char file_path[strlen(MESSAGE_FILE) + MAX_DIRPATH_LEN];
        strcpy(file_path, logged_usr->folder_path);
        FILE *fp = fopen(strcat(file_path, MESSAGE_FILE), "r");

        // Send messages line-by-line until reaching EOF
        while (fgets(curr_message, curr_msg_size + 1, fp) != NULL) {
            if (sendStr(fd, curr_message) == ERR_RETURN_CODE) {
                free(curr_message);
                fclose(fp);
                return ERR_RETURN_CODE;
            }
        }

        if (sendStr(fd, EOF_MSGS_FILE) == ERR_RETURN_CODE) {
            free(curr_message);
            fclose(fp);
            return ERR_RETURN_CODE;
        }

        fclose(fp);
        fclose(fopen(file_path, "w")); // Erase file content
        free(curr_message);

    } else if (usr_command == ONLINE_USRS_CMND) {
        char message[MAX_NUM_USERS*(MAX_NAME_LEN + 1)];
        memset(message, '\0', MAX_NUM_USERS*(MAX_NAME_LEN + 1));
        getOnlineUsersString(message);

        if (sendStr(fd, message) == ERR_RETURN_CODE) {
            return ERR_RETURN_CODE;
        }
    }

    return NO_COMMAND_EXECUTED;
}

void formatMessage(char *user_name, char *message, bool online, char* output) {
    if (online)
        strcat(output,"New message from ");
    else
        strcat(output, "Message received from ");
    strcat(output, user_name);
    strcat(output, ": ");
    strcat(output, message);
    strcat(output, "\n");
}

User *getUserByName(char* user_name) {
    for (int i=0; i < MAX_NUM_USERS; i++)
        if (strcmp(user_name, users[i].username) == 0)
            return &(users[i]);
    return NULL;
}

int addToMessageFile(User *dest_usr, char *final_message) {
    char file_path[strlen(MESSAGE_FILE) + MAX_DIRPATH_LEN];
    strcpy(file_path, dest_usr->folder_path);
    FILE *fp = fopen(strcat(file_path, MESSAGE_FILE), "a");
    if (fp == NULL)
        return ERR_RETURN_CODE;

    if (fwrite(final_message, sizeof(char), strlen(final_message), fp) < 0) {
        fclose(fp);
        return ERR_RETURN_CODE;
    }
    fclose(fp);
    return SUCCESS_RETURN_CODE;
};

void getOnlineUsersString(char *output) {
    bool isFirst = true;
    for (int i = 0; i < MAX_NUM_USERS; i++) {
        if (users[i].logged_in) {
            if (!isFirst) {
                strcat(output, ",");
            }
            strcat(output, users[i].username);
            isFirst = false;
        }
    }
}