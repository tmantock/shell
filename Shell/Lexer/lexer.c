//
//  lexer.c
//  Shell
//
//  Created by Tevin Mantock on 1/15/18.
//  Copyright © 2018 Tevin Mantock. All rights reserved.
//

#include "lexer.h"

void skip_whitespace(char *input, int *position) {
	while (input[*position] == ' ') {
		*position += 1;
	}
}

void skip_characters(char *input, int *position) {
	while (input[*position] != ' ' && input[*position] != '\0') {
		*position += 1;
	}
}

int is_letter(char c) {
	return ('a' <= c && 'z' >= c) || ('A' <= c && 'Z' >= c);
}

int is_number(char c) {
	return ('0' <= c && '9' >= c);
}

int is_flag_character(char c) {
	return c == '-';
}

int is_operator(char *str, int position) {
	int count = 1;
	char op = str[position++];

	if (!is_operator_character(op)) return 0;
	if (str[position++] == op) count++;

	return count == 2 && str[position] == ' ';
}

int is_operator_character(char c) {
	return c == '&' || c == '>';
}

char *copy_substring(char *input, int start, int end) {
	int i, x = 0;
	char *substr = malloc((end - start + 1) * sizeof(char));
	for (i = start; i < end; i++) {
		if (input[i] != '\\') substr[x++] = input[i];
	}

	substr[x] = '\0';

	return substr;
}

char *parse_quoted_string(char *input, int *start) {
	char *substr = NULL;
	int position = *start + 1;
	int peek = position + 1;
	int marks = 1;

	while (marks) {
		if (input[position] != '\\' && (input[peek] == '\'' || input[peek] == '"')) {
			marks--;
		}

		if (input[peek] == '\0') {
			if (input[position] == '"' || input[position] == '\'') break;

			fprintf(stderr, "Shell: parse string argument. Terminating quote is required.\n");
			return substr;
		}

		position++;
		peek++;
	}

	if (position - (*start + 1) <= 0) {
		*start = position;

		return strdup("");
	}

	substr = copy_substring(input, *start + 1, position);
	*start = position;

	return substr;
}

Queue *parse_input(char *input) {
	Queue *que = malloc(sizeof(Queue));
	int input_length = (int) strlen(input);
	int input_position = 0;

	while (input[input_position] != '\0' && input_position < input_length) {
		skip_whitespace(input, &input_position);

		int token_position = 0;
		int token_size = SHELL_TOK_BUFSIZE;
		char *operator = "";
		char **tokens = malloc(token_size * sizeof(char *));

		if (!tokens) {
			fprintf(stderr, "Shell Allocation Error\n");
			exit(EXIT_FAILURE);
		}

		while (!is_operator(input, input_position) && input[input_position] != '\0') {
			skip_whitespace(input, &input_position);
			if (input[input_position] != '"' && input[input_position] != '\'') {
				int forward_position = input_position;
				skip_characters(input, &forward_position);
				char *substr = copy_substring(input, input_position, forward_position);
				tokens[token_position] = substr;

				input_position = forward_position;
			} else if (input[input_position] == '\'' || input[input_position] == '"') {
				tokens[token_position] = parse_quoted_string(input, &input_position);
			}

			input_position++;
			token_position++;

			if (token_position >= token_size) {
				token_size += SHELL_TOK_BUFSIZE;
				tokens = realloc(tokens, token_size * sizeof(char*));
				if (!tokens) {
					fprintf(stderr, "Shell Allocation Error\n");
					exit(EXIT_FAILURE);
				}
			}
		}

		tokens[token_position] = NULL;

		if (is_operator(input, input_position)) {
			operator = copy_substring(input, input_position, input_position + 2);
			input_position += 2;
		} else {
			input_position++;
		}

		queueAppend(que, tokens, operator, token_position - 1);
	}

	return que;
}

void queueAppend(Queue *q, char **input, char * operator, int size) {
	InputNode *node = malloc(sizeof(InputNode));
	node->line = input;
	node->operator = operator;
	node->size = size;
	node->next = NULL;

	if (q->head == NULL) {
		q->head = node;

		return;
	}

	InputNode *current = q->head;

	while (current->next) current = current->next;

	current->next = node;
}

InputNode *queueTop(Queue *q) {
	assert(q->head != NULL);

	return q->head;
}

void queuePop(Queue *q) {
	assert(q->head != NULL);

	q->head = q->head->next;
}

int queueIsEmpty(Queue *q) {
	return q->head == NULL;
}

void freeInputNode(InputNode *in) {
	if (!in) return;

	if (in->line) free(in->line);
	free(in);
}
