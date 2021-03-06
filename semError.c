#include "semError.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ErrorList errorList;


void addError(char* msg){
    if(errorList.size == errorList.cap){
        errorList.cap = (int)(errorList.cap * 1.4 + 1);
        errorList.errors = realloc(errorList.errors, errorList.cap * sizeof(char*));
    }
    errorList.errors[errorList.size] = msg;
    errorList.size++;
}

void initErrorList(){
    errorList.cap = 1;
    errorList.size = 0;
    errorList.errors = malloc(sizeof(char*));
}

void error(char* msg){
    char* msg2 = malloc(strlen(msg) * sizeof(char));
    memcpy(msg2, msg, strlen(msg) * sizeof(char));

    addError(msg2);
}

void reportErrors(){
    for(int i = 0; i < errorList.size; i++){
        printf("%s\n", errorList.errors[i]);
    }
}
