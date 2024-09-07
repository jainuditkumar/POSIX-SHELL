#include "shell.h"
#include <dirent.h>
#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <ctime>
#include <pwd.h>
#include <grp.h>

using namespace std;

// ls -l is not working;; sabse last mein dekhenege

void handleLsCommand(vector<string> token)
{
    bool show_all = false;
    bool long_format = false;
    vector<string> dirs;
    for (int i = 1; i < (int)token.size(); i++)
    {
        struct stat buf;
        if (token[i] == "-a")
        {
            show_all = true;
        }
        else if (token[i] == "-l")
        {
            long_format = true;
        }
        else if (token[i] == "-al" or token[i] == "-la")
        {
            show_all = true;
            long_format = true;
        }
        else if (token[i] == "..")
        {
            char cwd[1024];
            getcwd(cwd, 1024);
            string temp = cwd;
            int ind = temp.rfind("/");
            string prev(temp.begin(), temp.begin() + ind);
            dirs.push_back(prev);
        }
        else if (token[i] == "~")
        {
            dirs.push_back(homeDir);
        }
        else if (stat(token[i].c_str(), &buf) == 0)
        {
            dirs.push_back(token[i]);
        }
        else
        {
            cerr << "ls: cannot access " << token[i] << " No such file or directory\n";
        }
    }

    if (dirs.empty())
    {
        dirs.push_back(".");
    }

    for (string dir : dirs)
    {
        DIR *dp;
        struct dirent *ep;
        dp = opendir(dir.c_str());
        if (dp != NULL)
        {
            std::cout << dir << ":" << endl;
            while ((ep = readdir(dp)) != NULL)
            {
                if (!show_all && ep->d_name[0] == '.')
                {
                    continue;
                }
                if (long_format)
                {
                    struct stat sb;
                    stat(ep->d_name, &sb);
                    std::cout << (S_ISDIR(sb.st_mode) ? "d" : "-");
                    std::cout << (sb.st_mode & S_IRUSR ? "r" : "-");
                    std::cout << (sb.st_mode & S_IWUSR ? "w" : "-");
                    std::cout << (sb.st_mode & S_IXUSR ? "x" : "-");
                    std::cout << (sb.st_mode & S_IRGRP ? "r" : "-");
                    std::cout << (sb.st_mode & S_IWGRP ? "w" : "-");
                    std::cout << (sb.st_mode & S_IXGRP ? "x" : "-");
                    std::cout << (sb.st_mode & S_IROTH ? "r" : "-");
                    std::cout << (sb.st_mode & S_IWOTH ? "w" : "-");
                    std::cout << (sb.st_mode & S_IXOTH ? "x" : "-");
                    std::cout << " " << sb.st_nlink << " ";
                    std::cout << getpwuid(sb.st_uid)->pw_name << " ";
                    std::cout << getgrgid(sb.st_gid)->gr_name << " ";
                    std::cout << sb.st_size << " ";
                    std::cout << ctime(&sb.st_mtime);
                    std::cout << ep->d_name << endl;
                }
                else
                {
                    std::cout << ep->d_name << endl;
                }
            }
            closedir(dp);
        }
        else
        {
            cerr << "Error opening directory " << dir << endl;
        }
    }
}