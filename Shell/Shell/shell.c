//
//  shell.c
//  Shell
//
//  Created by Tevin Mantock on 1/10/18.
//  Copyright © 2018 Tevin Mantock. All rights reserved.
//

#include "shell.h"

char *builtin_str[] = {
	"cd",
	"ls",
	"cat",
	"pwd",
	"touch",
	"echo",
	"help",
	"exit"
};

int (*builtin_func[]) (char **) = {
	&shell_cd,
	&shell_ls,
	&shell_cat,
	&shell_pwd,
	&shell_touch,
	&shell_echo,
	&shell_help,
	&shell_exit
};

int arglen(char **args) {
	int length = 0, i = 0;

	while(args[i] != NULL) {
		i++;
		length++;
	}

	return length;
}

int shell_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args) {
	if (args[1] == NULL) {
		fprintf(stderr, "Shell: Expected argument to \"cd\"\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("Shell");
		}
	}

	return 1;
}

int shell_ls(char **args) {
	int size = arglen(args);
	struct dirent **list;

	char *directory = ".";
	char delimeter = '\t';
	int n = 0;

	for (int i = 1; i < size; i++) {
		if (strncmp(args[i], "-l", 2) > 0 || strncmp(args[i], "-l", 2) == 0) {
			delimeter = '\n';
		}

		if (!is_flag_character(args[i][0])) {
			directory = args[i];
		}
	}

	if (args[1] == NULL)
		n = scandir(directory, &list, 0, alphasort);
	else
		n = scandir(directory, &list, 0, alphasort);

	if (n < 0) {
		perror("Shell scandir");
	} else {
		while (n--) printf("%s%c", list[n]->d_name, delimeter);
		printf("\n");
	}

	return 1;
}

int shell_cat(char **args) {
	int c;
	int bufsize = SHELL_RL_BUFSIZE, position = 0;
	char *buffer = malloc(bufsize * sizeof(char));
	FILE *f;

	if (args[1] == NULL) {
		fprintf(stderr, "Shell: cat error\n");
	} else {
		f = fopen(args[1], "r");
		if (f) {
			while ((c = getc(f)) != EOF) {
				buffer[position++] = c;

				if (position >= bufsize) {
					bufsize += SHELL_RL_BUFSIZE;
					buffer = realloc(buffer, bufsize);
				}
			}

			if (position >= bufsize) {
				bufsize += 1;
				buffer = realloc(buffer, bufsize);
			}

			buffer[position++] = '\0';
			fclose(f);
			printf("%s\n", buffer);
		}
	}

	free(buffer);
	return 1;
}

int shell_pwd(char **args) {
	int bufsize = SHELL_RL_BUFSIZE;
	char *buffer = malloc(bufsize * sizeof(char));

	if (getcwd(buffer, (size_t) bufsize) != NULL) {
		printf("%s\n", buffer);
	} else {
		perror("Shell: pwd");
	}

	free(buffer);

	return 1;
}

int shell_touch(char **args) {
	FILE *f;
	if (args[1] != NULL) {
		f = fopen(args[1], "w");

		if (f == NULL) {
			fprintf(stderr, "Shell: touch unable to create a file.\n");
			return -1;
		}

		fclose(f);
	} else {
		printf("Shell: touch command expects one argument to be the name of the file.\n");
	}

	return 1;
}

int shell_echo(char **args) {

	if (args[1] == NULL) {
		printf("Shell: echo command expects one argument to be a string.\n");
	} else {
		printf("%s\n", args[1]);
	}

	return 1;
}

int shell_help(char **args) {
	int i;

	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");

	for (i = 0; i < shell_num_builtins(); i++) {
		printf("  %s\n", builtin_str[i]);
	}

	printf("Use the man command for information on other programs.\n");
	return 1;
}

int shell_exit(char **args) {
	return 0;
}

void shell_loop(void) {
	char *line;
	char **args;
	int status;

	do {
		printf("$ ");
		line = shell_read_line();
		args = shell_split_line(line);
		status = shell_execute(args);

		free(line);
		free(args);
	} while (status);
}

char *shell_read_line(void) {
	int bufsize = SHELL_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer) {
		fprintf(stderr, "Shell Allocation Error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		c = getchar();

		if (c == EOF || c == '\n') {
			buffer[position] = '\0';

			return buffer;
		} else {
			buffer[position] = c;
		}

		position++;

		if (position >= bufsize) {
			bufsize += SHELL_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);

			if (!buffer) {
				fprintf(stderr, "Shell Allocation Error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

char **shell_split_line(char *line) {
	char **tokens = parse_input(line);

	return tokens;
}

int shell_launch(char **args) {
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			perror("Shell");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		perror("Shell");
	} else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

int shell_execute(char **args) {
	int i;

	if (args[0] == NULL) {
		return 1; // Empty command
	}

	for (i = 0; i < shell_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}

	return shell_launch(args);
}

