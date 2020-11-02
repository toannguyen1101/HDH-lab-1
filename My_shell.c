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

char* history=NULL;

void remove_space(char *line) {
    int begin = 0;
    int end = strlen(line) - 1;
    while (line[begin] == ' '&& begin<end)
        begin++;
    while (line[end] == ' '&&begin <end)
        end--;
    for (int i = begin;i <= end;i++)
        line[i - begin] = line[i];
    line[end - begin+1] = '\0';
}

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
    int check = strlen(line);
    if (line[check - 1] == '\n')
    {
        line[check - 1] = '\0';
    }
    if (strcmp(line, "!!") != 0 && line[0] != '\0')
    {
        if (history != NULL) 
            free(history);
        history = (char*)malloc(strlen(line) + 1);
        strcpy(history, line);

    }
    return line;
}
#define _delim " \t\r\n\a"

int is_type(char*line)
{
    for (int i = 0;i < strlen(line);i++) {
        if (line[i] == '|')
            return 2;
        else if (line[i] == '>')
            return 3;
        else if (line[i] == '<')
            return 4;
    }
    return 1;
}

char** parse_pipe(char* line) {
   
    size_t size = 2;
    char** args = (char**)malloc(size * sizeof(char*));

    for (int i = 0;i < 2;i++) {
        args[i] = strsep(&line, "|<>");
        if (args[i] == NULL)
            break;
    }
   
    return args;
}
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

void exec_OR(char **command,char **file){}
void exec_IR(char** command, char** file) {}
void exec_pipe(char**args,char**argspipe) {
    // 0 is read end, 1 is write end 
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0) {
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(args[0], args) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    }
    else {
        // Parent executing 
        p2 = fork();

        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }

        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(argspipe[0], argspipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        }
        else {
            // parent executing, waiting for two children 
            waitpid(p1, NULL, 0);
            waitpid(p2, NULL, 0);
        }
    }
}


void shell_loop() {
    char* line;
    char** args;
    char** argspipe;
    int status=1;
    do {
        line = takeinput();
        remove_space(line);
        if (line[0] == '\0')
            continue;
        if (strcmp(line, "!!") == 0) {
            if (history == NULL) {
                printf("No history\n");
                continue;
            }
            else
                strcpy(line, history);
        }
        
        //Process_string and take type of command.
        int type = is_type(line);
        char** temp;
        if (type == 1)
            args = split_line(line);
        else {
            temp = parse_pipe(line);
            args = split_line(temp[0]);
            argspipe = split_line(temp[1]);
        }
        //If exit, end the shell.
        if (strcmp(args[0], "exit") == 0)
            break;

        //Process command
        switch (type)
        {
        case 1:
            execArg(args);//Simple command
            break;
        case 2:
            exec_pipe(args, argspipe);//Pipe
            break;
        case 3:
            exec_OR(args, argspipe);//Input redirection
            break;
        case 4:
            exec_IR(args, argspipe);//Output redirection
            break;
        default:
            break;
        }
        
        if (type != 1)
            free(argspipe);
        free(args);
        free(line);
    } while (status);
}



int main()
{
    shell_loop();
}
