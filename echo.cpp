#include "shell.h"

void handleEchoCommand(string token)
{
    int n = token.size();
    if (token[0] == '\'')
    {
        cout << string(token.begin() + 1, token.begin() + n - 1) << endl;
    }
    else if (token[0] == '\"')
    {
        cout << string(token.begin() + 1, token.begin() + n - 1) << endl;
    }
    else
    {
        cout << token << endl;
    }
}