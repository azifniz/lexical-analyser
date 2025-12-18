#ifndef HEADER_H
#define HEADER_H

//structure to store file , tokens and linecount.
typedef struct st
{
    char *file_name;                                                    //to keep the file name
    FILE *file_ptr;

    char current_token[100];                                            //to store the token

    int line;                                                           //to keep the line count
}storage;

//helpers
int validate_arg(char *argv[], storage *token);
void categorizeToken(storage* token);

//checks
int isKeyword(const char* str);
int isOperator(const char* str);
int isSpecialCharacter(const char *ch);
int isConstant(const char* str, int line);
int isIdentifier(const char* str);


#endif