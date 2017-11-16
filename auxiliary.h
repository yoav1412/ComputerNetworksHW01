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

#define SUCCESS_RETURN_CODE 0
#define ERR_RETURN_CODE -1

#define DEFAULT_PORT 1337
#define MAX_STR_LEN 64

int LIST_OF_FILES_CMND = 1;
int DELETE_FILE_CMND = 2;
int ADD_FILE_CMND = 3;
int GET_FILE_CMND = 4;
int QUIT_CMND = 5;

int OPERATION_SUCCESSFUL = 1;
int OPERATION_FAILED = -1;

int LOGIN_FAILED_MSG = -1;
int LOGIN_SUCCESS_MSG = 1;


