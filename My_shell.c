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

int args_len(char** args) {
    int num = 0;
    while (args[num] != NULL)
        num++;
    return num;
}

int ampersand(char** args) {

    int check = 0;
    int n = args_len(args);
    if (strcmp(args[n - 1], "&") == 0)
    {
        check = 1;
        args[n - 1] = NULL; //xoa dau & vi shell khong hieu
    }
    else // lo nguoi dung nhap dinh lien, vd ls&
    {
        int len = strlen(args[n - 1]);
        char temp = args[n - 1][len - 1];
        if (temp == '&')
        {
            check = 1;
            args[n - 1][len-1]='\0';
        }
    }

    return check;
}

char* takeinput() {
    char* line = NULL;
    size_t size = 0;
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
    int amber_founded = ampersand(args);
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
    else if(amber_founded!=1) {
        // Parent process
        waitpid(pid, NULL, 0);
    }

    return;
}

void shell_loop() {
    char* line;
    char** args;
    int status=1;
    do {
        line = takeinput();
        args = split_line(line);
        if (strcmp(args[0], "exit")==0)
            break;
        execArg(args);
       
        free(args);
        free(line);
    } while (status);
}

int main()
{
    shell_loop();
}
