#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <readline/history.h>
#include <readline/readline.h>

char
*prompt(void)
{
  char cwd[1024];
  static char pbuf[1100];

  if (getcwd(cwd, sizeof(cwd)) == NULL)
    return("/ > ");

  char *dir = strrchr(cwd, '/');

  if (dir == NULL || strcmp(cwd, "/") == 0)
    dir = cwd;
  else 
    dir++;

  snprintf(pbuf, sizeof(pbuf), "%s > ", dir);
  return pbuf;
}

int
spawn(char **args)
{
  int status;
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return 1;
  }

  if (pid == 0) { 
    execvp(args[0], args); 
    if (errno == ENOENT) 
      fprintf(stderr, "%s: command not found\n", args[0]); 
    else
      perror("exec");

    exit(1); 
  } 

  wait(&status);

  if (WIFEXITED(status))
    return WEXITSTATUS(status);

  return 1;
}
  
int
main()
{
  for (;;)
  {
    char *input = readline(prompt());
    
    if (!input) {
      printf("\n");
      break;
    }

    if (*input) {
      add_history(input);
    }

    if (*input == '\0') {
      free(input);
      continue;
    }

    add_history(input);

    char *args[100];
    int i = 0;

    char *token = strtok(input, " ");

    while (token != NULL && i < 99) {
      args[i++] = token;
      token = strtok(NULL, " ");
    }

    args[i] = NULL;

    if (i == 0) {
      free(input);
      continue;
    }

    if (strcmp(args[0], "exit") == 0) {
      free(input);
      break;
    }

    if (strcmp(args[0], "cd") == 0) {
      char *dir = args[1] ? args[1] : getenv("HOME");
      if (chdir(dir) != 0)
        perror("cd");
      free(input);
      continue;
    }

    int and_pos = -1;

    for (int j = 0; j < i; j++) {
      if (strcmp(args[j], "&&") == 0) {
        and_pos = j;
        break;
      }
    }

    if (and_pos != -1) {
      args[and_pos] = NULL;
      if (spawn(args) == 0) {
        spawn(&args[and_pos + 1]);
      }
    } else {
      spawn(args);
    }

    free(input);
  }

  return 0;
}
