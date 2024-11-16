#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

#ifndef __HY345_SHELL_H__
/**
 * @brief Interface for definitions, types and debug modules
 */
#define __HY345_SHELL_H__

/* Types */
typedef char* pseudoString;
typedef struct shellVariable{
    pseudoString varName;
    pseudoString varValue;
}shellVar;

/* Definitions */
#define catalogSize 500
#define MAX_INPUT_SIZE 4000
#define RETURN_SUC 0
#define RETURN_FAI 1
#define ENV_OVERWRITE 1
#define ENV_NO_OVERWRITE 0
#define RUNNING 1
#define NO_APPEND 0
#define APPEND 1
#define REDIRECT_INPUT 96
#define REDIRECT_OUTPUT 69
#define REDIRECT_NONE 0

/* Debug */
#define TOKEN_DEBUG

#endif /* __HY345_SHELL_H__ */
