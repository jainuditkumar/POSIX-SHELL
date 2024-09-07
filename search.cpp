#include <iostream>
#include <filesystem>
#include <string>

using namespace std;

// Function to search for a file or directory recursively
bool searchDirectory(const filesystem::path &currentDir, const string &target)
{
    for (const auto &entry : filesystem::directory_iterator(currentDir))
    {
        if (entry.is_regular_file() || entry.is_directory())
        {
            if (entry.path().filename().string() == target)
            {
                return true;
            }
            if (entry.is_directory())
            {
                if (searchDirectory(entry.path(), target))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void handleSearchCommand(string token)
{
    filesystem::path cwd = filesystem::current_path();
    bool found = searchDirectory(cwd, token);

    if (found)
    {
        cout << "True" << endl;
    }
    else
    {
        cout << "False" << endl;
    }
}