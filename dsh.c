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

  if (dir == NULL || strcmp(cwd, "/") == 0) {
    dir = cwd;
  } else {
    dir++;
  }
  snprintf(pbuf, sizeof(pbuf), "%s > ", dir);
  return (pbuf);
}

int
spawn(char **args)
{
  int status;
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return (1);
  }

  if (!pid) { 
    execvp(args[0], args); 
    if (errno == ENOENT) {
      fprintf(stderr, "%s: command not found\n", args[0]); 
    } else {
      perror("exec");
    }

    exit(1); 
  } 

  wait(&status);

  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }
  return (1);
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

    char *args[100];
    int i = 0;

    char *token = strtok(input, " ");

    while (token != NULL && i < 99) {
      args[i++] = token;
      token = strtok(NULL, " ");
    }

    args[i] = NULL;

    if (!i) {
      free(input);
      continue;
    }

    if (!strcmp(args[0], "exit")) {
      free(input);
      break;
    }

    if (!strcmp(args[0], "cd")) {
      char *p = args[1] ? args[1] : getenv("HOME");

      if (chdir(p) != 0)
        perror("cd");
      free(input);
      continue;
    }

    char **ap;
	  int  r;

	  ap = args;

    while (ap && *ap) {
      int i;
      char **next = NULL;

      for (i = 0; ap[i]; i++) {
        if (strcmp(ap[i], "&&") == 0) {
          ap[i] = NULL;
          next = &ap[i + 1];
          break;
        }
      }

      if ((r = spawn(ap)) != 0)
        break;

      ap = next;
    }

    free(input);
  }

  return (0);
}
