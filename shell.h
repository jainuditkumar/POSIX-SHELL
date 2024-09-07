#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <string>
#include <vector>
#include <limits.h>
#include <sstream>
#include <sys/stat.h>
#include <ctime>
#include <pwd.h>
#include <grp.h>
#include <csignal>
#include <map>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
using namespace std;

extern string homeDir;
extern string prevDir;
void displayPrompt();
void handleCdCommand(string token);
void handleEchoCommand(string token);
void handlePwdCommand();
void handleLsCommand(vector<string> token);
void handlePinfoCommand(pid_t pid);
void handleSearchCommand(string token);
void executeCommandWithRedirection(vector<string> arg);
void handleHistory(string token);
int executeSystemCommand(const std::vector<std::string> &command);
void bringToForeground(pid_t pid);
void handleBackgroundCommand(const std::vector<std::string> &command);
void handleFgCommand(const std::vector<std::string> &command);
void history();
void handleEof();
void history(int n);
void display();
string handleKeyPress();
extern std::map<pid_t, std::string> backgroundProcesses;

#endif // SHELL_H