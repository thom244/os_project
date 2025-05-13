#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct {
    char name[100];
    int score;
}User;

int numUsers = 0;

void update_score(char *username, int value, User** users){
    if(numUsers == 0){
        *users = (User*)malloc(sizeof(User));
        strcpy((*users)[0].name, username);
        (*users)[0].score = value;
        numUsers++;
        return;
    }
    for(int i = 0; i < numUsers; i++){
        if(!strcmp((*users)[i].name, username)){
            (*users)[i].score += value;
            return;
        }
    }
    numUsers++;
    User *tmp = realloc(*users, (numUsers) * sizeof(User));
    if (tmp == NULL) {
        perror("Realloc failed");
        exit(1);
    }
    *users = tmp;
    strcpy((*users)[numUsers - 1].name, username);
    (*users)[numUsers - 1].score = value;
}

void readfile(char *filename, User** users) {
    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/treasures.txt", filename);
    int fd = open(filepath, O_RDONLY);
    if(fd < 0){
        perror("Error opening the file");
        exit(1);
    }
    char buf[4096];
    int bytes_read;
    int value;
    int count = 0;
    while ((bytes_read = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes_read] = '\0';
        char *line = strtok(buf, "\n");
        char username[100];
        while(line != NULL){
            count++;
            if(count % 6 == 2){
                strcpy(username, line);
            }
            if(count % 6 == 0){
                value = atoi(line);
                update_score(username, value, users);
            }
            line = strtok(NULL, "\n");
        }
    }
    if (bytes_read < 0) {
        perror("Error reading file");
    }
    

    close(fd);
}

int main(int argc, char **argv){
    if(argc != 2){
        perror("Usage: ./exe <hunt file>");
        exit(1);
    }
    User *users = NULL;
    readfile(argv[1], &users);
    for(int i = 0; i < numUsers; i++){
        printf("%s - %d\n", users[i].name, users[i].score);
    }
    free(users);

}