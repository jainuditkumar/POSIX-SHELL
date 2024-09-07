#include "shell.h"
#include <unistd.h>
#include <sys/wait.h>

void handlePipeline(string token)
{
    vector<string> commands;
    size_t pos = token.find("|");
    while (pos != string::npos)
    {
        commands.push_back(token.substr(0, pos));
        token.erase(0, pos + 1);
        pos = token.find("|");
    }
    commands.push_back(token);

    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        std::cerr << "Error creating pipe" << std::endl;
        return;
    }

    for (size_t i = 0; i < commands.size() - 1; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            close(pipefd[0]);               // Close read end
            dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to write end
            close(pipefd[1]);               // Close write end

            // Execute command
            execlp(commands[i].c_str(), commands[i].c_str(), (char *)NULL);
            std::cerr << "Error executing command '" << commands[i] << "'" << std::endl;
            exit(1);
        }
        else if (pid > 0)
        {
            // Parent process
            close(pipefd[1]); // Close write end
        }
        else
        {
            std::cerr << "Error forking process" << std::endl;
            return;
        }
    }

    // Last command in pipeline
    dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to read end
    close(pipefd[0]);              // Close read end

    // Execute last command
    execlp(commands.back().c_str(), commands.back().c_str(), (char *)NULL);
    std::cerr << "Error executing command '" << commands.back() << "'" << std::endl;
    exit(1);
}