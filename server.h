#include "auxiliary.h"

#define HELLO_STR "Welcome! Please log in.\n"
#define BACKLOG_SIZE 5


typedef struct User {
    char username[MAX_NAME_LEN];
    char password[MAX_NAME_LEN];
    char folder_path[MAX_DIRPATH_LEN];
    int num_of_files;
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
int saveDataToFile(char data[MAX_FILE_LENGTH], char path_to_save[MAX_DIRPATH_LEN + MAX_NAME_LEN]);
char* fileToStr(char file_path[MAX_DIRPATH_LEN + MAX_NAME_LEN]);