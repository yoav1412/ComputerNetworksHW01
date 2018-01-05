#include "auxiliary.h"

#define DEFAULT_HOST_NAME "localhost"
#define STDIN 0

void checkPathFormat(char* path);
int processClientCommand(char* usr_command_str, int sock, bool *quit);