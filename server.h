//
// Created by Yoav on 09-Nov-17.
//

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include <sys/stat.h>


#define MAX_NAME_LEN 25
#define MAX_DIRPATH_LEN 100 //TODO: add this const to documemnt.
#define MAX_NUM_USERS 15
#define SUCCES_RETURN_CODE 0
#define ERR_RETURN_CODE -1
#define DEFUALT_PORT 1337



typedef struct User {
   char username[MAX_NAME_LEN];
   char password[MAX_NAME_LEN];
} User;

User users[MAX_NUM_USERS];
int numUsers = 0;


int makeUsersList(char* userFilePath);
int openDirectories(char* dirpath);