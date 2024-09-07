#include "shell.h"

void handleCdCommand(string token)
{
    if (token == "..")
    {
        char cwd[1024];
        getcwd(cwd, 1024);
        prevDir = cwd;
        chdir("..");
    }
    else if (token == ".")
    {
        char cwd[1024];
        getcwd(cwd, 1024);
        prevDir = cwd;
    }
    else if (token == "-")
    {
        if (!prevDir.empty())
        {
            char oldCwd[1024];
            getcwd(oldCwd, 1024);
            if (chdir(prevDir.c_str()) == 0)
            {
                prevDir = oldCwd;
            }
            else
            {
                perror("cd");
            }
        }
        else
        {
            cerr << "cd: OLDPWD not set\n";
        }
    }
    else if (token == "~")
    {
        char cwd[1024];
        getcwd(cwd, 1024);
        prevDir = cwd;
        if (chdir(homeDir.c_str()) != 0)
        {
            perror("cd");
        }
    }
    else if (token[0] == '/')
    {
        char cwd[1024];
        getcwd(cwd, 1024);
        prevDir = cwd;
        if (chdir(token.c_str()) != 0)
        {
            perror("cd");
        }
    }
    else
    {
        char cwd[1024];
        getcwd(cwd, 1024);
        prevDir = cwd;
        string newPath = "";
        string cwdpath = cwd;
        newPath += string(cwd) + "/" + token;
        if (chdir(newPath.c_str()) != 0)
        {
            perror("cd");
        }
    }
}