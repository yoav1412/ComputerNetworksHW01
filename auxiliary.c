
#include "auxiliary.h"

int LIST_OF_FILES_CMND = 1;
int DELETE_FILE_CMND = 2;
int ADD_FILE_CMND = 3;
int GET_FILE_CMND = 4;
int QUIT_CMND = 5;

int OPERATION_SUCCESSFUL = 1;
int OPERATION_FAILED = -1;

int LOGIN_FAILED_MSG = -1;
int LOGIN_SUCCESS_MSG = 1;


char* strmove(char* str1, char* str2){
    size_t len = strlen(str2);
    memmove(str1, str2, len+1);
    return str1;
}

char* fileToStr(char file_path[MAX_DIRPATH_LEN + MAX_NAME_LEN]) {
    char* file_text = (char*)calloc(MAX_FILE_LENGTH, sizeof(char));
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL)
        return NULL;

    size_t num_read = fread(file_text, sizeof(char), MAX_FILE_LENGTH, fp);
    if (num_read < 1)
        return NULL;

    return file_text;
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
    chmod(path_to_save, 0777);
    return OPERATION_SUCCESSFUL;
}

int sendStr(int sock, char* str) {
    int buffer_size = strlen(str) + 1;
    if (send(sock, &buffer_size, sizeof(int), 0) == -1)
        return ERR_RETURN_CODE;
    if (send(sock, str, buffer_size * sizeof(char), 0) == -1)
        return ERR_RETURN_CODE;

    return SUCCESS_RETURN_CODE;
}

int recvStr(int sock, char* str) {
    int buffer_size;
    if (recv(sock, &buffer_size, sizeof(int), 0) == -1)
        return ERR_RETURN_CODE;
    if (recv(sock, str, buffer_size * sizeof(char), 0) == -1)
        return ERR_RETURN_CODE;

    return SUCCESS_RETURN_CODE;
}