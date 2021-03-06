#include "client.h"

int main(int argc, char** argv){
    int arg_to_int;
    char hostname[MAX_HOST_NAME] = DEFAULT_HOST_NAME;
    int port = DEFAULT_PORT;
    fd_set master;
    fd_set read_fds;

    if (argc > 3) {
        printf("Error: wrong number of arguments!\n");
        return ERR_RETURN_CODE;
    }

    if (argc >= 2) { // get hostname from argument
        strcpy(hostname, argv[1]);
    }
    if (argc == 3) { // get port from argument
        arg_to_int = atoi(argv[2]);
        if (arg_to_int == 0) {
            printf("Error: could not set port, check the argument and try again.");
            return ERR_RETURN_CODE;
        }
        port = arg_to_int;
    }

    // Prepare socket and necessary structs

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Error creating socket\n");
        return ERR_RETURN_CODE;
    }

    struct addrinfo hints, *servinfo, *p;
    char port_str[MAX_STR_LEN];
    sprintf(port_str, "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((getaddrinfo(hostname, port_str, &hints, &servinfo)) != 0) {
        printf("Error in getaddrinfo\n");
        return ERR_RETURN_CODE;
    }

    for (p = servinfo; p!= NULL; p = p->ai_next) {
        if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
            printf("Error connecting socket\n");
            continue;
        }
        break; // Connection successful
    }

    if (p == NULL) {
        printf("Error: failed to connect\n");
        return ERR_RETURN_CODE;
    }

    freeaddrinfo(servinfo);


    // Connection is successful, start communicating with client

    // Get "hello" message
    char input[MAX_STR_LEN];
    if (recvStr(sock, input) == ERR_RETURN_CODE)
        return ERR_RETURN_CODE;
    printf("%s", input);


    // Login screen
    bool login_successful = false;
    char username_input[MAX_NAME_LEN];
    char password_input[MAX_NAME_LEN];
    int msg;

    while (!login_successful) {
        // get username
        fgets(username_input,MAX_NAME_LEN, stdin);
        if (username_input == NULL || strncmp(username_input, "User: ", 6) != 0) {
            printf("error, please enter again your credentials.\n");
            continue;
        }
        strmove(username_input, username_input + 6);
        username_input[strlen(username_input) - 1] = '\0';

        // get password
        fgets(password_input,MAX_NAME_LEN, stdin);
        if (password_input == NULL || strncmp(password_input, "Password: ", 10) != 0) {
            printf("error, please enter again your credentials.\n");
            continue;
        }
        strmove(password_input, password_input + 10);
        password_input[strlen(password_input) - 1] = '\0';

        char credentials[CRED_LEN];
        strcpy(credentials, username_input);
        credentials[strlen(username_input)] = '\n';
        credentials[strlen(username_input)+1] = '\0';
        strcat(credentials, password_input);

        if (sendStr(sock, credentials) == ERR_RETURN_CODE)
            return ERR_RETURN_CODE;

        recv(sock, &msg, sizeof(int), 0);
        if (msg == LOGIN_SUCCESS_MSG) {
            login_successful = true;
        } else if ( msg == LOGIN_FAILED_MSG ) {
            printf("Wrong credentials, please try again.\n");
        }
    }

    // Logged in successfully. let's get the number of files in our folder
    int num_of_files_in_folder;
    recv(sock, &num_of_files_in_folder, sizeof(int), 0);
    printf("Hi %s, you have %d files stored.\n", username_input, num_of_files_in_folder);


    // While not quitting, keep the connection on and get commands from user

    bool quit = false;
    char usr_command_str[MAX_STR_LEN];

    // Prepare fd's set
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(sock, &master);
    FD_SET(STDIN, &master);
    int max_fd = sock > STDIN ? sock : STDIN;

    while (!quit) {

        read_fds = master;
        if (select(max_fd+1, &read_fds, NULL, NULL, NULL) == -1)
            return ERR_RETURN_CODE;

        // Check if there are pending messages from server
        if (FD_ISSET(sock, &read_fds)) {
            char *message_in = (char*)calloc(MAX_MSG_LEN, 1);
            if (recvStr(sock, message_in) == ERR_RETURN_CODE) {
                free(message_in);
                return ERR_RETURN_CODE;
            }
            printf("%s", message_in);
            free(message_in);
        }

        // Get command from user if there is one
        if (FD_ISSET(STDIN, &read_fds)) {
            fgets(usr_command_str, MAX_STR_LEN, stdin);
            processClientCommand(usr_command_str, sock, &quit);
        }
    }

    close(sock);
    return SUCCESS_RETURN_CODE;
}

void checkPathFormat(char* path){
    if (path[strlen(path)-1] == '/'){
        return;
    }
    strcat(path, "/");
}

int processClientCommand(char* usr_command_str, int sock, bool *quit) {
    int server_response;
    char* file_data;

    // list_of_files
    if (strcmp(usr_command_str, "list_of_files\n") == 0) {
        char* list_of_files = (char*)calloc(MAX_NUM_OF_FILES*MAX_NAME_LEN, sizeof(char));
        send(sock, &LIST_OF_FILES_CMND, sizeof(int), 0);
        if (recvStr(sock, list_of_files) == ERR_RETURN_CODE)
            return ERR_RETURN_CODE;
        printf("%s", list_of_files);
        free(list_of_files);

    // delete_file
    } else if (strncmp(usr_command_str, "delete_file ", strlen("delete_file ")) == 0) {
        send(sock, &DELETE_FILE_CMND, sizeof(int), 0);
        if (sendStr(sock, usr_command_str + strlen("delete_file ")) == ERR_RETURN_CODE)
            return ERR_RETURN_CODE;

        recv(sock, &server_response, sizeof(int), 0);
        if (server_response == OPERATION_SUCCESSFUL)
            printf("File removed.\n");
        else
            printf("No such file exists!\n");

    // add_file
    } else if (strncmp(usr_command_str, "add_file ",strlen("add_file ")) == 0) {
        char arguments[MAX_DIRPATH_LEN + MAX_NAME_LEN];
        char path_to_file[MAX_DIRPATH_LEN];
        char file_name[MAX_NAME_LEN];
        strcpy(arguments, usr_command_str + strlen("add_file "));
        strcpy(path_to_file, strtok(arguments, " "));

        char* filename_arg;
        if ((filename_arg = strtok(NULL, "\n")) == NULL){
            printf("Error: wrong arguments.\n");
            return NO_COMMAND_EXECUTED;
        }
        strcpy(file_name, filename_arg);
        file_data = fileToStr(path_to_file);
        if (file_data == NULL) {
            printf("Error: wrong arguments.\n");
            return NO_COMMAND_EXECUTED;
        }

        send(sock, &ADD_FILE_CMND, sizeof(int), 0);
        if (sendStr(sock, file_name) == ERR_RETURN_CODE) // Send required file name
            return ERR_RETURN_CODE;
        if (sendStr(sock, file_data) == ERR_RETURN_CODE) // Send the file data
            return ERR_RETURN_CODE;
        free(file_data);

        recv(sock, &server_response, sizeof(int), 0);
        if (server_response == OPERATION_SUCCESSFUL)
            printf("File added.\n");
        else
            printf("Error adding file.\n");

    // get_file
    } else if (strncmp(usr_command_str, "get_file ",strlen("get_file ")) == 0) {
        char arguments[MAX_DIRPATH_LEN + MAX_NAME_LEN];
        char file_name[MAX_NAME_LEN];
        strcpy(arguments, usr_command_str + strlen("get_file "));
        strcpy(file_name, strtok(arguments, " "));

        char *path_arg;
        if ((path_arg = strtok(NULL, "\n")) == NULL) {
            printf("Error: wrong arguments.\n");
            return NO_COMMAND_EXECUTED;
        }
        checkPathFormat(path_arg);
        send(sock, &GET_FILE_CMND, sizeof(int), 0);
        if (sendStr(sock, file_name) == ERR_RETURN_CODE) // Send required file name
            return ERR_RETURN_CODE;

        recv(sock, &server_response, sizeof(int), 0);
        if (server_response == OPERATION_FAILED) {
            printf("Error while reading file in server.");
        }

        char data[MAX_FILE_LENGTH];
        if (recvStr(sock, data) == ERR_RETURN_CODE)
            return ERR_RETURN_CODE;
        strcat(path_arg, file_name);
        if (saveDataToFile(data, path_arg) == OPERATION_FAILED)
            printf("Error saving file.\n");
    // msg
    } else if (strncmp(usr_command_str, "msg ",strlen("msg ")) == 0) {
        char arguments[MAX_NAME_LEN + MAX_MSG_LEN + 2];
        char dest_user_name[MAX_NAME_LEN];
        char *msg;

        strcpy(arguments, usr_command_str + strlen("msg "));
        strcpy(dest_user_name, strtok(arguments, ":"));

        if ((msg = (strtok(NULL, "\n")) + 1) == NULL) {
            printf("Error: wrong arguments.\n");
            return NO_COMMAND_EXECUTED;
        }

        send(sock, &MSG_CMND, sizeof(int), 0);
        if (sendStr(sock, dest_user_name) == ERR_RETURN_CODE) // Send destination user name
            return ERR_RETURN_CODE;
        if (sendStr(sock, msg) == ERR_RETURN_CODE) // Send message
            return ERR_RETURN_CODE;
    // read_msgs
    } else if (strncmp(usr_command_str, "read_msgs\n",strlen("read_msgs\n")) == 0) {
        send(sock, &READ_CMND, sizeof(int), 0);

        // Get all messages one by one and print them
        char message[FINAL_MSG_LEN];
        memset(message, '\0', FINAL_MSG_LEN);
        while (strncmp(message, EOF_MSGS_FILE, strlen(EOF_MSGS_FILE)) != 0) {
            if (recvStr(sock, message) == ERR_RETURN_CODE)
                return ERR_RETURN_CODE;
            if (strncmp(message, EOF_MSGS_FILE, strlen(EOF_MSGS_FILE)) != 0)
                printf("%s", message);
        }
    // users_online
    } else if (strncmp(usr_command_str, "users_online\n",strlen("users_online\n")) == 0) {
        send(sock, &ONLINE_USRS_CMND, sizeof(int), 0);
        char message[MAX_NUM_USERS*(MAX_NAME_LEN + 1)];
        memset(message, '\0', MAX_NUM_USERS*(MAX_NAME_LEN + 1));

        if (recvStr(sock, message) == ERR_RETURN_CODE)
            return ERR_RETURN_CODE;

        printf("online users: %s\n", message);

        // quit
    } else if (strcmp(usr_command_str, "quit\n") == 0) {
        send(sock, &QUIT_CMND, sizeof(int), 0);
        *quit = true;

    // other illegal command
    } else {
        printf("Illegal command, please try again.\n");
        return NO_COMMAND_EXECUTED;
    }

    return COMMAND_EXECUTED;
}