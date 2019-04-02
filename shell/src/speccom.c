#include "shell.h"

//If arg[0] is the name of an internal command
//this function will run that command. Otherwise
//it will just return 2. If the command is quit
//then this function will return 1; this will
//cause the shell to exit.
int speccom(char* *arg)
{
	int (*func)(char* *arg) = NULL;
	int ret = 0;

	if(!strcmp(arg[0], "cd")) func = &cd;
	else if(!strcmp(arg[0], "echo")) func = &shell_echo;
	else if(!strcmp(arg[0], "pause")) func = &shell_pause;
	else if(!strcmp(arg[0], "clr")) func = &clr;
	else if(!strcmp(arg[0], "environ")) func = &env;
	else if(!strcmp(arg[0], "dir")) func = &dir;
	else if(!strcmp(arg[0], "quit")) return 1;
	else return 2;

	ret = (*func)(arg);

	return ret;
}

//Internal change directory command
int cd(char* *arg)
{
	if(arg[1])
	{
		if(chdir(arg[1]) == -1)
			return -201;

		if(!getcwd(cwd, PATH_MAX))
			return -202;

		if(setenv("PWD", cwd, 1) == -1)
			return -203;
	}
	else printf("%s\n", cwd);

	return 0;
}

int shell_echo(char* *arg)
{
	size_t i = 1;

	for(; arg[i]; i++) printf("%s ", arg[i]);

	printf("\n");

	return 0;
}

//Internal list directory contents command
int dir(char* *arg)
{
	DIR *dirp = NULL;
	struct dirent *direntp = NULL;

	if(!(dirp = opendir(arg[1] ? arg[1] : ".")))
		return -301;

	for(; (direntp = readdir(dirp));)
		printf("%s\n", direntp->d_name);

	return 0;
}

int shell_pause(char* *arg)
{
	int c = 0;

	printf("Press enter to continue...\n");

	//empty stdin so no input is left behind
	while((c = fgetc(stdin)) != '\n')
		if(c == EOF) return -321;

	return 0;
}

int clr(char* *arg)
{
 	if(tputs(tgetstr("cl", NULL), 1, &putchar) == ERR)
 		return -311;

	return 0;
}

int env(char* *arg)
{
	size_t i = 0;

	for(; environ[i]; i++)
		printf("%s\n", environ[i]);

	return 0;
}
