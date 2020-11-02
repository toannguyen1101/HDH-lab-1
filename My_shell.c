#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>


char* history=NULL;

char* remove_space(char *line) {
    int begin = 0;
    int end = strlen(line) - 1;
    while (line[begin] == ' '&& begin<end)
        begin++;
    while (line[end] == ' '&&begin <end)
        end--;
    for (int i = begin;i <= end;i++)
        line[i - begin] = line[i];
    line[end - begin+1] = '\0';
    return line;
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
        args[n - 1] = NULL; 
    }
    else 
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
    
    int amber_founded = ampersand(args);
    
    pid_t pid, wpid;
    pid = fork();
    
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("Exevcp failed\n");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("Error forking\n");
        exit(EXIT_FAILURE);
    }
    else if(amber_founded!=1) {
        // Parent process
        waitpid(pid, NULL, 0);
    }

    return;
}

void exec_OR(char **command,char **filename){
    
    pid_t pid = fork();
    
    if (pid == 0)
    {
        int fd = open(filename[0], O_CREAT | O_WRONLY, 0666);
        if (fd < 0)
        {
            perror("Open error\n");
            return;
        }
        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            perror("Dup2 error\n");
            return;
        }
        close(fd);
        execvp(command[0], command);
        perror("Execvp failed\n");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("Fork failed\n");
        return;
    }
    else 
        waitpid(pid, NULL, 0);
}
void exec_IR(char** command, char** filename) {
    
    pid_t pid = fork();
  
    if (pid == 0)
    {
        int fd = open(filename[0], O_RDONLY, 0666);
        if (fd < 0)
        {
            perror("Open failed\n");
            return;
        }
        if (dup2(fd, STDIN_FILENO) < 0)
        {
            perror("Dup2 failed");
            return;
        }
        close(fd);
        execvp(command[0], command);
        perror("Execvp failed\n");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("Fork failed\n");
        return;
    }
    else
        waitpid(pid, NULL, 0);

}
void exec_pipe(char**args,char**argspipe) {
    // 0 is read end, 1 is write end 
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) < 0) {
        printf("Pipe could not be initialized\n");
        return;
    }
    pid1 = fork();
    if (pid1 < 0) {
        printf("Could not fork\n");
        return;
    }

    if (pid1 == 0) {
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(args[0], args) < 0) {
            printf("Could not execute command 1..\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        // Parent executing 
        pid2 = fork();

        if (pid2 < 0) {
            printf("\nCould not fork");
            return;
        }

        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (pid2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(argspipe[0], argspipe) < 0) {
                printf("Could not execute command 2..\n");
                exit(EXIT_FAILURE);
            }
        }
        else {
            // parent executing, waiting for two children 
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }
}


void shell_loop() {
    char* line;
    char** args=NULL;
    char** argspipe=NULL;
    int status=1;
    do {
        line = takeinput();
        line = remove_space(line);
        if (line[0] == '\0')
            continue;
        if (strcmp(line, "!!") == 0) {
            if (history == NULL) {
                printf("No commands in history\n");
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
        
        if(type!=1)
            free(argspipe);
        free(args);
        free(line);
    } while (status);
}



int main()
{
    shell_loop();
}
