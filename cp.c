#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <fcntl.h>      // open() and O_XXX flags 
#include <sys/stat.h>   // S_IXXX flags 
#include <sys/types.h>  // mode_t 
#include <unistd.h>     // close() 
#include <dirent.h>

#include "cp.h"

typedef struct {
    char *name;
    mode_t mode;
    int flag; 
} File;
    
static void initializeFile(File *file, char *name, mode_t mode, int flag) {
    file->name = name;
    file->mode = mode;
    file->flag = flag;
} 

static int m_copyFileToFile(File *sourch, File *target) {
    #define BUF_SIZE 64

    int fdSourch = open(sourch->name, sourch->flag);
    if (fdSourch < 0) {
        printf("Cann't open %s\n", sourch->name);
        return -1;
    }
  
    int fdTarget = open(target->name, target->flag, target->mode);
    if (fdTarget < 0) {
        printf("Cann't open %s\n", target->name);
        close(fdSourch);
        return -1;  
    }

    char *bufferRead = (char *) calloc(BUF_SIZE, sizeof(char *));   
    ssize_t nRead = read(fdSourch, bufferRead, sizeof bufferRead);
    while (nRead > 0) {
        char *bufferWrite = bufferRead;
        while (nRead > 0) {
            ssize_t nWritten = write(fdTarget, bufferWrite, nRead);
            if (nWritten >= 0) {
                nRead = nRead - nWritten;
                bufferWrite = bufferWrite + nWritten;
            } else {
                printf("Cann't write to %s\n", target->name);
                close(fdSourch);
                close(fdTarget);
                free(bufferRead);
                return -1;          
            }
        } 
        nRead = read(fdSourch, bufferRead, sizeof bufferRead);       
    }
    free(bufferRead);

    if (nRead < 0) {
        printf("Cann't read %s\n", sourch->name);
        close(fdSourch);
        close(fdTarget);
        return -1;
    }
    close(fdSourch);
    close(fdTarget);
    return 0;
    #undef BUF_SIZE
}

int copyFileToFile(char *nameSourch, char *nameTarget) {
    struct stat infoFile;
    if (stat(nameSourch, &infoFile) != 0) {
        printf("Cann't find file %s\n", nameSourch);
        return -1;
    }
    if (S_ISDIR(infoFile.st_mode)) {
        printf("Cann't copy directoire %s\n", nameSourch);
        return -1;
    }
    File sourch, target;
    initializeFile(&sourch, nameSourch, infoFile.st_mode, O_RDONLY);
  
    if (stat(nameTarget, &infoFile) != 0) {
        initializeFile(&target, nameTarget, sourch.mode & 0777, O_WRONLY | O_CREAT); 
    }
    else if (S_ISDIR(infoFile.st_mode)) {
        printf("Cann't copy directoire %s\n", nameTarget);
        return -1;
    } 
    else {
        initializeFile(&target, nameTarget, infoFile.st_mode, O_WRONLY | O_TRUNC);
    } 
   
    if (!strcmp(sourch.name, target.name)) {
        printf("%s and %s are the same file\n", sourch.name, target.name);
        return -1;
    }
    return m_copyFileToFile(&sourch, &target);
}

int strsearch(char *string, char symbol) {
	// strchr
    for (int i = strlen(string) - 2; i >= 0; i--) {
        if ((string[i] == symbol))
            return i;
    }
    return -1;
}

int copyFileToDir(char *nameSourch, char *nameDir) {
    int pos = strsearch(nameSourch, '/');
    char *nameTarget = (char *) malloc(strlen(nameDir + 1) * (strlen(nameSourch) - pos) * sizeof(char));
    strcpy(nameTarget, nameDir);
    if (nameDir[strlen(nameDir) - 1] != '/')
        strcat(nameTarget, "/");
    strcat(nameTarget, &nameSourch[pos + 1]);
    int n = copyFileToFile(nameSourch, nameTarget);
    free(nameTarget);
    return n;                    
}

int copyDirToDir(char *nameSourch, char *nameDir) {
    DIR *dir;
    struct dirent *entry;
    struct stat infoDir;
    int pos = -1;

    if (stat(nameSourch, &infoDir) != 0) {
        printf("Cann't find directoire %s\n", nameSourch);
        return -1;
    }
    if (!S_ISDIR(infoDir.st_mode)) {
        printf("%s\n isn't a directoire", nameSourch);
        return -1;
    }

    if (stat(nameDir, &infoDir) != 0) {
        mkdir(nameDir, S_IRWXU);
    }  
  
    char *targetPathName = (char *) malloc( (strlen(nameDir) + 1) * (strlen(nameSourch) + 1) * sizeof(char) ); 
    pos = strsearch(nameSourch, '/');
  
    strcpy(targetPathName, nameDir);
    strcat(targetPathName, "/");
    strcat(targetPathName, &nameSourch[pos + 1]); 
    mkdir(targetPathName, S_IRWXU);

    dir = opendir(nameSourch);
    if (dir == NULL) {
        printf("Cann't open directoire %s\n", nameSourch);
        return -1;
    }

    entry = readdir(dir); 
    char *sourchPathName = (char *) malloc((strlen(nameDir) + 1) * (strlen(nameSourch) + 1) * (PATH_MAX) * sizeof(char)); 
    while ((entry != NULL)) {
        if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
            entry = readdir(dir);
            continue;
        }
    
        strcpy(sourchPathName, nameSourch);
        strcat(sourchPathName, "/");
        strcat(sourchPathName, entry->d_name);

        if (stat(sourchPathName, &infoDir) == 0) {
            if (S_ISDIR(infoDir.st_mode)) {
                copyDirToDir(sourchPathName, targetPathName);
            } 
            else {
                copyFileToDir(sourchPathName, targetPathName);
            }
        }
        else {
            printf("Cann't find file or directoire %s\n", sourchPathName);
            free(sourchPathName);
            free(targetPathName);
            closedir(dir);
            return -1;
        }
        entry = readdir(dir);
    }
    free(sourchPathName);
    free(targetPathName);
    closedir(dir);
    return 0; 
}
