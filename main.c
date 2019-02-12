#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>  

#include "cp.h"

typedef struct {
    char **data;
    int size; 
} NameFiles;

int main (int argc, char **argv) {
        
    char *nameTarget;
    struct stat infoFile;

    int isFile = 0;
    int isDir = 0;
    int isDescription = 0; 

    NameFiles nameSourch;
    nameSourch.data = (char **) malloc(argc * sizeof(char *));
    nameSourch.size = 0;

    for (int i = 0; i < argc - 1; i++) {
        if (!strcmp(argv[i + 1], "-r")) {
            isDescription = 1;
            continue;
        }
        if (argv[i+1][0] == '-')
            continue;
        nameSourch.data[nameSourch.size++] = argv[i+1];
    }
    nameTarget = nameSourch.data[nameSourch.size - 1];
    nameSourch.size--;

    if (nameSourch.size < 1) {
        printf("Missing file operand\n");
        free(nameSourch.data);
        return -1;
    }

    if ((stat(nameTarget, &infoFile)) != 0) {
        if (nameSourch.size > 1 && !isDescription) {
            printf("Cann't find directory %s\n", nameTarget);
            free(nameSourch.data);
            return -1;
        }
        isFile = 1;
    }
    if (S_ISDIR(infoFile.st_mode)) {
        isDir = 1;
    } 
    else if (nameSourch.size > 1 && !isDescription) {
        printf("Cann't find directory %s\n", nameTarget);
        free(nameSourch.data);
        return -1;  
    } 
    else 
        isFile = 1;
  
    if (isFile && isDescription){
        printf("%s isn't a directory\n", nameTarget);
        free(nameSourch.data);
        return -1;
    }

    if (isFile) {
        copyFileToFile(nameSourch.data[0], nameTarget);
    }

    if (isDir && !isDescription) {
        for (int i = 0; i < nameSourch.size; i++) {
            copyFileToDir(nameSourch.data[i], nameTarget);
        }
    }

    if (isDir && isDescription) {
        for (int i = 0; i < nameSourch.size; i++) {
            copyDirToDir(nameSourch.data[i], nameTarget);
        }
    }

    free(nameSourch.data);
    return 0;
}
