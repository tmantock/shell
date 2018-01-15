//
//  shell.h
//  Shell
//
//  Created by Tevin Mantock on 1/10/18.
//  Copyright © 2018 Tevin Mantock. All rights reserved.
//

#ifndef shell_h
#define shell_h

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "lexer.h"

#define SHELL_RL_BUFSIZE 1024
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a\""

int shell_cd(char **);
int shell_ls(char **);
int shell_cat(char **);
int shell_pwd(char **);
int shell_touch(char **);
int shell_echo(char **);
int shell_help(char **);
int shell_exit(char **);

void shell_loop(void);
char *shell_read_line(void);
char **shell_split_line(char *);
int shell_launch(char **);
int shell_execute(char **);

#endif /* shell_h */

