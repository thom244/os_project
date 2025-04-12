#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct Treasure{
    char *treasure_id;
    char *username;

    struct GPS{
        float latitude;
        float longitude;
    }GPS;

    char *clue;
    int value;
}Treasure;


int options(char *s){
    if(!strcmp(s, "add"))
        return 10;
    else if(!strcmp(s, "list"))
        return 11;
    else if(!strcmp(s, "view"))
        return 12;
    else if(!strcmp(s, "remove_treasure"))
        return 13;
    else if(!strcmp(s, "remove_hunt"))
        return 14;
    else
        return 15;
}

void add_hunt(char *hunt_id, Treasure treasure){
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/");
    strcat(cwd, hunt_id);
    struct stat st = {0};
    if (stat(cwd, &st) == -1) {
        if (mkdir(cwd, 0777) == -1) {
            perror("Error creating directory");
            exit(1);
        }
    }

    char file_path[1280];
    snprintf(file_path, sizeof(file_path), "%s/treasures.txt", cwd);

    int file = open(file_path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (file == -1) {
        perror("Error opening file");
        exit(1);
    }

    char buffer[4096];
    int len = snprintf(buffer, sizeof(buffer), "%s\n%s\n%f\n%f\n%s\n%d\n", treasure.treasure_id,
              treasure.username, treasure.GPS.latitude, treasure.GPS.longitude, treasure.clue, treasure.value);

    if (write(file, buffer, len) == -1) {
        perror("Error writing to file");
        close(file);
        exit(1);
    }
    printf("Treasure added successfully to %s\n", file_path);
    close(file);
}


void remove_treasure(char *hunt_id, char *treasure_id) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/");
    strcat(cwd, hunt_id);

    char file_path[1280];
    snprintf(file_path, sizeof(file_path), "%s/treasures.txt", cwd);

    int file = open(file_path, O_RDONLY);
    if (file < 0) {
        perror("Cannot open file for reading");
        return;
    }

    char temp_file_path[1280];
    snprintf(temp_file_path, sizeof(temp_file_path), "%s/temp_treasures.txt", cwd);
    int temp_file = open(temp_file_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (temp_file < 0) {
        perror("Cannot open temp file for writing");
        close(file);
        return;
    }

    char buf[4096];
    ssize_t bytes_read;
    char *line;
    int found = 0;

    while ((bytes_read = read(file, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes_read] = '\0';
        line = strtok(buf, "\n");

        while (line != NULL) {
            char current_treasure_id[256];
            strncpy(current_treasure_id, line, sizeof(current_treasure_id) - 1);
            current_treasure_id[sizeof(current_treasure_id) - 1] = '\0';

            if (strcmp(current_treasure_id, treasure_id) == 0) {
                found = 1;
                for (int i = 0; i < 5; i++) {
                    line = strtok(NULL, "\n");
                    if (line == NULL) break;
                }
            } else {
                write(temp_file, line, strlen(line));
                write(temp_file, "\n", 1);

                for (int i = 0; i < 5; i++) {
                    line = strtok(NULL, "\n");
                    if (line == NULL) break;
                    write(temp_file, line, strlen(line));
                    write(temp_file, "\n", 1);
                }
            }

            line = strtok(NULL, "\n");
        }
    }

    close(file);
    close(temp_file);

    if (!found) {
        printf("Treasure with ID '%s' not found in hunt '%s'\n", treasure_id, hunt_id);
        remove(temp_file_path);
        return;
    }

    if (rename(temp_file_path, file_path) == -1) {
        perror("Error replacing original file");
        remove(temp_file_path);
        return;
    }

    printf("Treasure with ID '%s' removed successfully from hunt '%s'\n", treasure_id, hunt_id);
}

void list_treasures(char *filepath) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/");
    strcat(cwd, filepath);

    struct dirent *entry;
    DIR *dir = opendir(cwd);
    if (!dir) {
        perror("Cannot open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {

            char full_path[1280];
            snprintf(full_path, sizeof(full_path), "%s/%s", cwd, entry->d_name);

            int file = open(full_path, O_RDONLY, 0644);
            if (file < 0) {
                perror("Cannot open file");
                continue;
            }

            char buf[4096];
            ssize_t bytes_read;
            while ((bytes_read = read(file, buf, sizeof(buf) - 1)) > 0) {
                buf[bytes_read] = '\0';
                printf("%s", buf);
            }
            if (bytes_read < 0) {
                perror("Error reading file");
            }
            printf("\n");
            close(file);
        }
    }
    closedir(dir);
}

void view_treasure(char* hunt_id, char* treasure_id){
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/");
    char full_path[1280];
    snprintf(full_path, sizeof(full_path), "%s%s/treasures.txt", cwd, hunt_id);

    int file = open(full_path, O_RDONLY);
    if (file < 0) {
        printf("Cannot open file %s\n", full_path);
        perror("Cannot open file");
        return;
    }
    char buf[4096];
    char *line;
    int found = 0;
    ssize_t bytes_read;
    while ((bytes_read = read(file, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes_read] = '\0';
        line = strtok(buf, "\n");

        while (line != NULL) {
            char current_treasure_id[256];
            strncpy(current_treasure_id, line, sizeof(current_treasure_id) - 1);
            current_treasure_id[sizeof(current_treasure_id) - 1] = '\0';

            if (strcmp(current_treasure_id, treasure_id) == 0) {
                printf("%s\n", line);
                found = 1;
                for (int i = 0; i < 5; i++) {
                    line = strtok(NULL, "\n");
                    printf("%s\n", line);
                    if (line == NULL) break;
                }
                break;
            }
            else {
                for (int i = 0; i < 5; i++) {
                    line = strtok(NULL, "\n");
                    if (line == NULL) break;
                }
            }
            line = strtok(NULL, "\n");
        }
    }
    if (!found) {
        printf("Treasure with ID '%s' not found in hunt '%s'\n", treasure_id, hunt_id);
        return;
    }

    close(file);
}

void read_treasures(char *filename, char *hunt_id) {
    int file = open(filename, O_RDONLY);
    if (file < 0) {
        perror("Error opening file");
        return;
    }

    char buf[4096];
    ssize_t bytes_read;
    char *line;
    Treasure treasure;

    while ((bytes_read = read(file, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes_read] = '\0';
        line = strtok(buf, "\n");

        while (line != NULL) {
            treasure.treasure_id = strdup(line);

            line = strtok(NULL, "\n");
            if (line == NULL) break;
            treasure.username = strdup(line);

            line = strtok(NULL, "\n");
            if (line == NULL) break;
            treasure.GPS.latitude = atof(line);

            line = strtok(NULL, "\n");
            if (line == NULL) break;
            treasure.GPS.longitude = atof(line);

            line = strtok(NULL, "\n");
            if (line == NULL) break;
            treasure.clue = strdup(line);

            line = strtok(NULL, "\n");
            if (line == NULL) break;
            treasure.value = atoi(line);

            add_hunt(hunt_id, treasure);

            line = strtok(NULL, "\n");
        }
    }

    if (bytes_read < 0) {
        perror("Error reading file");
    }

    close(file);
}

void remove_hunt(char *hunt_id){
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/");
    strcat(cwd, hunt_id);

    struct dirent *entry;
    DIR *dir = opendir(cwd);

    if (!dir)   return;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char full_path[1280];
            snprintf(full_path, sizeof(full_path), "%s/%s", cwd, entry->d_name);
            struct stat statbuf;
            if (stat(full_path, &statbuf) == -1) continue;

            if (S_ISDIR(statbuf.st_mode)) {
                remove_hunt(full_path);
                rmdir(full_path);
            } else {
                remove(full_path);
            }
        }
    }
    closedir(dir);
    int i;
    if((i = rmdir(cwd)) != 0){
        printf("Cannot remove the %s directory\n", cwd);
    }
}

int main(int argc, char **argv){
    
    switch(options(argv[1])){
        case 10:{
            if (argc != 3) {
                perror("Usage: add <hunt_id>\n");
                exit(1);
            }
            
            char file_name[256];
            printf("If you want to read from a file, enter the file name, otherwise press 1\n");
            scanf("%s", file_name);

            int c;
            while ((c = getchar()) != '\n' && c != EOF);

            if (strcmp(file_name, "1") != 0) {
                read_treasures(file_name, argv[2]);
                break;
            }

            Treasure treasure;
            char buffer[256];

            printf("Enter treasure ID: ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';
            treasure.treasure_id = strdup(buffer);
        
            printf("Enter username: ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';
            treasure.username = strdup(buffer);
        
            printf("Enter latitude: ");
            fgets(buffer, sizeof(buffer), stdin);
            treasure.GPS.latitude = atof(buffer);
        
            printf("Enter longitude: ");
            fgets(buffer, sizeof(buffer), stdin);
            treasure.GPS.longitude = atof(buffer);
        
            printf("Enter clue: ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';
            treasure.clue = strdup(buffer);
        
            printf("Enter value: ");
            fgets(buffer, sizeof(buffer), stdin);
            treasure.value = atoi(buffer);
        
            add_hunt(argv[2], treasure);
            break;
            
        }
        case 11:{
            if(argc != 3){
                perror("Usage: list <hunt_id>\n");
                exit(1);
            }
            list_treasures(argv[2]);
            break;
        }
        case 12:{
            if(argc != 4){
                perror("Usage: view <hunt_id> <id>\n");
                exit(1);
            }
            view_treasure(argv[2], argv[3]);
            break;
        }
        case 13:{
            if(argc != 4){
                perror("Usage: remove_treasure <hunt_id> <id>\n");
                exit(1);
            }
            remove_treasure(argv[2], argv[3]);
            break;
        }
        case 14:{
            if(argc != 3){
                perror("Usage: remove_hunt <hunt_id>\n");
                exit(1);
            }
            remove_hunt(argv[2]);
            break;
        }
        default:{
            printf("Unknown command \"%s\"\n", argv[1]);
            exit(1);
        }
    }
    return 0;
}
