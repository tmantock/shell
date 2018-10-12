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
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

#include "lexer.h"

#define SHELL_RL_BUFSIZE 1024
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a\""

typedef struct
{
    pid_t *process;
    int size;
    int count;
} Processes;

int shell_cd(char **, int, int);
int shell_status(char **, int, int);
int shell_exit(char **, int, int);

void shell_loop(void);
char *shell_read_line(void);
List *shell_split_line(char *);
int shell_launch(List *, Processes *);
int shell_execute(List *, int, Processes *);

void sigint_handler(int);
void sigstp_handler(int);
void check_background_process(Processes *);

Processes *create_processes();
void add_process(Processes *, pid_t);
void remove_process(Processes *, pid_t);
void destroy_procces(Processes *);

#endif /* shell_h */
