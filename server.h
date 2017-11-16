#include "auxiliary.h"

#define BACKLOG_SIZE 5
char HELLO_STR[MAX_STR_LEN] = "Welcome! Please log in.\n";

typedef struct User {
    char username[MAX_NAME_LEN];
    char password[MAX_NAME_LEN];
    char folder_path[MAX_DIRPATH_LEN];
} User;

User users[MAX_NUM_USERS];
int numUsers = 0;

int getNumOfFiles(User user);
int makeUsersList(char* userFilePath);
bool folderExists(char* dirpath);
int openDirectories(char* dirpath);
bool checkCredentials(User usr_from_client, User **logged_usr);
char* getListOfFiles(char folder_path[MAX_DIRPATH_LEN]);
int deleteFile(char file_path[MAX_NAME_LEN]);
