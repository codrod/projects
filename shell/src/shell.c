#include "shell.h"

char cwd[PATH_MAX]; //current working directory
int STDOUT, STDIN;

int main(int argc, char *argv[])
{
	size_t i = 1;
	char *loc;

	if(tgetent(NULL, getenv("TERM")) == ERR)
		return 4;

	STDIN = dup(STDIN_FILENO);
	STDOUT = dup(STDOUT_FILENO);

	//This sets the "shell" environment variable
	if(argv[0][0] != '/')
	{
		loc = strrchr(argv[0], '/');
		*loc = 0;

		if(!realpath(argv[0], cwd))
			return 6;

		*loc = '/';
		strcat(cwd, loc);

		if(setenv("shell", cwd, 1) == -1)
			return 4;
	}
	else if(setenv("shell", argv[0], 1) == -1)
		return 5;

	//initialize cwd
	if(!getcwd(cwd, PATH_MAX))
		return 1;

	if(argc > 1)
	{
		for(; i < argc; i++)
			if(batch(argv[i]))
				return 3;
	}
	else shell();

	return 0;
}

com_t* allcom()
{
	com_t *com = NULL;

	if(!(com = calloc(1, sizeof(com_t))))
		return NULL;

	com->arg = NULL;
	com->out = com->in = NULL;
	com->next = NULL;

	return com;
}

//freecom() will free all the command
//structures that are linked to 'com'.
void freecom(com_t *com)
{
	void *tmp = NULL;
	size_t i = 0;

	while(com)
	{
		//There may be no arguments if an error
		//occurs during parsing.
		if(com->arg)
		{
			for(i = 0; i < com->len; i++)
				if(com->arg[i])
					free(com->arg[i]);

			//if 'com->out' and 'com->in' equal NULL then
			//then they dont need to be freed.
			if(com->out) free(com->out);
			if(com->in) free(com->in);

			free(com->arg);
		}

		tmp = com->next;
		free(com);
		com = tmp;
	}

	return;
}

//splitcom() creates a command structure
//from a string representing a command.
int splitcom(com_t* *comp, char *str)
{
	com_t *com = NULL;
	void *tmp = NULL;

	size_t i = 0, j = 0, argi = 0, nargs = 0, argsiz = 0;
	int wasprint = 0, isquote = 0;

	if(!(com = allcom()))
		return -1;

	for(*comp = com; str[i]; i++, com->len = argi)
	{
		if(str[i] == '"')
		{
			if(isquote)
			{
				isquote = 0;
				com->arg[argi++][j] = 0;
				com->arg[argi] = NULL;
				j = argsiz = wasprint = 0;

				continue;
			}
			else isquote = 1;
		}

		if(str[i] == '|')
		{
			//Allocate a new command structure and link it
			//to the command list.

			if(!(com->next = allcom()))
				return -3;

			com = com->next;
			com->next = NULL;

			argi = j = nargs = argsiz = 0;
			wasprint = 0;
		}
		else if(isquote || !isspace(str[i])) //If not whitespace or between ""
		{
			wasprint = 1;

			//Expand array if necessary
			if(argi >= nargs)
			{
				if(!(tmp = realloc(com->arg, (nargs + 8) * sizeof(char*))))
					return -2;

				com->arg = tmp;
				nargs += 8;
			}

			if(j >= argsiz)
			{
				if(!argsiz) com->arg[argi] = NULL;

				if(!(tmp = realloc(com->arg[argi], (argsiz + 16) * sizeof(char))))
					return -4;

				com->arg[argi] = tmp;
				argsiz += 16;
			}

			com->arg[argi][j++] = str[i];
		}
		else if(wasprint) //If whitespace separating two arguments
		{
			com->arg[argi++][j] = 0;
			com->arg[argi] = NULL;
			j = argsiz = wasprint = 0;
		}
	}

	return 0;
}

//This function verifies that
//the command structure is valid.
int vercom(com_t *start)
{
	com_t *com = start;
	size_t i = 0;

	for(; com; com = com->next)
	{
		//'com->arg' is only NULL if
		//a pipe is missing it's succeeding command
		//or the string is empty.
		if(!com->arg) return -20;

		for(i = 0; com->arg[i]; i++)
		{
			//The first '"' is left behind
			//by splitcom() so this function
			//knows this argument should be
			//interpreted literally.
			if(com->arg[i][0] == '"')
			{
				memmove(&com->arg[i][0], &com->arg[i][1],
				(strlen(&com->arg[i][1]) + 1) * sizeof(char));
			}
			else if(!strcmp(com->arg[i], ">") || !strcmp(com->arg[i], ">>"))
			{
				//This makes sure the '>' or '>>' is used correctly
				if(!i || com->out || !com->arg[i + 1] || com->next)
					return -21;

				if(!strcmp(com->arg[i], ">>"))
					com->flags |= APP_OUT;

				//Separates the output file from the rest
				//of the arguments
				com->out = com->arg[i + 1];
				free(com->arg[i]);
				com->arg[i] = NULL;
				com->arg[++i] = NULL;

				//Strips leading " left behind by splitcom (see above)
				if(com->out[0] == '"')
					memmove(&com->out[0], &com->out[1],
					(strlen(&com->out[1]) + 1) * sizeof(char));
			}
			else if(!strcmp(com->arg[i], "<"))
			{
				if(!i || com->in || !com->arg[i + 1] || com != start)
					return -22;

				com->in = com->arg[i + 1];
				free(com->arg[i]);
				com->arg[i] = NULL;
				com->arg[++i] = NULL;

				//Strips leading " left behind by splitcom (see above)
				if(com->in[0] == '"')
					memmove(&com->in[0], &com->in[1],
					(strlen(&com->in[1]) + 1) * sizeof(char));
			}
			else if(!strcmp(com->arg[i], "&"))
			{
				if(!i || com->arg[i + 1] || com->next)
					return -23;

				//Sets a flag so execcom() knows to
				//background the process
				start->flags |= BACK_COM;
				free(com->arg[i]);
				com->arg[i] = NULL;

				//This will cause output to be redirected
				//to /dev/null unless its already been redirected
				//to a file.
				if(!com->out)
				{
					if(!(com->out = malloc(10 * sizeof(char*))))
						return -24;

					strcpy(com->out, "/dev/null");
				}
			}
			//<, >, and >> must be at the end of the
			//command so arguments are clearly
			//separate
			else if(com->out || com->in) return -25;
		}
	}

	return 0;
}

//This function will execute a command structure
int execcom(com_t *start)
{
	com_t *com = start;
	pid_t pid = -1;
	int pipes[MAX_PIPES][2], outf = -1, inf = -1, ret = 0, status = 0;
	size_t pipei = 0;

	//Iterate through command list
	for(; com; com = com->next, pipei++)
	{
		//If there is another command in the list
		//we will need another pipe. Notice there
		//are multiple pipes.
		if(com->next && pipe(pipes[pipei]) == -1)
			return -30;

		//This just opens the output file if there
		//is an output file.
		if(com->out && (outf = open(com->out,
		 ((com->flags & APP_OUT ? O_APPEND : O_TRUNC) | O_CREAT | O_WRONLY), S_IRWXU)) == -1)
			return -31;

		//This opens the input file assuming there is one
		if(com->in && (inf = open(com->in, O_RDONLY)) == -1)
			return -32;

		//If an output file is open then redirect stdout to the file
		if(com->out)
		{
			if(dup2(outf, STDOUT_FILENO) == -1)
				return -39;
		}
		else if(com->next)//If this isnt the last command in
		{                 //the command list then redirect
		                  //stdout to the current pipe's write end
			if(dup2(pipes[pipei][1], STDOUT_FILENO) == -1)
				return -40;
		}

		//If an input file is open then redirect stdin to the file
		if(com->in)
		{
			if(dup2(inf, STDIN_FILENO) == -1)
				return -41;
		}
		else if(com != start)//If this isnt the first command
		{                    //then redirect stdin to the read
		                     //end of the previous pipe.
			if(dup2(pipes[pipei - 1][0], STDIN_FILENO) == -1)
				return -42;
		}

		if((ret = speccom(com->arg)) == 2)
		{
			if(!(pid = fork()))
			{
				//Close the read end of the pipe if a pipe is open
				if(com->next && close(pipes[pipei][0]) == -1)
					exit(-34);

				if(setenv("parent", getenv("shell"), 1) == -1)
					exit(-44);

				if(execvp(com->arg[0], com->arg) == -1)
					exit(-33);
			}
			else
			{
				//Close the write end of the current pipe.
				if(com->next && close(pipes[pipei][1]) == -1)
					return -35;

				//If this command isnt backgrounded then we will wait for it
				//to complete.
				if(!(start->flags & BACK_COM) && waitpid(pid, &status, 0) == -1)
					return -38;

				//checks the exit status for errors
				if(WEXITSTATUS(status))
					return -33;
			}
		}

		if((com->in || com != start) && dup2(STDIN, STDIN_FILENO) == -1)
			return -39;

		if((com->out || com->next) && dup2(STDOUT, STDOUT_FILENO) == -1)
			return -40;

		//Close output/input files
		if(com->out && close(outf) == -1)
			return -36;

		if(com->in && close(inf) == -1)
			return -37;

		//return value from speccom()
		if(ret < 0 || ret == 1) return ret;
	}

	return 0;
}

//This function will run a command line string.
//This function will first convert the command
//line string into a command structure. Then
//it will execute the command structure using
//execcom().
int runcom(char *str)
{
	com_t *com = NULL;
	int ret = 0;

	if(!(ret = splitcom(&com, str)))
		if(!(ret = vercom(com)))
			ret = execcom(com);

	freecom(com);
	return ret;
}

//This is the shell itself
int shell()
{
	char *str = NULL;
	size_t size = 0;
	int ret = 0;

	for(; ret != 1; errno = 0)
	{
		printf("<%s>: ", cwd);

		getline(&str, &size, stdin);

		if((ret = runcom(str)) < 0)
		{
			fprintf(stderr, "ERROR(%i): cant run command : ", ret);

			if(errno) perror(NULL);
			else printf("\n");
		}
	}

	free(str);

	return ret;
}

//This function runs a batch file.
//The path and filename are represented
//by the string 'file'.
int batch(char *file)
{
	FILE *fp = NULL;
	char *com = NULL;
	size_t siz = 0;
	int ret = 0;

	if(fp = fopen(file, "r"))
	{
		while(getline(&com, &siz, fp) != -1)
			if(ret = runcom(com)) break;

		if(com) free(com);

		if(fclose(fp) == EOF)
			ret = -302;
	}
	else ret = -303;

	if(ret)
	{
		fprintf(stderr, "ERROR(1001): cant run batch script : ");

		if(errno) perror(NULL);
		else printf("\n");
	}

	return ret;
}
