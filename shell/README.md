Shell
=====

A basic shell that supports I/O redirection, pipes, background execution, and batch scripts. The redirection and pipe syntax is the normal <,>,| stuff. Batch scripts are passed as arguments. Background execution is specified with an ending '&' (i.e. foo&).

### Special commands
* cd "dir"
* clr - Clear the screen.
* dir "path"
* environ - List all the environment strings.
* echo "comment"
* help
* pause - Pause operation of the shell until 'Enter' is pressed
* quit

### Environment
* shell=path - Where path is the full path for the shell executable
* parent=path - Where path is the full path for the shell executable
