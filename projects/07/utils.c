#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "limits.h"

#include "utils.h"

char buff[PATH_MAX];

char *mystrcat(char *dest, char *src){
    while(*dest) dest++;
    while(*dest++ = *src++){
        printf(".");
    };
    printf("\n");
    /* return dest; */
    return --dest;
}

void CutStringAfterPattern(char *string_to_cut, char *pattern){
    int original_idx = 0;
    int comparison_idx = 0;

    while(string_to_cut[original_idx] != '\0'){
        if(string_to_cut[original_idx] == pattern[comparison_idx]) {
            comparison_idx++;
            if(pattern[comparison_idx] == '\0'){
                string_to_cut[original_idx + 1] = '\0';
                return;
            }
        }
        else comparison_idx = 0;
        original_idx++;
    }
}

const char *GetFullPath(char *path){
    printf("Evaluating %s\n", path);
    char *res = realpath(path, buff);
    if(res){
        printf("Full path is %s\n", res);
    } else {
        printf("Error in realpath method.\n");
        return NULL;
    }
    return buff;
}

const char* GetExecutableDir() {
    ssize_t len = readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
      buff[len] = '\0';
    }
    CutStringAfterPattern(buff, "transverse");
    return buff;
}



FILE *LoadFile(const char *path, char *mode){
    FILE *fptr = fopen(path, mode);
    if (fptr == NULL) {
        printf ("File %s failed to open. Abording...\n", path);
        exit(EXIT_FAILURE);
    } else {
        printf("File %s loaded successfully.\n", path);
    }

    return fptr;
}

void WriteToFile(char *path, char *content){
    FILE *fptr = LoadFile(path, "w");
    if (fptr != NULL) {
        fprintf(fptr, content);
        fclose(fptr);
        /* fwrite(content, result_buffer_size, 3, fptr); */
    }
}
