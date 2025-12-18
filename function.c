#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "header.h"

//keywords in c
static const char* keywords[32] = 
{
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
};

//operators in c
static const char* operators[38] = 
{
    "+", "-", "*", "/", "%", "=", "==", "!=", ">", "<", ">=", "<=",
    "&&", "||", "!", "&", "|", "^", "~", "<<", ">>",
    "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=",
    "++", "--", "->", ".", "?", ":", ","
};

//special characters
static const char* specialCharacters = "(),;:{}[]#\"'\\.";

//argument validation function
int validate_arg(char *argv[], storage *token)
{
    if (argv[1] == NULL)                                                //if null
    {
        printf("## Error : No file is found\n");
        return 0;
    }
    else if (strstr(argv[1], ".c") != NULL)                             //checking if the file is a c file
    {
        token->file_name = argv[1];                                     //stroring the token
        token->file_ptr = fopen(token->file_name, "r");                 //opening the file

        if (token->file_ptr == NULL)
        {
            printf("## Error : Unable to open file\n");
            return 0;
        }

        return 1;
    }
    else
    {
        printf("## Error : Invalid file , the file should be a .c file\n");
        return 0;
    }
}

//function to characterise tokens
void categorizeToken(storage* t)
{
    char *token = t->current_token;                                             //getting the token

    if (isKeyword(token))                                                       //keyword
    {
        printf(">> KEYWORD             : %s\n", token);
        return;
    }

    if (isIdentifier(token))                                                    //identifier
    {
        printf(">> IDENTIFIER          : %s\n", token);
        return;
    }

    int c = isConstant(token, t->line);                                         //constant
    if (c == 1)
    {
        printf(">> CONSTANT            : %s\n", token);
        return;
    }
    else if (c == -1)
    {
        // Error message printed inside isConstant()
        return;   // DO NOT classify further
    }

    if (isOperator(token))                                                      //operator
    {
        printf(">> OPERATOR            : %s\n", token);
        return;
    }

    if (isSpecialCharacter(token))                                              //special character
    {
        printf(">> SPECIAL CHARACTER   : %s\n", token);
        return;
    }

    printf("## ERROR (line %d)      : Unrecognized token '%s'\n", t->line, token);

}



//function to comparer the keyword
int isKeyword(const char* str)
{
    for(int i=0; i<32; i++)
    {
        if(strcmp(str,keywords[i]) == 0)                        //comparing with the keywords array
            return 1;
    }
    return 0;
}

//function for identifiers
int isIdentifier(const char* str)
{
    if (!(isalpha(str[0]) || str[0] == '_'))                    //First char must be alphabet or underscore
        return 0;

    for (int i = 1; str[i] != '\0'; i++)                        //Remaining chars must be alphanumeric or underscore
    {
        if (!(isalnum(str[i]) || str[i] == '_'))
            return 0;
    }

    if (isKeyword(str))                                        //if it is keyword it is not identifier
        return 0;

    return 1;   
}


//function for special characters
int isSpecialCharacter(const char *ch)
{
    return (strlen(ch) == 1 && strchr(specialCharacters, ch[0]) != NULL);
}


//function for operators
int isOperator(const char* str)
{
    for (int i = 0; i < 38; i++)
    {
        if (strcmp(str, operators[i]) == 0)                       //comparing with the operators array
            return 1;
    }
    return 0;
}


//functions to check constants and their errors.
int isConstant(const char* str, int line)
{
    int len = strlen(str);
    if (len == 0)                                                //if token is empty
        return 0;

    
    int starts_like_constant = 0;                                //to checking valid start

    if (isdigit(str[0]) || str[0] == '\'' || str[0] == '\"')
    {
        starts_like_constant = 1;                                // valid start
    }
    else if ((str[0] == '+' || str[0] == '-') && isdigit(str[1]))
    {
        starts_like_constant = 1;                                // +10, -20, +3.14
    }

    if (!starts_like_constant)                                  // if not a constant
    {
        return 0;                                               
    }

     
    if (str[0] == '\"' && str[len - 1] == '\"')                 //if string constant
        return 1;

    // CHARACTER CONSTANT ----------------                      //error checking
    if (str[0] == '\'' && str[len - 1] == '\'')
    {
        if (len == 3)                                           // 'a'
            return 1;
        if (len == 4 && str[1] == '\\')                         // '\n'
            return 1;

        printf("## ERROR at line %d     : Invalid numeric constant %s\n", line, str);
        return -1;
    }

    //HEX CONSTANT ----------------                             //error checking
    if (len > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        for (int j = 2; j < len; j++)
        {
            if (!isxdigit(str[j]))
            {
                printf("## ERROR at line %d     : Invalid hex constant %s\n", line, str);
                return -1;                                      //error
            }
        }
        return 1;                                               //constant
    }       

    // BINARY CONSTANT ----------------                         //error checking
    if (len > 2 && str[0] == '0' && (str[1] == 'b' || str[1] == 'B'))
    {
        for (int j = 2; j < len; j++)
        {
            if (str[j] != '0' && str[j] != '1')
            {
                printf("## ERROR at line %d     : Invalid binary constant %s\n", line, str);
                return -1;
            }
        }
        return 1;
    }

    // OCTAL CONSTANT ----------------                          //error checking
    if (str[0] == '0' && len > 1 && strchr(str, '.') == NULL && str[1] != 'x' && str[1] != 'X' && str[1] != 'b' && str[1] != 'B')
    {
        for (int j = 1; j < len; j++)
        {
            if (str[j] < '0' || str[j] > '7')
            {
                printf("## ERROR at line %d     : Invalid octal constant %s\n", line, str);
                return -1;
            }
        }
        return 1;
    }

    // INTEGER / FLOAT ----------------                         //error checking
    int dot_seen = 0;                                           //flags
    int exp_seen = 0;
    int digit_seen = 0;

    for (int j = 0; j < len; j++)
    {
        char c = str[j];

        if (isdigit(c))                                         //if digit
        {
            digit_seen = 1;
            continue;
        }

        if (c == '.' && !dot_seen)                              //if there is dot
        {
            dot_seen = 1;
            continue;
        }

        if ((c == 'e' || c == 'E') && digit_seen && !exp_seen)  //handle exponents
        {
            exp_seen = 1;
            j++;

            if (str[j] == '+' || str[j] == '-')
                j++;

            if (!isdigit(str[j]))
            {
                printf("## ERROR at line %d     : Invalid numeric constant %s\n", line, str);
                return -1;
            }

            continue;
        }
 
        printf("## ERROR at line %d     : Invalid numeric constant %s\n", line, str);  // Invalid character inside numeric constant
        return -1;
    }

    if (digit_seen)                                                         
        return 1;                                                                   //constant

    return -1;                                                                      //error
}



