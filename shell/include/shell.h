#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <dirent.h>

#include <ncurses.h>
#include <term.h>

#define MAX_COM_LEN 2048
#define MAX_PIPES 50
#define BACK_COM 0x1
#define APP_OUT 0x2

//This is just a structure for representing
//a command. It can be linked together
//so multiple commands can be chained together.
//(ls -a / | wc)
typedef struct com
{
	char* *arg; //arguments passed to execp()
	size_t len; //length of 'arg'
	
	char *out; //path of output file (ls > file.txt)
	char *in; //path of input file (wc < file.txt)
	
	uint16_t flags; //special flags
	
	struct com *next; //next command
}
com_t;

extern char* *environ;
extern char cwd[PATH_MAX];

com_t* allcom(); //allocate command structure
void freecom(com_t *com); //deallocate command structure

int splitcom(com_t* *com, char *str); //splits a string into arguments 
int vercom(com_t *com); //verify command structure
int execcom(com_t *com); //execute command structure
int speccom(char* *com); //run special commands
int runcom(char *str); //runs a command line string

int shell(); //shell
int batch(char *file); //runs a batch script

int cd(char* *arg); //change directory
int dir(char* *arg); //list directory contents
int shell_pause(char* *arg);
int clr(char* *arg);
int env(char* *arg);
int shell_echo(char* *arg);
