#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

void print_prompt(void)
{
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      char *base = strrchr(cwd, '/');
      if (base && *(base + 1) != '\0')
        printf("%s > ", base + 1);
    } else {
      printf("/ > ");
    }
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
      if (args[1] == NULL)
        chdir(getenv("HOME"));
      else
        chdir(args[1]);

      free(buffer);
      continue;
    }

    if (strcmp(args[0], "exit") == 0) {
      free(buffer);
      break;
    }

    pid_t pid = fork();
    if (pid == 0) {
      execvp(args[0], args);
        if (errno == ENOENT)
          fprintf(stderr,
                  "%s: command not found\n",
                  args[0]);
        else
          perror("exec");
        
        exit(1);
      }
    else {
      wait(NULL);
    }

    free(buffer);
  }
}
