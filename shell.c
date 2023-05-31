#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
//limits
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100

size_t MAX_LINE_LEN = 10000;

// builtin commands
#define EXIT_STR "exit"
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



void initialize()
{

	// allocate space for the whole line
	assert( (line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// allocate space for individual tokens
	assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);

	// open stdin as a file pointer 
	assert( (fp = fdopen(STDIN_FILENO, "r")) != NULL);

}

int tokenize (char * string)
{
	int token_count = 0;
	int size = MAX_TOKENS;
	char *this_token;

	while ( (this_token = strsep( &string, " \t\v\f\n\r")) != NULL) {

		if (*this_token == '\0') continue;


		tokens[token_count] = this_token;

		printf("Token %d: %s\n", token_count, tokens[token_count]);

		token_count++;

		if(token_count >= size){
			size*=2;
			// if there are more tokens than space ,reallocate more space
			assert ( (tokens = realloc(tokens, sizeof(char*) * size)) != NULL);
		}
	}
	return token_count;
}


int read_command() 
{

	// getline will reallocate if input exceeds max length
	assert( getline(&line, &MAX_LINE_LEN, fp) > -1); 

	printf("Shell read this line: %s\n", line);

	return tokenize(line);

}

int run_command() {

	if (strcmp( tokens[0], EXIT_STR ) == 0)
		return EXIT_CMD;

	return UNKNOWN_CMD;
}


int main()
{
	initialize();
	int token_count;
	char ** local_tokens = NULL;
		do {
		printf("sh550> ");
		token_count = read_command();



		}while(run_command() != EXIT_CMD);



}