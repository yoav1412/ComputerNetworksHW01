#include "auxiliary.h"

#define BACKLOG_SIZE 5
#define MESSAGE_FILE "/Messages_received_offline.txt"
char HELLO_STR[MAX_STR_LEN] = "Welcome! Please log in.\n";

typedef struct User {
    char username[MAX_NAME_LEN];
    char password[MAX_NAME_LEN];
    char folder_path[MAX_DIRPATH_LEN];
    int sock_fd;
    bool logged_in;
} User;

User users[MAX_NUM_USERS];
int numUsers = 0;
int max_fd = 0;

int getNumOfFiles(User user);
int makeUsersList(char* userFilePath);
bool folderExists(char* dirpath);
int openDirectories(char* dirpath);
bool checkCredentials(User usr_from_client, User **logged_usr);
char* getListOfFiles(char folder_path[MAX_DIRPATH_LEN]);
int deleteFile(char file_path[MAX_NAME_LEN]);
void updateMax(int new_fd);
bool isFdLoggedIn(int fd);
int recvCredentials(int new_sock, User *usr_from_client);
User *getUserFromFd(int fd);
int processCommand(int usr_command, User *logged_usr, int fd);
void formatMessage(char *user_name, char *message, bool online, char *output);
User *getUserByName(char* user_name);
int addToMessageFile(User *dest_usr, char *final_message);
void getOnlineUsersString(char *output);