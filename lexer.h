//
//  lexer.h
//  Shell
//
//  Created by Tevin Mantock on 5/27/18.
//  Copyright © 2018 Tevin Mantock. All rights reserved.
//

#ifndef lexer_h
#define lexer_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

#define SHELL_TOK_BUFSIZE 64
#define SHELL_FLAG_CHARACTER "-"

typedef struct InputNode
{
    char **line;
    int size;
    char ops;
    struct InputNode *next;
} InputNode;

typedef struct
{
    InputNode **container;
    int iterator;
    int size;
    int count;
} List;

void skip_whitespace(char *, int *);
void skip_characters(char *, int *);
void skip_comments(char *, int *);
int is_letter(char);
int is_number(char);
int is_word(char, char);
int is_operator_character(char, char);
char *copy_substring(char *, int, int);
List *parse_input(char *);

List *createList();
void listAppend(List *, char **, char, int);
InputNode *listNextNode(List *);
int listHasNext(List *);
void listPop(List *);
int listIsEmpty(List *);
void freeInputNode(InputNode *);
void destroyList(List *);

#endif /* lexer_h */
