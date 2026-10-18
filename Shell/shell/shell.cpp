#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <algorithm>

void lsh_loop(void);
char *lsh_read_line(void);
char **lsh_split_line(char *line);
int lsh_execute(char **args);

int lsh_num_builtins(void);
extern const char *builtin_str[];
extern int (*builtin_func[])(char **);

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  // Load config files, if any.

  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}

void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

char *lsh_read_line(void)
{
  std::string line;
  if (!std::getline(std::cin, line)) {
    if (std::cin.eof()) {
      exit(EXIT_SUCCESS);
    }
    perror("readline");
    exit(EXIT_FAILURE);
  }

  char *result = (char *)malloc(line.length() + 1);
  if (!result) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  std::copy(line.begin(), line.end(), result);
  result[line.length()] = '\0';
  return result;
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = (char **)malloc((size_t)bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = (char **)realloc(tokens, (size_t)bufsize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int lsh_execute(char **args)
{
  int i;
  std::string command;

  if (args == NULL || args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  for (i = 0; args[i] != NULL; i++) {
    if (i > 0) {
      command += " ";
    }
    command += args[i];
  }

  if (system(command.c_str()) == -1) {
    perror("lsh");
    return 1;
  }

  return 1;
}
/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
const char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins(void) {
  return (int)(sizeof(builtin_str) / sizeof(char *));
}

/*
  Builtin function implementations.
*/
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char **args)
{
  int i;
  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int lsh_exit(char **args)
{
  return 0;
}