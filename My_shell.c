#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<time.h>

char* takeinput() {
    char* line = NULL;
    int size = 0;
    printf("T_Shell>> ");
   
    getline(&line, &size, stdin);

    return line;
}
#define _delim " \t\r\n\a"
char** split_line(char*line) {
    
    int size = 64;
    int pos = 0;
    char** tokens = (char**)malloc(size * sizeof(char*));
    char* token;

    token = strtok(line, _delim);
    while (token != NULL) {
        tokens[pos] = token;
        pos++;

        if (pos >= size) {
            size += 64;
            tokens = (char**)realloc(tokens, size * sizeof(char*));
        }

        token = strtok(NULL, _delim);
    }
    
    tokens[pos] = NULL;
    return tokens;
}

void execArg(char **args) {
    pid_t pid, wpid;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("exevcp failed");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("Error forking");
        exit(EXIT_FAILURE);
    }
    else {
        // Parent process
        waitpid(pid, NULL, 0);
    }

    return;
}

void shell_loop() {
    char* line;
    char** args;
    do {
        line = takeinput();
        args = split_line(line);
        execArg(args);
        free(args);
        free(line);
    } while (strcmp(args[0], "exit") != 0);
}

int main()
{
    shell_loop();
}
