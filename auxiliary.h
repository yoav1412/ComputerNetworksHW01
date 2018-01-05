#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <dirent.h>

#define MAX_HOST_NAME 80
#define MAX_NAME_LEN 25
#define MAX_DIRPATH_LEN 100
#define MAX_NUM_OF_FILES 15
#define MAX_NUM_USERS 15
#define MAX_FILE_LENGTH 2048
#define CRED_LEN 2*MAX_NAME_LEN + 3

#define SUCCESS_RETURN_CODE 0
#define ERR_RETURN_CODE -1
#define INVALID_FD -1
#define COMMAND_EXECUTED 1
#define NO_COMMAND_EXECUTED 0
#define EOF_MSGS_FILE "FINISHED"

#define DEFAULT_PORT 1337
#define MAX_STR_LEN 64
#define MAX_MSG_LEN 100
#define FINAL_MSG_LEN strlen("Message received from ") + MAX_MSG_LEN + MAX_NAME_LEN + 3

extern int LIST_OF_FILES_CMND;
extern int DELETE_FILE_CMND;
extern int ADD_FILE_CMND;
extern int GET_FILE_CMND;
extern int MSG_CMND;
extern int READ_CMND;
extern int ONLINE_USRS_CMND;
extern int QUIT_CMND;

extern int OPERATION_SUCCESSFUL;
extern int OPERATION_FAILED;

extern int LOGIN_FAILED_MSG;
extern int LOGIN_SUCCESS_MSG;

char* strmove(char* str1, char* str2);
char* fileToStr(char file_path[MAX_DIRPATH_LEN + MAX_NAME_LEN]);
int saveDataToFile(char data[MAX_FILE_LENGTH], char path_to_save[MAX_DIRPATH_LEN + MAX_NAME_LEN]);
int sendStr(int sock, char* str);
int recvStr(int sock, char* str);
