#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
// limits
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100
#define MAX_HISTORY 100

size_t MAX_LINE_LEN = 10000;

// builtin commands
#define EXIT_STR "exit"
#define HISTORY_STR "hist"
#define PREV_STR "!!"
#define EXIT_CMD 0
#define UNKNOWN_CMD 99
#define TRUE 1
#define FALSE 0
#define WRITE_TO_FILE 1
#define READ_FROM_FILE -1
#define NO_REDIRECT 0

FILE *fp; // file struct for stdin
char **tokens;
char *line;
char *line2;
char *history[MAX_HISTORY];
int history_count = 0;

void initialize()
{

	// allocate space for the whole line
	assert((line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// allocate space for individual tokens
	assert((tokens = malloc(sizeof(char *) * MAX_TOKENS)) != NULL);

	// open stdin as a file pointer
	assert((fp = fdopen(STDIN_FILENO, "r")) != NULL);
}

int tokenize(char *string)
{
	int token_count = 0;
	int size = MAX_TOKENS;
	char *this_token;

	while ((this_token = strsep(&string, " \t\v\f\n\r")) != NULL)
	{

		if (*this_token == '\0')
			continue;

		tokens[token_count] = this_token;

		// printf("Token %d: %s\n", token_count, tokens[token_count]);

		token_count++;

		if (token_count >= size)
		{
			size *= 2;
			// if there are more tokens than space ,reallocate more space
			assert((tokens = realloc(tokens, sizeof(char *) * size)) != NULL);
		}
	}
	return token_count;
}

int read_command()
{

	// getline will reallocate if input exceeds max length
	assert(getline(&line, &MAX_LINE_LEN, fp) > -1);
	// line2 = strdup(line);
	// line2[strcspn(line2, "\r\t\n")] = 0;
	// dedsec995
	// printf("The line: '%s'",line2);
	if (strcmp(line, "!!") == 10)
	{
		// DO not Add this in History
	}
	else
	{
		history[history_count] = strdup(line);
		history_count++;
	}
	// printf("Shell read this line: %s\n%d", line,strcmp(line,"!!"));

	return tokenize(line);
}

int run_command()
{
	int redirect = NO_REDIRECT;
	int is_piped = FALSE;
	int pipe_loc = -1;
	int pipe_fd[2];

	if (strcmp(tokens[0], EXIT_STR) == 0)
		return EXIT_CMD;

	if (strcmp(tokens[0], HISTORY_STR) == 0)
	{
		for (int i = 0; i < history_count; i++)
		{
			printf("%d. %s", i + 1, history[i]);
		}
		return 1;
	}

	if (strcmp(tokens[0], PREV_STR) == 0)
	{
		if (history_count == 0)
		{
			printf("History is Empty or no command in History.\n");
			return 1;
		}
		else if (history_count > 0)
		{

			int prev = history_count - 1;
			line2 = strdup(history[prev]); // Since strsep remove the tokens[i]
			int token_count = tokenize(history[prev]);
			history[prev] = strdup(line2); // Recopy the original History String
			// Recursively call to rerun the command
			return run_command();
		}
	}

	for (int i = 0; i < MAX_TOKENS; i++)
	{
		if (tokens[i] == NULL)
		{
			// No token??
			break;
		}
		else if (strcmp(tokens[i], ">") == 0)
		{
			redirect = WRITE_TO_FILE;
			tokens[i] = NULL;					 // Set the ">" token to NULL
			freopen(tokens[i + 1], "w", stdout); // Redirect stdout to the file
			break;
		}
		else if (strcmp(tokens[i], "<") == 0)
		{
			redirect = READ_FROM_FILE;
			tokens[i] = NULL;					// Set the "<" token to NULL
			freopen(tokens[i + 1], "r", stdin); // Redirect stdin from the file
			break;
		}
		else if (strcmp(tokens[i], "|") == 0)
		{
			is_piped = TRUE;
			pipe_loc = i;
			tokens[i] = NULL; // Set the "|" token to NULL
			break;
		}
	}
	if (is_piped)
	{
		// Create a pipe
		if (pipe(pipe_fd) == -1)
		{
			perror("Pipe creation failed");
			return UNKNOWN_CMD;
		}

		// Create two child processes
		pid_t pid1, pid2;
		pid1 = fork();

		if (pid1 < 0)
		{
			perror("Forking error");
			exit(1);
		}
		else if (pid1 == 0)
		{
			// Child 1 process: Executes the first command and writes to the pipe
			close(pipe_fd[0]);				 // Close the read end of the pipe
			dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
			execvp(tokens[0], tokens);
			perror("Command execution failed");
			exit(1);
		}
		else
		{
			// Parent process
			pid2 = fork();

			if (pid2 < 0)
			{
				perror("Forking error");
				exit(1);
			}
			else if (pid2 == 0)
			{
				// Child 2 process: Executes the second command and reads from the pipe
				close(pipe_fd[1]);				// Close the write end of the pipe
				dup2(pipe_fd[0], STDIN_FILENO); // Redirect stdin to the read end of the pipe
				execvp(tokens[pipe_loc + 1], &tokens[pipe_loc + 1]);
				perror("Command execution failed");
				exit(1);
			}
			else
			{
				// Parent process
				close(pipe_fd[0]); // Close the read end of the pipe
				close(pipe_fd[1]); // Close the write end of the pipe
				waitpid(pid1, NULL, 0);
				waitpid(pid2, NULL, 0);
			}
		}
	}
	else
	{
		// No pipe, execute the command as before
		pid_t pid = fork();

		if (pid < 0)
		{
			perror("Forking error");
			exit(1);
		}
		else if (pid == 0)
		{
			// Child process: Executes the command
			execvp(tokens[0], tokens);
			perror("Command not found");
			exit(1);
		}
		else
		{
			// Parent process
			int status;
			waitpid(pid, &status, 0);
		}
	}

	// Reset file redirection
	if (redirect == WRITE_TO_FILE)
	{
		fclose(stdout);
		freopen("/dev/tty", "w", stdout);
	}
	else if (redirect == READ_FROM_FILE)
	{
		fclose(stdin);
		freopen("/dev/tty", "r", stdin);
	}

	return UNKNOWN_CMD;
}

void clean_tokens_array()
{
	// Need to reset it or it conflicts with other commands like ls
	for (int i = 0; i < MAX_TOKENS; i++)
	{
		tokens[i] = NULL;
	}
}

int main()
{
	initialize();
	int token_count;
	char **local_tokens = NULL;
	do
	{
		printf("sh550> ");
		clean_tokens_array(); // Lets reset
		token_count = read_command();
	} while (run_command() != EXIT_CMD);
}