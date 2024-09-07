#include "shell.h"

void handlePwdCommand()
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        std::cout << cwd << std::endl;
    }
    else
    {
        perror("getcwd");
    }
}