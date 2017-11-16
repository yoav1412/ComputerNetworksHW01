#include "client.h"

int main(int argc, char** argv){
    int arg_to_int;
    char hostname[MAX_HOST_NAME] = DEFAULT_HOST_NAME;
    int port = DEFAULT_PORT;

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
            printf("Error cinnecting socket\n");
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
    recv(sock, input, MAX_STR_LEN, 0);
    printf("%s", input);


    // Login screen
    bool login_successful;
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
        strcpy(username_input, username_input + 6);
        username_input[strlen(username_input) - 1] = '\0';

        // get password
        fgets(password_input,MAX_NAME_LEN, stdin);
        if (password_input == NULL || strncmp(password_input, "Password: ", 10) != 0) {
            printf("error, please enter again your credentials.\n");
            continue;
        }
        strcpy(password_input, password_input + 10);
        password_input[strlen(password_input) - 1] = '\0';

        send(sock, username_input, MAX_NAME_LEN, 0);
        send(sock, password_input, MAX_NAME_LEN, 0);

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

    bool quit;
    char usr_command_str[MAX_STR_LEN];
    char list_of_files[MAX_NUM_OF_FILES*MAX_NAME_LEN];
    char* file_data;
    int server_response;

    while (!quit) {

        fgets(usr_command_str, MAX_STR_LEN, stdin);

        // list_of_files
        if (strcmp(usr_command_str, "list_of_files\n") == 0) {
            send(sock, &LIST_OF_FILES_CMND, sizeof(int), 0);
            recv(sock, &list_of_files, MAX_NUM_OF_FILES*MAX_NAME_LEN, 0);
            printf("%s", list_of_files);

        // delete_file
        } else if (strncmp(usr_command_str, "delete_file ", strlen("delete_file ")) == 0) {
            send(sock, &DELETE_FILE_CMND, sizeof(int), 0);
            send(sock, usr_command_str + strlen("delete_file "), MAX_STR_LEN, 0);

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
                continue;
            }
            strcpy(file_name, filename_arg);
            file_data = fileToStr(path_to_file);
            if (file_data == NULL) {
                printf("Error: wrong arguments.\n");
                continue;
            }

            send(sock, &ADD_FILE_CMND, sizeof(int), 0);
            send(sock, file_name, sizeof(file_name), 0); // Send required file name
            send(sock, file_data, MAX_FILE_LENGTH, 0); // Send the file data
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

            char* path_arg;
            if ((path_arg = strtok(NULL, "\n")) == NULL){
                printf("Error: wrong arguments.\n");
                continue;
            }
            checkPathFormat(path_arg);
            send(sock, &GET_FILE_CMND, sizeof(int), 0);
            send(sock, file_name, sizeof(file_name), 0); // Send required file name

            recv(sock, &server_response, sizeof(int), 0);
            if (server_response == OPERATION_FAILED) {
                printf("Error while reading file in server.");
                continue;
            }

            char data[MAX_FILE_LENGTH];
            recv(sock, data, MAX_FILE_LENGTH, 0);
            strcat(path_arg, file_name);
            if (saveDataToFile(data, path_arg) == OPERATION_FAILED)
                printf("Error saving file.\n");

        // quit
        } else if (strcmp(usr_command_str, "quit\n") == 0) {
            send(sock, &QUIT_CMND, sizeof(int), 0);
            quit = true;

        // other illegal command
        } else {
            printf("Illegal command, please try again.\n");
        }
    }

    close(sock);
    return SUCCESS_RETURN_CODE;
}

int saveDataToFile(char data[MAX_FILE_LENGTH], char path_to_save[MAX_DIRPATH_LEN + MAX_NAME_LEN]) {
    FILE* opf;
    opf = fopen(path_to_save, "w");
    if (opf == NULL)
        return OPERATION_FAILED;

    if (fprintf(opf, "%s", data) < 0) {
        fclose(opf);
        return OPERATION_FAILED;
    }

    fclose(opf);
    return OPERATION_SUCCESSFUL;
}


char* fileToStr(char file_path[MAX_DIRPATH_LEN + MAX_NAME_LEN]) {
    char* file_text = (char*)malloc(sizeof(char)*MAX_FILE_LENGTH);
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL)
        return NULL;

    size_t num_read = fread(file_text, sizeof(char), MAX_FILE_LENGTH, fp);
    if (num_read < 1)
        return NULL;

    return file_text;
}

void checkPathFormat(char* path){
    if (path[strlen(path)-1] == '/'){
        return;
    }
    strcat(path, "/");
}