#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

void print_prompt(void)
{
  char cwd[1024];

  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    printf("/ > ");
  }

  char *last = strrchr(cwd, '/');

  if (last == NULL)
    printf("%s > ", cwd);
  else if (*(last + 1) != '\0')
    printf("%s > ", last + 1);
  else
    printf("/ > ");
}

int run_command(char **args)
{
  pid_t pid = fork();
  int status;

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
  
int main()
{
  while (1)
  {
    char *buffer = NULL;
    size_t size = 0;

    print_prompt();

    ssize_t char_read = getline(&buffer, &size, stdin);

    if (char_read == -1) {
      printf("\n");
      free(buffer);
      break;
    }

    if (char_read > 0 && buffer[char_read - 1] == '\n')
      buffer[char_read - 1] = '\0';

    char *args[100];

    int i = 0;

    char *token = strtok(buffer, " ");
    while (token != NULL && i < 99) {
      args[i++] = token;
      token = strtok(NULL, " ");
    }

    args[i] = NULL;

    if (i == 0) {
      free(buffer);
      continue;
    }

    if (strcmp(args[0], "cd") == 0) {
      char *dir = args[1];
      if (dir == NULL)
        dir = getenv("HOME");
      if (chdir(dir) != 0)
        perror("cd");

      free(buffer);
      continue;
    }

    if (strcmp(args[0], "exit") == 0) {
      free(buffer);
      break;
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

      char **cmd1 = args;
      char **cmd2 = &args[and_pos + 1];

      int status = run_command(cmd1);

      if (status == 0)
        run_command(cmd2);

      free(buffer);
      continue;
    }

    run_command(args);

    free(buffer);
  }
}
