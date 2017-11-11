//
// Created by Yoav on 09-Nov-17.
//


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>


#define MAX_NAME_LEN 25
#define MAX_DIRPATH_LEN 100 //TODO: add this const to documemnt.
#define MAX_NUM_USERS 15
#define MAX_STR_LEN 64
#define BUFFER_SIZE 40
#define MAX_FILE_LENGTH 2048
#define SUCCES_RETURN_CODE 0
#define ERR_RETURN_CODE -1
#define MAX_HOST_NAME 50
#define DEFUALT_PORT 1337
#define DEFAULT_HOST_NAME "localhost"