/*
PROJECT NAME : LEXICAL ANALYSER
DESCRIPTION : A command-line application developed in C that performs lexical analysis (tokenization) 
              of C source files. The program scans an input .c file and extracts tokens such as keywords,
              identifiers, constants (numeric/string/char), operators, and special characters while 
              reporting lexical errors (unterminated strings/char literals, malformed numeric literals, 
              unmatched delimiters). The project demonstrates file I/O, stateful scanning, character-class 
              checks, and modular design — with separate header and source files for scanner logic, token 
              classification, and utilities — for clarity and maintainability.

*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "header.h"

int main(int argc, char *argv[])
{
    storage token;                                                          //structure variable

    if (validate_arg(argv, &token) != 1)                                    //validating the c file
    {
        printf(">> Argument validation Failed\n");
        return 0;
    }

    char ch, next;                                                          //current character and next character
    int i = 0;
    int line = 1;                                                           // line number tracking
    token.line = 1;                                                         //line number tracking


    while ((ch = fgetc(token.file_ptr)) != EOF)                             //scan till EOF
    {
        if (ch == '\n')
        {
            line++;
            token.line = line;                                              // store line number in struct
        }


        //  SPACE handling -------------------------------
        if (isspace(ch))
        {
            if (i > 0)                                                    // tokenize if space is found
            {
                token.current_token[i] = '\0';
                categorizeToken(&token);
                i = 0;
            }
            continue;
        }

        // COMMENT handling -----------------------------
        if (ch == '/')
        {
            next = fgetc(token.file_ptr);                                //get next charcter

            // SINGLE LINE COMMENT ---------------------------
            if (next == '/')
            {
                while ((ch = fgetc(token.file_ptr)) != EOF)             //do not read till new line
                {
                    if (ch == '\n')
                    {
                        line++;
                        token.line = line;
                        break;
                    }
                }
                continue;
            }

            //MULTI LINE COMMENT --------------------
            if (next == '*')
            {
                int end_found = 0;                                      //flag to find end

                while ((ch = fgetc(token.file_ptr)) != EOF)
                {
                    if (ch == '\n')
                    {
                        line++;
                        token.line = line;
                    }

                    if (ch == '*')
                    {
                        next = fgetc(token.file_ptr);                   //check next character

                        if (next == '\n')
                        {
                            line++;
                            token.line = line;
                        }

                        if (next == '/')
                        {
                            end_found = 1;
                            break;
                        }
                        else
                        {
                            ungetc(next, token.file_ptr);
                        }
                    }
                }

                if (!end_found)                                          //if comment not terminated
                {
                    printf("## ERROR at line %d      : Unterminated comment\n", token.line);
                }
                continue;
            }

            ungetc(next, token.file_ptr);
        }


        // SKIP HEADER starting with # -------------------- 
        if (ch == '#')
        {
            while ((ch = fgetc(token.file_ptr)) != EOF)                  // Skip entire line until newline
            {
                if (ch == '\n')
                {
                    line++;
                    token.line = line;
                    break;
                }
            }
            continue; 
        }

        //STRING CONSTANT handling ----------------------------------
        if (ch == '\"')
        {
            token.current_token[i++] = ch;

            while ((ch = fgetc(token.file_ptr)) != EOF)
            {
                token.current_token[i++] = ch;

                if (ch == '\"')                                     // End of string
                    break;

                if (ch == '\n')                                     //to handle unterminated string literal
                {
                    line++;
                    token.line = line;
                    printf("## ERROR at line %d      : Unterminated string literal\n", token.line);
                    break;
                }

            }

            token.current_token[i] = '\0';
            categorizeToken(&token);
            i = 0;
            continue;
        }

        //CHARACTER CONSTANT handling ---------------------------
        if (ch == '\'')
        {
            token.current_token[i++] = ch;                          

            int escaped = 0;                                        //flag

            while ((ch = fgetc(token.file_ptr)) != EOF)             //start reading
            {
                token.current_token[i++] = ch;                      //update charactr to the structure

                if (ch == '\n')                                     //handles unterminated character constant
                {
                    line++;
                    token.line = line;
                    printf("## ERROR at line %d      : Unterminated character literal\n", token.line);
                    break;
                }


                if (!escaped && ch == '\\')
                {
                    escaped = 1;                                    // next character is escaped
                    continue;
                }

                if (!escaped && ch == '\'')
                {
                    break;                                          // proper end of char literal
                }

                escaped = 0;                                        // reset escape flag
            }

            token.current_token[i] = '\0';
            categorizeToken(&token);                                //send token to categorize
            i = 0;
            continue;
        }


        //IDENTIFIERS / NUMBER -------------------------
        if (isalnum(ch) || ch == '_')                               //if number or underscore
        {
            token.current_token[i++] = ch;
            continue;
        }

        //REMOVE PREVIOUS TOKEN -----------------------             //
        if (i > 0)
        {
            token.current_token[i] = '\0';
            categorizeToken(&token);
            i = 0;
        }

        // MULTI-CHAR OPERATORS ----------------------              // ++, != etc..
        next = fgetc(token.file_ptr);

        char op2[3] = { ch, next, '\0' };

        if (isOperator(op2))
        {
            strcpy(token.current_token, op2);
            categorizeToken(&token);
            continue;                                               // both characters scanned
        }
        else
        {
            ungetc(next, token.file_ptr);                           // to place the character back
        }

        // SINGLE CHAR OPERATOR ------------------                  //+ , - etc..
        char op1[2] = { ch, '\0' };

        if (isOperator(op1))
        {
            strcpy(token.current_token, op1);
            categorizeToken(&token);
            continue;
        }

        //SPECIAL CHARACTER ---------------------                   // {, }, ;, etc...
        if (isSpecialCharacter(op1))
        {
            strcpy(token.current_token, op1);
            categorizeToken(&token);
            continue;
        }

        printf("## ERROR (line %d)      : Unrecognized token '%s'\n", token.line, token.current_token); //handle any unknown token
    }

    if (i > 0)                                                      //to send the last character 
    {
        token.current_token[i] = '\0';
        categorizeToken(&token);
    }

    return 0;
}
