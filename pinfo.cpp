#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

std::string getProcessInfo(pid_t pid, const std::string &key)
{
    std::string filePath = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream file(filePath);
    if (!file)
    {
        return "Error: unable to open file";
    }

    std::string line, value;
    while (std::getline(file, line))
    {
        if (line.find(key) != std::string::npos)
        {
            std::stringstream ss(line);
            std::string dummy;
            ss >> dummy >> value;
            break;
        }
    }
    file.close();
    return value;
}

std::string getExecutablePath(pid_t pid)
{
    std::string filePath = "/proc/" + std::to_string(pid) + "/exe";
    char path[1024];
    ssize_t len = readlink(filePath.c_str(), path, 1024);
    if (len == -1)
    {
        return "Error: unable to read link";
    }
    path[len] = '\0';
    return std::string(path);
}

void handlePinfoCommand(pid_t pid)
{
    std::string status = getProcessInfo(pid, "State:");
    std::string vm = getProcessInfo(pid, "VmSize:");
    std::string path = getExecutablePath(pid);

    if (status.find("Error:") != std::string::npos || vm.find("Error:") != std::string::npos || path.find("Error:") != std::string::npos)
    {
        std::cerr << "Error: unable to retrieve process information" << std::endl;
        return;
    }

    std::cout << "pid -- " << pid << std::endl;
    std::cout << "Process Status -- " << status << std::endl;
    std::cout << "memory -- " << vm << " {Virtual Memory}" << std::endl;
    std::cout << "Executable Path -- " << path << std::endl;
}