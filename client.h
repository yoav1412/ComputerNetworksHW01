#include "aux.h"

#define DEFAULT_HOST_NAME "localhost"

int saveDataToFile(char data[MAX_FILE_LENGTH], char path_to_save[MAX_DIRPATH_LEN + MAX_NAME_LEN]);
char* fileToStr(char file_path[MAX_DIRPATH_LEN + MAX_NAME_LEN]);