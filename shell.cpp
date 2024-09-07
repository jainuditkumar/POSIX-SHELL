#include "shell.h"

pid_t foregroundPid = -1;

std::map<pid_t, std::string> backgroundProcesses;

string homeDir;
string prevDir;

void displayPrompt()
{
    char hostname[1024];
    gethostname(hostname, 1024);

    char cwd[1024];
    getcwd(cwd, 1024);

    char *username = getenv("USER");

    homeDir = string(cwd);
    setenv("HOME", cwd, 1);

    string currDir = cwd;

    if (currDir == homeDir)
    {
        currDir = "~";
    }
    else
    {
        size_t found = currDir.find(homeDir);
        if (found != string::npos)
        {
            currDir.replace(0, found + homeDir.size(), "~");
        }
    }

    std::cout << "<" << username << "@" << hostname << ":" << currDir << "> ";
}

int executeBackgroundCommand(const std::vector<std::string> &command)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        char *args[command.size() + 1];
        for (size_t i = 0; i < command.size(); ++i)
        {
            args[i] = const_cast<char *>(command[i].c_str());
        }
        args[command.size()] = NULL;
        setpgid(0, 0);
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }
    else if (pid > 0)
    {
        // Parent process
        backgroundProcesses[pid] = "Running";
        std::cout << pid << std::endl; // Print PID only
    }
    else
    {
        // Error forking
        perror("fork");
        return 1;
    }
    return 0;
}

void bringToForeground(pid_t pid)
{
    auto it = backgroundProcesses.find(pid);
    if (it == backgroundProcesses.end())
    {
        std::cerr << "No such background process with PID " << pid << std::endl;
        return;
    }

    // Bring the process to the foreground
    kill(pid, SIGCONT); // Continue the process
    tcsetpgrp(STDIN_FILENO, getpgid(pid));
    waitpid(pid, NULL, 0); // Wait for it to complete

    // Remove the process from background map
    backgroundProcesses.erase(it);
}

void handleFgCommand(const std::vector<std::string> &command)
{
    if (command.size() != 2)
    {
        std::cerr << "Usage: fg <pid>" << std::endl;
        return;
    }

    pid_t pid = std::stoi(command[1]);
    auto it = backgroundProcesses.find(pid);
    if (it == backgroundProcesses.end())
    {
        std::cerr << "No such background process with PID " << pid << std::endl;
        return;
    }

    // Bring the process to the foreground
    kill(pid, SIGCONT);                    // Continue the process
    tcsetpgrp(STDIN_FILENO, getpgid(pid)); // Set the process group ID to control terminal
    foregroundPid = pid;                   // Update foregroundPid
    waitpid(pid, NULL, WUNTRACED);         // Wait for the process to stop or exit

    // Remove the process from background map
    backgroundProcesses.erase(it);
    foregroundPid = -1; // Reset foregroundPid after process is done
}

int executeSystemCommand(const std::vector<std::string> &command)
{
    bool background = false;

    if (!command.empty() && command.back() == "&")
    {
        background = true;
        const_cast<std::vector<std::string> &>(command).pop_back(); // Remove "&" from command
    }

    if (background)
    {
        return executeBackgroundCommand(command);
    }
    else
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            char *args[command.size() + 1];
            for (size_t i = 0; i < command.size(); ++i)
            {
                args[i] = const_cast<char *>(command[i].c_str());
            }
            args[command.size()] = NULL;

            execvp(args[0], args);
            perror("execvp");
            exit(1);
        }
        else if (pid > 0)
        {
            // Parent process
            foregroundPid = pid; // Track the foreground process

            // Wait for the foreground process
            int status;
            waitpid(pid, &status, WUNTRACED);

            // Reset foregroundPid
            foregroundPid = -1;

            if (WIFSTOPPED(status))
            {
                // The process was stopped
                std::cout << "\nProcess " << pid << " stopped." << std::endl;
                backgroundProcesses[pid] = "Stopped";
            }
            else if (WIFSIGNALED(status))
            {
                // The process was terminated by a signal
                std::cout << "\nProcess " << pid << " terminated by signal." << std::endl;
            }
            else if (WIFEXITED(status))
            {
                // The process exited normally
                std::cout << "\nProcess " << pid << " exited with status " << WEXITSTATUS(status) << "." << std::endl;
            }
        }
        else
        {
            // Error forking
            perror("fork");
            return 1;
        }
    }
    return 0;
}

void handleBackgroundCommand(const std::vector<std::string> &command)
{
    executeBackgroundCommand(command);
}

// Function to remove consecutive double quotes from a string
string removeConsecutiveQuotes(const std::string &str)
{
    std::string result;
    bool prevQuote = false;
    for (char c : str)
    {
        if (c == '"')
        {
            if (prevQuote)
            {
                result.pop_back();
                continue;
            }
        }
        result += c;
        prevQuote = (c == '"');
    }
    return result;
}

vector<string> tokenizeOnSpaces(const string &str)
{
    vector<string> tokens;
    string currentToken;
    bool insideDoubleQuote = false;
    bool insideSingleQuote = false;
    for (char c : str)
    {
        if (c == '"')
        {
            if (!insideSingleQuote)
            {
                insideDoubleQuote = !insideDoubleQuote;
            }
            else
            {
                currentToken += c;
            }
        }
        else if (c == '\'')
        {
            if (!insideDoubleQuote)
            {
                insideSingleQuote = !insideSingleQuote;
            }
            else
            {
                currentToken += c;
            }
        }
        else if (c == ' ' && !(insideDoubleQuote || insideSingleQuote))
        {
            if (!currentToken.empty())
            {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        }
        else
        {
            currentToken += c;
        }
    }
    if (!currentToken.empty())
    {
        tokens.push_back(currentToken);
    }
    return tokens;
}

bool findop(vector<string> line)
{
    for (auto i : line)
    {
        if (i == "<" or i == ">>" or i == ">" or i == "|")
        {
            return true;
        }
    }
    return false;
}
// Signal handler for SIGTSTP (CTRL-Z)
void handleSigTstp(int signum)
{
    if (foregroundPid > 0)
    {
        // Stop the foreground process and move it to the background
        kill(foregroundPid, SIGTSTP);
        backgroundProcesses[foregroundPid] = "Stopped";
        foregroundPid = -1;
    }
    else
    {
        std::cout << "\nNo foreground process running." << std::endl;
    }
}

// Signal handler for SIGINT (CTRL-C)
void handleSigInt(int signum)
{
    if (foregroundPid > 0)
    {
        // Interrupt the foreground process
        kill(foregroundPid, SIGINT);
        std::cout << "\nProcess " << foregroundPid << " interrupted." << std::endl;
        foregroundPid = -1;
        displayPrompt();
    }
    else
    {
        std::cout << "\nNo foreground process running." << std::endl;
    }
}

// Signal handler for CTRL-D (EOF)
void handleEof()
{
    cout << "\n";
    exit(0); // Exiting the shell
}
// Set up signal handling
void setupSignalHandlers()
{
    // Handle CTRL-Z (SIGTSTP)
    struct sigaction sigtstpAction;
    sigtstpAction.sa_handler = handleSigTstp;
    sigemptyset(&sigtstpAction.sa_mask);
    sigtstpAction.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGTSTP, &sigtstpAction, nullptr);

    // Handle CTRL-C (SIGINT)
    struct sigaction sigintAction;
    sigintAction.sa_handler = handleSigInt;
    sigemptyset(&sigintAction.sa_mask);
    sigintAction.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGINT, &sigintAction, nullptr);

    // Handle CTRL-D (EOF) by using terminal settings
    struct termios termSettings;
    tcgetattr(STDIN_FILENO, &termSettings);
    termSettings.c_cc[VEOF] = 4; // CTRL-D
    tcsetattr(STDIN_FILENO, TCSANOW, &termSettings);
}

int main()
{
    setupSignalHandlers(); // Initialize signal handling
    displayPrompt();
    while (true)
    {
        string input;
        input = handleKeyPress();

        if (input.empty())
            getline(cin, input);

        handleHistory(input);
        // Remove consecutive double quotes
        input = removeConsecutiveQuotes(input);

        // Tokenize on semicolonsv
        string token;
        vector<string> semicolonTokens;
        int quote = 0;
        int i = 0;
        while (i < (int)input.size())
        {
            string token = "";
            while (i < (int)input.size() and (input[i] != ';' or (input[i] == ';' and quote % 2 == 1)))
            {
                if (input[i] == '"' or input[i] == '\'')
                {
                    quote++;
                }
                token += input[i++];
            }
            i++;
            if (!token.empty())
            {
                semicolonTokens.push_back(token);
            }
        }

        // Tokenize on spaces, but not inside double quotes
        vector<vector<string>> line;
        for (const auto &semicolonToken : semicolonTokens)
        {
            line.push_back(tokenizeOnSpaces(semicolonToken));
        }

        for (int i = 0; i < (int)line.size(); i++)
        {
            if (line[i][0] == "cd")
            {
                if (line[i].size() > 2)
                {
                    cerr << "cd: too many arguments\n";
                }
                else if (line[i].size() == 1)
                {
                    if (chdir(homeDir.c_str()) != 0)
                    {
                        perror("cd");
                    }
                }
                else
                {
                    handleCdCommand(line[i][1]);
                }
            }
            else if (line[i][0] == "pwd")
            {
                handlePwdCommand();
            }
            else if (line[i][0] == "fg")
            {
                handleFgCommand(line[i]);
            }
            else if (findop(line[i]))
            {
                executeCommandWithRedirection(line[i]);
            }
            else if (line[i][0] == "echo")
            {
                if (line[i].size() == 1)
                    cout << "\n";
                else
                    handleEchoCommand(line[i][1]);
            }
            else if (line[i][0] == "ls")
            {
                handleLsCommand(line[i]);
            }
            else if (line[i][0] == "pinfo")
            {
                if (line[i].size() == 2)
                    handlePinfoCommand(stoi(line[i][1]));
                else
                    executeSystemCommand(line[i]);
            }
            else if (line[i][0] == "search")
            {
                handleSearchCommand(line[i][1]);
            }
            else if (line[i][0] == "history")
            {
                if (line[i].size() == 1)
                {
                    history();
                }
                else
                {
                    history(stoi(line[i][1]));
                }
            }
            else if (line[i][0] == "exit")
            {
                exit(0);
                break;
            }
            else
            {
                executeSystemCommand(line[i]);
            }
        }
        char hostname[1024];
        gethostname(hostname, 1024);

        char *username = getenv("USER");

        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL && cwd != homeDir)
        {
            cout << "<" << username << "@" << hostname << ":" << cwd << "> ";
        }
        else if (getcwd(cwd, sizeof(cwd)) != NULL && cwd == homeDir)
        {
            displayPrompt();
        }
        else
        {
            perror("getcwd");
        }
    }
}
