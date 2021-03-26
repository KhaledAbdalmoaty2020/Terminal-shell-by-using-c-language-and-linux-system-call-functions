#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For strtok() and strcmp()
#include <unistd.h> // For fork(), 
#include <sys/wait.h> // For waitpid() 

char SHELL_NAME[50] = "myShell";
int QUIT = 0;

// Function to read a line from command into the buffer
char *readLine()
{
	char *line = (char *)malloc(sizeof(char) * 1024); // Dynamically Allocate Buffer
	char c;
	int pos = 0, bufsize = 1024;
	if (!line) // Buffer Allocation Failed
	{
		printf("\nBuffer Allocation Error.");
		exit(EXIT_FAILURE);
	}
	while(1)
	{
		c=getchar();//take one character from the user 
		//to remove the \n when we press enter after command and put Null at the end 
		if (c == EOF || c == '\n') // If End of File or New line, replace with Null character
		{
			line[pos] = '\0';
			return line;
		}
		else
		{
			line[pos] = c;
		}
		pos ++;
		// If we have exceeded the buffer
		if (pos >= bufsize)
		{
			bufsize += 1024;
			line = realloc(line, sizeof(char) * bufsize);
			if (!line) // Buffer Allocation Failed
			{
			printf("\nBuffer Allocation Error.");
			exit(EXIT_FAILURE);
			}
		}
	}

}

// Function to split a line into constituent commands
char **splitLine(char *line)
{  
	char **tokens = (char **)malloc(sizeof(char *) * 64);
	char *token;
	char delim[10] = " \t\n\r\a";//according to it strtok() will separate 
	int pos = 0, bufsize = 64;
	if (!tokens)
	{
		printf("\nBuffer Allocation Error.");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, delim);//token will contain the first piece of command
	while (token != NULL)
	{
		tokens[pos] = token;
		pos ++;
		if (pos >= bufsize)
		{
			bufsize += 64;
			line = realloc(line, bufsize * sizeof(char *));
			if (!line) // Buffer Allocation Failed
			{
			printf("\nBuffer Allocation Error.");
			exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, delim);//to get the next piece of command
	}
	tokens[pos] = NULL;//put Null at the end of array 
	return tokens;
}

// Section Dealing with Built-in Commands

// Function Declarations
int myShell_cd(char **args);
int myShell_exit();

// Definitions
char *builtin_cmd[] = {"cd", "exit"};

int (*builtin_func[]) (char **) = {&myShell_cd, &myShell_exit}; // Array of function pointers for call from execShell

int numBuiltin() // Function to return number of builtin commands
{
	return sizeof(builtin_cmd)/sizeof(char *);
}

// Builtin command definitions
int myShell_cd(char **args)//function to change the directory 
{
	if (args[1] == NULL) //to check if there is a command
	{
		printf("myShell: expected argument to \"cd\"\n");
	} 
	else 
	{
		//chdir("path")->is a system call function to change the current directory 
		//to directory specified into the path
		if (chdir(args[1]) != 0) //change dir not succsseded
		{
			perror("myShell: ");//error message
		}
	}
	return 1;
}

int myShell_exit()
{
	QUIT = 1;
	return 0;
}

// Function to create child process and run command
int myShellLaunch(char **args)
{
	pid_t pid, wpid;//pid_t is an datatype equl to integar datatype
	int status;//to save in it the status of the child process
	pid = fork();//if the fork() success it return 0
	//fork()-> is the system call to make child process
	if (pid == 0)
	{
		// The Child Process
		//noteThat->execvp()->execut some command like ls , ls -l , cat fileName-
		if (execvp(args[0], args) == -1)
		{
			perror("myShell: ");//error message
		}
	exit(EXIT_FAILURE);
	}
	else if (pid < 0)//fork() didn't success
	{
		//Forking Error
		perror("myShell: ");
	}
	else
	{
		// The Parent Process
		do 
		{
			wpid = waitpid(pid, &status, WUNTRACED);//wait for child process until it finish
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

// Function to execute command from terminal
int execShell(char **args)
{
	int ret;
	if (args[0] == NULL)//to check if there is not command
	{
		// Empty command
		return 1;
	}
	// Loop to check for builtin functions
	for (int i=0; i< numBuiltin(); i++) // numBuiltin() -> returns the number of builtin functions
	{
		if(strcmp(args[0], builtin_cmd[i])==0) // Check if user function matches builtin function name
			return (*builtin_func[i])(args); // Call respective builtin function with arguments
	}
	ret = myShellLaunch(args);
	return ret;
}


// When myShell is called Interactively
int myShellInteract()
{
	char *line; //point to character
	char **args; //array of pointer to character
	while(QUIT == 0)//Quit is integer container
	{
		printf("%s> ", SHELL_NAME);//char SHELL_NAME[50] = "myShell";
		line=readLine();//to get user input put it into array of character 
		args=splitLine(line);//func to remove all delim like " " or \t etc 
		//and put all user command into array contiguiously
		execShell(args);// Function to execute command from terminal
		free(line);
		free(args);
	}
	return 1;
}


int main(int argc, char **argv)
{

	if (argc == 1)
		//when the program run we go to myshellinteract() function
		//as it make aloop to be able execute more than one order 
		myShellInteract(); 

	else
		printf("\nInvalid Number of Arguments");

	// Exit the Shell
	return EXIT_SUCCESS;
}