#include "shell.h"

using namespace std;

// ls | grep .*txt

// Helper function to handle input redirection
int handleInputRedirection(const string &file)
{
    int fd = open(file.c_str(), O_RDONLY);
    if (fd < 0)
    {
        cerr << "Error: Couldn't open input file " << file << endl;
        return -1;
    }
    if (dup2(fd, STDIN_FILENO) < 0)
    {
        cerr << "Error: dup2 failed for input redirection" << endl;
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

// Helper function to handle output redirection
int handleOutputRedirection(const string &file, bool append)
{
    int fd = open(file.c_str(), O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
    if (fd < 0)
    {
        cerr << "Error: Couldn't open output file " << file << endl;
        return -1;
    }
    if (dup2(fd, STDOUT_FILENO) < 0)
    {
        cerr << "Error: dup2 failed for output redirection" << endl;
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

// Helper function to parse the command into individual commands based on pipes
vector<vector<string>> parsePipelines(const vector<string> &args)
{
    vector<vector<string>> pipelines;
    vector<string> currentCmd;

    for (const auto &arg : args)
    {
        if (arg == "|")
        {
            pipelines.push_back(currentCmd);
            currentCmd.clear();
        }
        else
        {
            currentCmd.push_back(arg);
        }
    }

    if (!currentCmd.empty())
    {
        pipelines.push_back(currentCmd);
    }

    return pipelines;
}

// Helper function to handle execution of a single command with redirection
void executeSingleCommand(vector<string> cmd)
{
    bool inputRedir = false, outputRedir = false, append = false;
    string inputFile, outputFile;
    vector<string> actualCmd;

    // Parse redirection symbols
    for (size_t i = 0; i < cmd.size(); ++i)
    {
        if (cmd[i] == "<" && i + 1 < cmd.size())
        {
            inputRedir = true;
            inputFile = cmd[++i];
        }
        else if (cmd[i] == ">" && i + 1 < cmd.size())
        {
            outputRedir = true;
            outputFile = cmd[++i];
            append = false;
        }
        else if (cmd[i] == ">>" && i + 1 < cmd.size())
        {
            outputRedir = true;
            outputFile = cmd[++i];
            append = true;
        }
        else
        {
            actualCmd.push_back(cmd[i]);
        }
    }

    // Fork and execute the command
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        if (inputRedir && handleInputRedirection(inputFile) != 0)
            exit(EXIT_FAILURE);
        if (outputRedir && handleOutputRedirection(outputFile, append) != 0)
            exit(EXIT_FAILURE);

        vector<char *> execArgs;
        for (auto &arg : actualCmd)
        {
            execArgs.push_back(const_cast<char *>(arg.c_str()));
        }
        execArgs.push_back(nullptr);

        execvp(execArgs[0], execArgs.data());
        cerr << "Error: Failed to execute command." << endl;
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        waitpid(pid, nullptr, 0); // Parent process waits for child
    }
    else
    {
        cerr << "Error: Forking process failed." << endl;
    }
}

// Function to handle piped commands
void executePipedCommands(const vector<vector<string>> &pipelines)
{
    int numCommands = pipelines.size();
    int pipeFds[2], prevFd = -1;

    for (int i = 0; i < numCommands; ++i)
    {
        pipe(pipeFds); // Create a new pipe for each iteration

        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            if (i > 0)
            {
                // If not the first command, get input from previous pipe
                dup2(prevFd, STDIN_FILENO);
                close(prevFd);
            }

            if (i < numCommands - 1)
            {
                // If not the last command, set output to the current pipe
                dup2(pipeFds[1], STDOUT_FILENO);
            }

            close(pipeFds[0]); // Close unused read end of the pipe

            // Execute the command
            executeSingleCommand(pipelines[i]);
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            // Parent process
            waitpid(pid, nullptr, 0); // Wait for the child process

            close(pipeFds[1]); // Close the write end of the pipe
            if (prevFd != -1)
                close(prevFd); // Close the previous pipe

            prevFd = pipeFds[0]; // Save the current read end for the next iteration
        }
        else
        {
            cerr << "Error: Forking process failed." << endl;
        }
    }

    if (prevFd != -1)
        close(prevFd); // Close the last pipe after all commands are done
}

// Main function to execute a command with redirection and pipes
void executeCommandWithRedirection(vector<string> args)
{
    vector<vector<string>> pipelines = parsePipelines(args);

    if (pipelines.size() > 1)
    {
        // Handle pipeline execution
        executePipedCommands(pipelines);
    }
    else
    {
        // Single command without pipes
        executeSingleCommand(pipelines[0]);
    }
}
