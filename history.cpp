#include "shell.h"

void display()
{
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
void handleHistory(string input)
{
    string path = homeDir + "/history.txt";
    input += "\n";
    // Open the file with read and append flags, creating if it doesn't exist
    int fd = open(path.c_str(), O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        perror("open");
        return;
    }

    // Write the input to the history file
    write(fd, input.c_str(), (int)input.size());
    close(fd);

    // Ensure only the latest 20 commands are kept in history.txt

    string line;

    vector<string> vec;
    // Open file again to read its contents
    ifstream historyFile(path);
    while (getline(historyFile, line))
    {
        vec.push_back(line);
    }
    historyFile.close();

    // Keep only the last 20 commands in the vector
    if (vec.size() > 20)
    {
        vec.erase(vec.begin(), vec.end() - 20);
    }

    // Rewrite the file with the last 20 commands
    ofstream outFile(path, ios::trunc); // Truncate the file
    for (const string &cmd : vec)
    {
        outFile << cmd << "\n";
    }
    outFile.close();
}

void history()
{
    string path = homeDir + "/history.txt";
    vector<string> vec;

    ifstream historyFile(path);
    string line;

    // Read the entire history into a vector
    while (getline(historyFile, line))
    {
        vec.push_back(line);
    }
    historyFile.close();

    // Get the most recent 10 commands (or fewer if less than 10 exist)
    int start = (vec.size() > 10) ? vec.size() - 10 : 0;

    // Print the last 10 commands
    for (int i = start; i < (int)vec.size(); ++i)
    {
        cout << i + 1 - start << " " << vec[i] << endl;
    }
}
void history(int n)
{
    string path = homeDir + "/history.txt";
    vector<string> vec;

    ifstream historyFile(path);
    string line;

    // Read the entire history into a vector
    while (getline(historyFile, line))
    {
        vec.push_back(line);
    }
    historyFile.close();

    // Get the most recent 10 commands (or fewer if less than 10 exist)
    int start = (vec.size() > n) ? vec.size() - n : 0;

    // Print the last 10 commands
    for (int i = start; i < (int)vec.size(); ++i)
    {
        cout << i + 1 - start << " " << vec[i] << endl;
    }
}

vector<string> historyVector;
int historyIndex = -1;
string input; // Global or passed by reference

void loadHistory()
{
    string path = homeDir + "/history.txt";
    historyVector.clear();

    ifstream historyFile(path);
    string line;

    // Load the entire history from the file into the vector
    while (getline(historyFile, line))
    {
        historyVector.push_back(line);
    }
    historyFile.close();

    // Set history index to the end, ready to traverse backwards
    historyIndex = historyVector.size();
}

string handleKeyPress()
{
    loadHistory(); // Ensure history is loaded before keypress handling

    struct termios orig_termios;
    tcgetattr(STDIN_FILENO, &orig_termios); // Get current terminal attributes
    struct termios new_termios = orig_termios;

    // Disable canonical mode and echo
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    char ch;
    string input;

    while (true)
    {
        ch = getchar();   // Get character input
        if (ch == '\033') // Arrow keys
        {
            getchar(); // skip the [
            ch = getchar();
            if (ch == 'A') // UP arrow key
            {
                // Traverse backward in history
                if (historyIndex > 0)
                {
                    historyIndex--;

                    // Clear the current line
                    cout << "\33[2K\r"; // Clear the entire line and move cursor to the beginning

                    display(); // Display prompt
                    input = historyVector[historyIndex];
                    cout << input; // Show history command
                }
            }
            else if (ch == 'B') // DOWN arrow key (optional)
            {
                // Traverse forward in history
                if (historyIndex < (int)historyVector.size() - 1)
                {
                    historyIndex++;
                    input = historyVector[historyIndex];

                    // Clear the current line
                    cout << "\33[2K\r"; // Clear the entire line and move cursor to the beginning

                    display(); // Display prompt
                    cout << input;
                }
                else
                {
                    historyIndex = historyVector.size();
                    input.clear();

                    // Clear the current line
                    cout << "\33[2K\r"; // Clear the entire line and move cursor to the beginning

                    display(); // Clear prompt
                }
            }
        }
        else if (ch == '\t') // TAB key for autocomplete
        {
            // Extract the last word/prefix from input
            string prefix;
            size_t pos = input.find_last_of(" ");
            if (pos != string::npos)
                prefix = input.substr(pos + 1);
            else
                prefix = input;

            // Get list of files/directories in the current directory
            DIR *dir;
            struct dirent *ent;
            vector<string> matches;
            if ((dir = opendir(".")) != NULL)
            {
                while ((ent = readdir(dir)) != NULL)
                {
                    string name = ent->d_name;
                    if (name.find(prefix) == 0) // Match the prefix
                    {
                        matches.push_back(name);
                    }
                }
                closedir(dir);
            }

            // Handle matches
            if (matches.size() == 1) // Single match
            {
                string completion = matches[0].substr(prefix.size());
                input += completion; // Complete the input
                cout << completion;  // Print the completion
                cout.flush();
            }
            else if (matches.size() > 1) // Multiple matches
            {
                cout << "\n"; // New line before listing matches
                for (const auto &match : matches)
                {
                    cout << match << " ";
                }
                cout << "\n";
                cout.flush();
                display();
                cout << input; // Re-display the prompt and current input
            }
        }
        else if (ch == '\n') // Enter key
        {
            cout << endl;
            tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios); // Restore terminal settings
            return input;                                    // Return the collected input
        }
        else if (ch == 127) // Backspace key
        {
            if (!input.empty())
            {
                // Remove last character from the input string
                input.pop_back();

                // Move cursor back, erase character, and move cursor back again
                cout << "\b \b"; // '\b' moves cursor back, ' ' prints space to erase character, '\b' moves back again
                cout.flush();
            }
        }
        else if (ch == 4) // ASCII code for CTRL-D is 4
        {
            tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios); // Restore terminal settings
            handleEof();                                     // Call EOF handler
        }
        else
        {
            input.push_back(ch); // Add character to input string
            cout << ch;          // Echo the character
            cout.flush();
        }
    }
}
