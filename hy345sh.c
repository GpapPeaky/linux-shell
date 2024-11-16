#include "hy345sh.h"
/**
 * @version 1.0
 * 
 * @author GpapPeaky George Papamatthaiakis (https://github.com/GpapPeaky)
 * CSD5328
 * 
 * @note Homework 1 for hy-345
 */

pseudoString STUDENT;
pseudoString USER;
pseudoString CATALOG;

pseudoString fileParameters;
pseudoString fname;
int appendFlag;
int redirectionIndication;

/**
 * @brief Debug print
 */
void printDebug(void){
    printf("<dbg>\n");
}

/**
 * @brief Initialises the promt
 * 
 * @returns 0 on success, 1 on failure
 */
int initPromt(void){
    STUDENT = "csd5328";
    USER = getlogin();
    if(USER == NULL){ printf("User not initialised correctly\n"); return RETURN_FAI; }
    CATALOG = getcwd(CATALOG, catalogSize);
    if(CATALOG == NULL){ printf("Catalog not initialised correctly\n"); return RETURN_FAI; }

    return RETURN_SUC;
}

/**
 * @brief Prints the promt's items
 */
void printPromt(void){
    printf("<%s>-hy345@<%s>:<%s>\n", STUDENT, USER, CATALOG);
    return;
}

/**
 * @brief Trims the whitespace off a string
 * 
 * @param str string to trim
 * 
 * @returns trimmed string
 */
pseudoString trimWhitespace(pseudoString str){
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;
    pseudoString end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    *(end + 1) = '\0';
    return str;
}

/**
 * @brief Trims the quotation marks off a string
 * 
 * @param str string to trim
 * 
 * @returns trimmed string
 */
pseudoString trimQuotationMarks(pseudoString str){
    size_t len = strlen(str);
    size_t newIndex = 0;  /* Index for the modified string */

    /* Loop through the original string */
    for (size_t i = 0; i < len; ++i) {
        /* If the current character is not a quotation mark, copy it to the new position */
        if (str[i] != '"') {
            str[newIndex++] = str[i];
        }
    }

    /* Null-terminate the resulting string */
    str[newIndex] = '\0';

    return str;
}

/**
 * @brief Executes the redirection of there is one
 * 
 * @param fileParameters parameters of the command to redirect
 * @param fname file to redirect from or to
 * @param appendFlag flag for appending
 * @param redirectionIndication indicator if we have input or output
 */
void redirectionExecute(pseudoString fileParameters, pseudoString fname, int appendFlag, int redirectionIndication){

    int fileDescriptor; /* File desciptor */
    int procId = fork();

    pseudoString command = strtok(fileParameters, " ");
    pseudoString args[MAX_INPUT_SIZE];
    int idx = 0;

    while (command != NULL) {
        args[idx++] = command;
        command = strtok(NULL, " ");
    }
    args[idx] = NULL;

    #ifdef TOKEN_DEBUG
        printf("Executing command: %s with args:\n", args[0]);
        for(int i = 0; i < idx; i++){
            printf("Arg[%d]: %s\n", i, args[i]);
        }
    #endif

    if(procId == 0){
        if(redirectionIndication == REDIRECT_INPUT){
            fileDescriptor = open(fname, O_RDONLY);
            if(fileDescriptor < 0){
                perror("Failed to open input file\n");
                exit(EXIT_FAILURE);
            }

            dup2(fileDescriptor, STDIN_FILENO);
            close(fileDescriptor);
        }

        if(redirectionIndication == REDIRECT_OUTPUT){
            if(appendFlag == APPEND){
                /* Unorthodox syntax, kind of */
                fileDescriptor = open(fname, O_WRONLY | O_APPEND | O_CREAT, 0644); /* Open file in append mode */
            }else{
                fileDescriptor = open(fname, O_WRONLY | O_TRUNC | O_CREAT, 0644);  /* Open file in write mode (overwrite) */
            }

            if(fileDescriptor < 0){
                perror("Failed to open output file\n");
                exit(EXIT_FAILURE);
            }

            dup2(fileDescriptor, STDOUT_FILENO);
            close(fileDescriptor);
        }

        if(execvp(args[0], args) == -1){
            exit(EXIT_FAILURE);
        }
    }else if(procId > 0){
        int status;
        wait(&status);
    }else{
        printf("Fork failed, process terminated\n");
    }

    return;
}

/**
 * @brief Tokenises the input of the terminal, and returns the tokens to 2 different arrays of char*. One
 * that saves the parameters of the input string, and one that saves the command called.
 * 
 * @param input input to tokenise
 * @param commands array of commands to execute (1D-pseudoStrings)
 * @param parameters array of parameters (2D-pseudoStrings)
 * 
 * @returns 0 on success, 1 on failure
 */
int tokeniseInput(pseudoString input, pseudoString* commands, pseudoString** parameters){
    pseudoString* tempArray = (pseudoString*)malloc(MAX_INPUT_SIZE * sizeof(pseudoString));
    if(tempArray == NULL){ printf("Temporary array for the input string, \
    not initialised correctly\n"); return RETURN_FAI; }

    shellVar variable; /* Variable */
    pseudoString token = strtok(input, ";"); /* First iteration to initialise the tokenisation loop */
    int commandCount = 0;

    /* We first tokenise to seperate the input from ';' */
    while(token != NULL){
        pseudoString trimmedCommand = trimWhitespace(token); /* Trim the token */
        tempArray[commandCount++] = trimmedCommand;  /* copy the token */
        token = strtok(NULL, ";");
    }

    /*________________________________________________________________________________________________________________________*/

    /* We check for assignments in the */
    for(int i = 0 ; i < commandCount ; i++){
        int parameterCount = 0;
        pseudoString currentCommand = tempArray[i];

        if(strchr(currentCommand, '<') != NULL || strchr(currentCommand, '>') != NULL){  /* Redirection found */
            #ifdef TOKEN_DEBUG
                printf("Redirection Found\n");
            #endif

            char redirectionChar;
            fname = (pseudoString)malloc(MAX_INPUT_SIZE * sizeof(char));
            fileParameters = (pseudoString)malloc(MAX_INPUT_SIZE * sizeof(char));
            appendFlag = NO_APPEND; /* later use */
            redirectionIndication = REDIRECT_NONE;
            char commandCopy[MAX_INPUT_SIZE];

            strncpy(commandCopy, currentCommand, sizeof(commandCopy));
            commandCopy[sizeof(commandCopy) - 1] = '\0'; // Ensure null-termination

            for(int p = 0 ; commandCopy[p] != '\0' ; p++){
                redirectionChar = commandCopy[p];
                #ifdef TOKEN_DEBUG
                    printf("Parameters[%d]: %c\n", p, commandCopy[p]);
                #endif
                switch(redirectionChar){
                    case '<': /* Input redirection */
                        fileParameters = trimWhitespace(strtok(commandCopy, "<")); /* We have the file parameters, and command */
                        fname = trimWhitespace(strtok(NULL, " \t\n")); /* We parse the file name, with delimeter the whitespace */
                        #ifdef TOKEN_DEBUG
                            printf("File and parameters: %s [%s]\n", fname, fileParameters);
                        #endif
                        redirectionIndication = REDIRECT_INPUT;

                        break;
                    case '>': /* Output redirection */
                        if(commandCopy[p + 1] == '>'){ /* Double >>, append */
                            fileParameters = trimWhitespace(strtok(commandCopy, ">"));

                            p++; /* Skip the next '>' */

                            fname = trimWhitespace(strtok(&commandCopy[p + 1], " \t\n")); /* Start where the file name begins */
                            appendFlag = APPEND;
                            #ifdef TOKEN_DEBUG
                                printf("Appending file: %s [%s]\n", fname, fileParameters);
                            #endif
                        }else{
                            fileParameters = trimWhitespace(strtok(commandCopy, ">")); 
                            fname = trimWhitespace(strtok(&commandCopy[p + 1], " \t\n"));
                            #ifdef TOKEN_DEBUG
                                printf("Overwriting file: %s [%s]\n", fname, fileParameters);
                            #endif
                        }
                        
                        redirectionIndication = REDIRECT_OUTPUT;
                        break;
                    default:
                        /* Nothing happens, no redirection found */
                        break;
                }
            }

            redirectionExecute(fileParameters, fname, appendFlag, redirectionIndication);
        }

        /*________________________________________________________________________________________________________________________*/

        /* We first need to check for '=', in order to implement global variables
        then we check for ' ' */
        if(/* Toyota CHR ahh function */ strchr(currentCommand, '=') != NULL){
            /* Tokensise the variable name, as it will stop before it finds the '=' */
            variable.varName = trimWhitespace(strtok(currentCommand, "="));
            /* Tokenise the remaining string for the value */
            variable.varValue = trimWhitespace(strtok(NULL, "="));

            /* Remove quotation marks if found */
            if(variable.varValue[0] == '"'){
                variable.varValue = trimQuotationMarks(variable.varValue);
                #ifdef TOKEN_DEBUG
                    printf("%s\n", variable.varValue);
                #endif
            }

            if(variable.varValue != NULL && variable.varName != NULL){
                setenv(variable.varName, variable.varValue, ENV_OVERWRITE);
                #ifdef TOKEN_DEBUG
                    printf("Name / Value - %s / %s at %d\n", variable.varName, variable.varValue, i);
                #endif
                commands[i] = "O"; /* Out of bounds value, since O command doesn't exist */
            }else{
                #ifdef TOKEN_DEBUG
                    printf("Failed to assign the shell variable %s\n", variable.varName);
                #endif
            }

            continue; /* We continue to the next command produced by the first tokenisation, since the global variable assignment 
            command, is inherently different than the others */

            #ifdef TOKEN_DEBUG
                printf("Unreachable\n");
            #endif
        }

        /*________________________________________________________________________________________________________________________*/

        pseudoString parameterToken = strtok(currentCommand, " ");
        commands[i] = parameterToken; /* Assign the first token as the command */
        #ifdef TOKEN_DEBUG
            printf("Command %s assigned at %d\n", commands[i], i);
        #endif

        /* We know tokenise to seperate the produced substring from ' ' */
        while(parameterToken != NULL){
            /* Check if in a parameter the dollar sign is typed */
            if(parameterToken[0] == '$'){
                pseudoString enviromentVariableName = parameterToken + 1; /* Ignore the '$' notation */
                pseudoString enviromentVariableValue = getenv(enviromentVariableName); /* Get the value */

                if(enviromentVariableValue != NULL){
                    parameters[i][parameterCount++] = enviromentVariableValue;
                }
            }else{
                parameters[i][parameterCount++] = parameterToken;
            }

            /* Continue removing the whitespaces */
            parameterToken = strtok(NULL, " ");
        }

        parameters[i][parameterCount] = NULL; /* For graceful termination */

        #ifdef TOKEN_DEBUG
            printf("Command[%d]: %s\n", i, commands[i]);
            for(int j = 0 ; j < parameterCount ; j++){
                printf("\tParameters[%d] -> %s\n", j, parameters[i][j]);
            }
        #endif
    }

    /* Free allocated memory */
    free(tempArray);

    return RETURN_SUC;
}

/**
 * @brief Reads from the promt, and completes the correct calculations and evaluations
 * 
 * @param input input to tokenise
 * @param commands array of commands to execute
 * @param parameters array of parameters for the commands
 * 
 * @returns 0 on success, 1 on failure
 */
int readAndExecute(pseudoString input, pseudoString* commands, pseudoString** parameters){
    printPromt();
    if(fgets(input, MAX_INPUT_SIZE, stdin) == NULL){
        printf("Error reading input for stdin\n");
        return RETURN_FAI;
    }

    input[strcspn(input, "\n")] = '\0'; /* Remove newline */

    if(tokeniseInput(input, commands, parameters) == RETURN_FAI){
        printf("Unable to tokenise input\n");
        return RETURN_FAI;
    }

    for(int commandCount = 0; commands[commandCount] != NULL; commandCount++){
        int procId = fork();
        if(procId == 0){
            if(strcmp(commands[commandCount], "O") != 0){ /* OUT OF BOUNDS */
                execvp(commands[commandCount], parameters[commandCount]);
                #ifdef TOKEN_DEBUG
                    printf("->EXECUTING COMMAND %d\n", commandCount); /* HUH?: This prints after execvp()?? */
                #endif
            }
            exit(EXIT_FAILURE);
        }else if (procId > 0){
            int status;
            wait(&status);
        }else{
            printf("Fork failed, process terminated\n");
        }
    }

    return RETURN_SUC;
}

int main(void){
    pseudoString input = (pseudoString)malloc(MAX_INPUT_SIZE * sizeof(char));
    pseudoString* command = (pseudoString*)malloc(MAX_INPUT_SIZE * sizeof(pseudoString));
    pseudoString** parameters = (pseudoString**)malloc(MAX_INPUT_SIZE * sizeof(pseudoString*));
    for(int i = 0 ; i < MAX_INPUT_SIZE ; i++){
        parameters[i] = (pseudoString*)malloc(MAX_INPUT_SIZE * sizeof(pseudoString)); /* Allocate memory for ther other dimension */
    }

    /* Initialise promt print values */
    initPromt();

    while(RUNNING){
        readAndExecute(trimQuotationMarks(input), command, parameters);
    }

    /* Free used memory */
    free(input);
    free(command);
    for(int i = 0 ; i < MAX_INPUT_SIZE ; i++){
        free(parameters[i]);
    }
    free(parameters);

    return RETURN_SUC;
}
