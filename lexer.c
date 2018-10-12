//
//  lexer.c
//  Shell
//
//  Created by Tevin Mantock on 5/27/18.
//  Copyright © 2018 Tevin Mantock. All rights reserved.
//

#include "lexer.h"

// Function skips whitespace in a string
void skip_whitespace(char *input, int *position)
{
    // Increase the position when a whitespace char is encountered
    while (isspace(input[*position]))
    {
        *position += 1;
    }
}

// Function skips character in a string
void skip_characters(char *input, int *position)
{
    // Increase the pointer if the character isn't whitespace
    while (!isspace(input[*position]) && input[*position] != '\0')
    {
        *position += 1;
    }
}

// Function skips comments in a string
void skip_comments(char *input, int *position)
{
    // Skip the entire line of the comment if there is one by incrementing the pointer
    if (input[*position] == '#')
    {
        while (input[*position] != '\n' && input[*position] != '\0' && input[*position] != EOF)
        {
            *position += 1;
        }
    }
}

// Function checks if character is a letter
int is_letter(char c)
{
    // Compare ASCII codes
    return ('a' <= c && 'z' >= c) || ('A' <= c && 'Z' >= c);
}

// Function checks if a character is a number
int is_number(char c)
{
    // Compare ASCII codes
    return ('0' <= c && '9' >= c);
}

// Function checks if the charcater is a valid non-operator character
int is_word(char c, char nc)
{
    // Is non-op character if it isn't a space or an operator
    return !isspace(c) && !is_operator_character(c, nc) && c != '\0';
}

// Function checks if the character is a valid operator
int is_operator_character(char c, char nc)
{
    // Valid operator if the < and > are seperated by a space on both sides
    // Valid operator if the & is the last character
    return (c == '&' && (nc == '\0' || nc == '\n')) || ((c == '>' || c == '<') && (nc != '\0' && nc != '\n' && nc == ' '));
}

// Function copys a string to the heap
char *copy_substring(char *input, int start, int end)
{
    int i, x = 0;
    char *substr = malloc((end - start + 1) * sizeof(char));
    for (i = start; i < end; i++)
    {
        // expand the pid
        // Check if there are two $
        if (i + 1 < end && input[i] == '$' && input[i + 1] == '$')
        {
            char strpid[10];
            pid_t pid = getpid();
            sprintf(strpid, "%d", pid);
            // Resize the string to contain the pid
            substr = realloc(substr, (sizeof(substr) - 1 + strlen(strpid)) * sizeof(char));
            // Add in the pid char by char
            for (int y = 0; y < strlen(strpid); y++)
            {
                substr[x++] = strpid[y];
            }

            i++;
        }
        else
        {
            substr[x++] = input[i];
        }
    }
    // Add null terminator at the end
    substr[x] = '\0';

    return substr;
}

// Function parses a string for operators and expands $$ and inserts each word into a list
List *parse_input(char *input)
{
    // Create the list
    List *lst = createList();
    int input_length = (int)strlen(input);
    int input_position = 0;
    // While the end of the line hasn't been reached
    // Outer loop is for entire string
    while (input[input_position] != '\0' && input_position < input_length)
    {
        // Skip white space and comments
        skip_whitespace(input, &input_position);
        skip_comments(input, &input_position);
        // Inner loop is for each distinct commands seperated by the operators
        // Prepare the token array for strings
        int token_position = 0;
        int token_size = SHELL_TOK_BUFSIZE;
        char ops = '\0';
        char **tokens = malloc(token_size * sizeof(char *));
        // Make sure memory was allocated
        if (!tokens)
        {
            fprintf(stderr, "Shell Allocation Error\n");
            exit(EXIT_FAILURE);
        }
        // While the end of the string hasn't been reached
        while (input[input_position] != '\0')
        {
            // Skip white space and comments
            skip_whitespace(input, &input_position);
            skip_comments(input, &input_position);
            // Add words into the tokens array
            if (is_word(input[input_position], input[input_position + 1]))
            {
                int start = input_position;
                while (is_word(input[input_position], input[input_position + 1]) && input[input_position] != '\0')
                {
                    input_position++;
                }

                tokens[token_position++] = copy_substring(input, start, input_position);
            }
            // This is the end of the command if the operator has been found
            else if (is_operator_character(input[input_position], input[input_position + 1]))
            {
                ops = input[input_position++];
                break;
            }
            // Skip trailing whitespace
            skip_whitespace(input, &input_position);
            // Resize the tokens array if necessary
            if (token_position >= token_size)
            {
                token_size += SHELL_TOK_BUFSIZE;
                tokens = realloc(tokens, token_size * sizeof(char *));
                if (!tokens)
                {
                    fprintf(stderr, "Shell Allocation Error\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        // Terminate the tokens array
        tokens[token_position] = NULL;
        // Add the args and operator to the list
        listAppend(lst, tokens, ops, token_position);
        // Reset the ops char
        ops = '\0';
    }

    return lst;
}

// Function creates and initializes a list data structure
List *createList()
{
    List *lst = (List *)malloc(sizeof(List));
    lst->size = 10;
    lst->count = 0;
    lst->iterator = 0;
    lst->container = (InputNode **)malloc(sizeof(InputNode) * lst->size);

    return lst;
}

// Function appends a node tot the list
void listAppend(List *l, char **input, char operator, int size)
{
    if (size == 0)
    {
        return;
    }

    // Setup the node
    InputNode *node = malloc(sizeof(InputNode));
    node->line = input;
    node->ops = operator;
    node->size = size;
    // Add the node and increment the count
    l->container[l->count++] = node;
    // Resize if necessary
    if (l->size == l->count)
    {
        l->size *= 2;
        l->container = realloc(l->container, sizeof(InputNode) * l->size);
    }
}

// Function iterates over the list
InputNode *listNextNode(List *l)
{
    if (!l || l->count == 0 || l->iterator >= l->count)
        return NULL;

    return l->container[l->iterator++];
}

// Function checks if there's a next node from the current iterator position
int listHasNext(List *l)
{
    if (!l || l->count == 0)
        return 0;

    return l->iterator < l->count;
}

// Function checks if the list is empty
int listIsEmpty(List *l)
{
    return l->count == 0;
}

// Function frees an input node and the list of args it holds
void freeInputNode(InputNode *in)
{
    if (!in)
        return;

    if (in->line)
        free(in->line);
    free(in);
}

// Fucntion destroys the nodes in a list and the list itself
void destroyList(List *l)
{
    for (int i = 0; i < l->count; i++)
    {
        freeInputNode(l->container[i]);
    }

    free(l);
}
