# Small Shell
Small shell is a Linux-like shell written in C. It allows for the execution of basic built-in Linux commands, foreground and background processes, input and output redirection and signal handling for SIGINT and SIGTSTP.

## Features
- Custom command prompt (`$`)
- Support for:
  - Foregroung and background processes (`&`)
  - Input redirection (`<`)
  - Output redirection (`>`)
- Built-in commands:
  - `exit`
  - `cd`
  - `status`
- Comment and blank line handling
- Signal handling for:
  - `SIGINT` (Ctrl+C)
  - `SIGTSTP` (Ctrl+Z)
- Foreground-only mode toggle via `SIGTSTP`

##Executing External Commands
- All non built-in commands are executed using `fork()`, `exec()` family, and `waitpid()`

## Rules and Limitations
-   Maximum command line length: 2048 characters
-   Maximum arguments: 512
-   Command arguments are separated by spaces
-   No support for:
  - Pipes (`|`)
  - Mid-line comments
- If `&` appears as the final character of a command, the command runs as a background process.
- If `&` appears anywhere else, its treated as an argument to a command
- Built-in commands ignore `&` and always run in the forground
- Lines starting with `#` are treated as comments
- Mid-line comments are not supported
