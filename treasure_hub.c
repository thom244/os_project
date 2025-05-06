#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define SIGLSTH (SIGRTMIN + 1)
#define SIGLSTT (SIGRTMIN + 2)
#define SIGVWT (SIGRTMIN + 3)
#define SIGSTP (SIGRTMIN + 4)

pid_t child_pid;
int monitor_pid;
char hunt_id[100];
char treasure_id[100];

void list_hunts_handler(int sig){
    pid_t pid = fork();
    if (pid == 0) {
        execl("./hunts", "hunts", "list_all", NULL);
        perror("execl failed");
        exit(1);
    }
}

void sigchld_handler(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == monitor_pid) {
            printf("Monitor process %d exited with status %d\n", pid, WEXITSTATUS(status));
            monitor_pid = 0;
        }
    }
}

void list_treasures_handler(int sig){
    char buf[100];
    int fd = open("tmp_args.txt", O_RDONLY);
    if (fd < 0) {
        perror("Error opening in child");
        exit(1);
    }
    int n = read(fd, buf, sizeof(buf)-1);

    close(fd);
    if (n < 0) {
        perror("Error reading in child");
        exit(1);
    }
    buf[n] = '\0';
    pid_t pid = fork();
    if (pid == 0) {
        execl("./hunts", "hunts", "list", buf, NULL);
        perror("execl failed");
        exit(1);
    }
}

void view_handler(int sig){
    char buf[200];
    char local_hunt_id[100], local_treasure_id[100];

    int fd = open("tmp_args.txt", O_RDONLY);
    if (fd < 0) {
        perror("open tmp_args.txt");
        exit(1);
    }
    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n < 0) {
        perror("read tmp_args.txt");
        exit(1);
    }
    buf[n] = '\0';

    sscanf(buf, "%s %s", local_hunt_id, local_treasure_id);
    pid_t pid = fork();
    if (pid == 0) {
        execl("./hunts", "hunts", "view", local_hunt_id, local_treasure_id, NULL);
        perror("execl failed");
        exit(1);
    }
}

void stop_handler(int sig) {
    printf("Received stop signal. Exiting monitor...\n");
    exit(0);
}


void child(){
    struct sigaction list_treasures, list_hunts, view_treasures, stop_monitor;

    memset(&list_treasures, 0x00, sizeof(struct sigaction));
    memset(&list_hunts, 0x00, sizeof(struct sigaction));
    memset(&view_treasures, 0x00, sizeof(struct sigaction));
    memset(&stop_monitor, 0x00, sizeof(struct sigaction));

    list_hunts.sa_handler = list_hunts_handler;
    list_treasures.sa_handler = list_treasures_handler;
    view_treasures.sa_handler = view_handler;
    stop_monitor.sa_handler = stop_handler;


    if (sigaction(SIGLSTH, &list_hunts, NULL) < 0) {
      perror("sigaction SIGLSTH alarm");
      exit(-1);
    }
    if (sigaction(SIGLSTT, &list_treasures, NULL) < 0) {
      perror("sigaction SIGLSTT");
      exit(-1);	     
    }
    if (sigaction(SIGVWT, &view_treasures, NULL) < 0) {
        perror("sigaction SIGVWT");
        exit(-1);	     
    }
    if (sigaction(SIGSTP, &stop_monitor, NULL) < 0) {
        perror("sigaction SIGSTP");
        exit(-1);	     
    }
    printf("Monitor started with PID %d\n", getpid());
    while (1) {
        pause();
    }
}

int main(int argc, char **argv){
    char s[100];

    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = sigchld_handler;
    sigaction(SIGCHLD, &sa_chld, NULL);

    scanf("%s", s);
    while(1){
        fflush(stdout);
        if(strcmp(s, "start_monitor") == 0){
            if (monitor_pid != 0) {
                printf("Monitor is already running\n");
                continue;
            }
            if (system("gcc -Wall -o hunts treasure_manager.c") != 0) {
                fprintf(stderr, "Compilation of treasure_manager.c failed.\n");
                exit(1);
            }            
            if((child_pid = fork()) < 0){
                perror("Error creating child process");
                exit(1);
            }
            else if(child_pid == 0){
                child();
            }
            else{
                monitor_pid = child_pid;
            }
        }
        else if(!strcmp(s, "exit")){
            if(monitor_pid != 0)
                printf("Error: the monitor is still running\n");
            else
                exit(0);
        }
        else if (monitor_pid == 0) {
            printf("The monitor is not started\n");
        }        
        else if(!strcmp(s, "list_hunts")){
            kill(monitor_pid, SIGLSTH);
        }
        else if(!strcmp(s, "list_treasures")){
            scanf("%s", hunt_id);
            int fd = open("tmp_args.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open tmp_args.txt");
                exit(1);
            }
            write(fd, hunt_id, strlen(hunt_id));
            close(fd);
            kill(monitor_pid, SIGLSTT);
        }
        else if(!strcmp(s, "view")){
            scanf("%s %s", hunt_id, treasure_id);
            int fd = open("tmp_args.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open tmp_args.txt");
                exit(1);
            }
            dprintf(fd, "%s %s", hunt_id, treasure_id);
            close(fd);
            kill(monitor_pid, SIGVWT);
        }
        else if(!strcmp(s, "stop_monitor")){
            kill(monitor_pid, SIGSTP);
            int status;
            waitpid(monitor_pid, &status, 0);
            printf("Monitor exited with status %d\n", WEXITSTATUS(status));
            monitor_pid = 0;
        }
        else {
            printf("Unknown command\n");
        }
        scanf("%s", s);
    }
}