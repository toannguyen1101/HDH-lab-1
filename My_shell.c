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
   
    getline(&line, &size, stdin);

    return line;
}
#define _delim " \t\r\n\a"
char** split_line(char*line) {
    int size = 64, position = 0;
    char** tokens = (char**)malloc(size * sizeof(char*));
    char* token;

    token = strtok(line, _delim);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= size) {
            size += 64;
            tokens = (char**)realloc(tokens, size * sizeof(char*));
        }

        token = strtok(NULL, _delim);
    }
    tokens[position] = NULL;
    return tokens;
}

int main()
{
    
}
