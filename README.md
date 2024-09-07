## README

# Custom POSIX Shell

This is a custom POSIX shell implemented in C++ with several built-in commands, process handling, signal handling, I/O redirection, piping, command history, and autocompletion features. The shell mimics common shell behaviors and provides a basic interactive command-line interface for executing commands.

## Features

-**Builtin Commands**:  
 The shell supports the following built-in commands:

- `cd`: Change the current working directory with support for:
  - `.`: current directory
  - `..`: parent directory
  - `-`: previous directory
  - `~`: home directory
- `pwd`: Print the current working directory.
- `echo`: Display a line of text.
- `ls`: List directory contents.
- `history`: View command history.
- `pinfo`: Display information about a process.
- `search`: Search for a specific term in files.

- **Process Management**:  
  The shell supports both foreground and background process handling:

  - Foreground process management using the `fg` command.
  - Background process management with automatic handling of background jobs via the `&` operator.

- **Input/Output Redirection**:  
  Commands can be executed with I/O redirection:

  - `<`: Input redirection.
  - `>`: Output redirection.
  - `>>`: Append output redirection.
  - `|`: Piping between commands.

- **Signal Handling**:

  - `CTRL-Z`: Suspends the current foreground job and moves it to the background.
  - `CTRL-C`: Interrupts the current foreground job.
  - `CTRL-D`: Exits the shell (End of File).

- **Command History**:

  - Stores up to 20 commands in a `history.txt` file and loads the most recent 10 into memory.
  - Allows navigation through previous commands using the UP arrow key.

- **Autocompletion**:  
  The shell autocompletes file and directory names in the current working directory when the `TAB` key is pressed.

## Project Structure

- **shell.cpp**: Main file containing the shell's entry point and core logic.
- **cd.cpp**: Implementation of the `cd` command.
- **echo.cpp**: Implementation of the `echo` command.
- **pwd.cpp**: Implementation of the `pwd` command.
- **ls.cpp**: Implementation of the `ls` command.
- **history.cpp**: Management of command history.
- **pinfo.cpp**: Process information retrieval via the `pinfo` command.
- **search.cpp**: Search functionality implementation.
- **io_redirection.cpp**: Handles I/O redirection and piping.

## Compilation and Execution

To compile the shell, use the provided Makefile.

1. **Compilation**:

   ```bash
   make
   ```

2. **Run the Shell**:

   ```bash
   ./shell
   ```

3. **Clean Up**:
   To remove compiled object files and the executable, run:
   ```bash
   make clean
   ```

## Example Usage

```bash
<user@hostname:~> cd /home
<user@hostname:/home> pwd
/home
<user@hostname:/home> echo Hello World
Hello World
<user@hostname:/home> ls
file1.txt  file2.txt  directory/
<user@hostname:/home> pinfo 1234
Process ID: 1234
Process Status: Running
<user@hostname:/home> history
1 cd /home
2 pwd
3 echo Hello World
<user@hostname:/home> exit
```

## Known Limitations

- The `echo` command does not currently handle environment variables or escape flags.
- The shell does not fully support advanced piping with more than two commands.
- The implementation of `pinfo` requires manual process IDs as input.

## License

This project is open-source and available under the MIT License.

---
