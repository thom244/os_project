#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Treasure{
    int treasure_id;
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

void add_hunt(int hunt_id){

}


int main(int argc, char **argv){
    printf("%d\n", options(argv[1]));
    switch(options(argv[1])){
        case 10:{
            if(argc != 3 || !atoi(argv[2])){
                perror("Usage: add <hunt_id>\n");
                exit(1);
            }
            add_hunt(atoi(argv[2]));
        }
        case 11:{
            if(argc != 3 || !atoi(argv[2])){
                perror("Usage: list <hunt_id>\n");
                exit(1);
            }
            //list_treasures(atoi(argv[2]));
        }
        case 12:{
            if(argc != 4 || !atoi(argv[2]) || !atoi(argv[3])){
                perror("Usage: view <hunt_id> <id>\n");
                exit(1);
            }
            //view_treasure(atoi(argv[2]), atoi(argv[3]));
        }
        case 13:{
            if(argc != 4 || !atoi(argv[2]) || !atoi(argv[3])){
                perror("Usage: remove_treasure <hunt_id> <id>\n");
                exit(1);
            }
            //remove_treasure(atoi(argv[2]), atoi(argv[3]));
        }
        case 14:{
            if(argc != 3 || !atoi(argv[2])){
                perror("Usage: remove_hunt <hunt_id>\n");
                exit(1);
            }
            //remove_hunt(atoi(argv[2]));
        }
        default:{
            printf("Unknown command \"%s\"\n", argv[1]);
            exit(1);
        }
    }
}
