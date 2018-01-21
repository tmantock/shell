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

char **parse_input(char *input) {
	int input_position = 0, token_position = 0;
	int token_size = SHELL_TOK_BUFSIZE;
	char **tokens = malloc(token_size * sizeof(char *));

	if (!tokens) {
		fprintf(stderr, "Shell Allocation Error\n");
		exit(EXIT_FAILURE);
	}

	while (input[input_position] != '\0') {
		skip_whitespace(input, &input_position);

		if (is_letter(input[input_position]) || is_number(input[input_position])) {
			int forward_position = input_position;
			skip_characters(input, &forward_position);
			char *substr = copy_substring(input, input_position, forward_position);
			tokens[token_position] = substr;

			input_position = forward_position;
		} else if (input[input_position] == '\'' || input[input_position] == '"') {
			tokens[token_position] = parse_quoted_string(input, &input_position);
		} else if (input[input_position] == '-') {
			// Build structs for tokens or keep it simple?
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

	return tokens;
}
