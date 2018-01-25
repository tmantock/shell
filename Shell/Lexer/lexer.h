//
//  lexer.h
//  Shell
//
//  Created by Tevin Mantock on 1/15/18.
//  Copyright © 2018 Tevin Mantock. All rights reserved.
//

#ifndef lexer_h
#define lexer_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define SHELL_TOK_BUFSIZE 64
#define SHELL_FLAG_CHARACTER "-"

typedef struct InputNode {
	char **line;
	int size;
	char *operator;
	struct InputNode *next;
} InputNode;

typedef struct {
	InputNode *head;
	int size;
} Queue;

void skip_whitespace(char *, int *);
void skip_characters(char *, int *);
int is_letter(char);
int is_number(char);
int is_flag_character(char);
int is_operator(char *, int);
int is_operator_character(char);
char *copy_substring(char *, int, int);
char *parse_quoted_string(char *, int*);
Queue *parse_input(char *);

void queueAppend(Queue *, char **, char *, int);
InputNode *queueTop(Queue *);
void queuePop(Queue *);
int queueIsEmpty(Queue *);
void freeInputNode(InputNode *);

#endif /* lexer_h */
