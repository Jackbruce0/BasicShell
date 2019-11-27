/******************************************************************************
 NAME: Jack Bruce
 USERNAME: cssc0013
 PROJECT: CS570 Program 4
 DUE DATE: 11/29/19 
 INSTRUCTOR: Dr. John Carroll
 FILE: p2.h
 NOTES: Header file for shell code in p2.c. Includes work from p0 and p1 with
    getword.c and getword.h
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "getword.h"
#define MAXITEM 100 /* max number of words per line */
#define MAXARGS 20 /* max number of args for command */
#define HISTLEN 10 /* length of history */

typedef enum { false, true } bool;

extern bool real_pipe;

void sighandler(int signum);

void exec_fail_handler(int status, char *command);

void change_directory(char **newargv);

void setinput(void);

void setoutput(void);

/* This struct will be used for storing previous lines of input
   It basically saves the operating state of the program
   To be used with `!!` and `history` features */
typedef struct Line
{
    char newargv[MAXARGS][MAXITEM];
    int wordcount;
    char infile[MAXITEM];
    char outfile[MAXITEM];
    bool redirect_out;
    bool redirect_in;
    bool redirect_out_err;
    bool append_out;
    bool append_err;
    bool background;
    bool error;
    int newargc;
    int pipe_nx;
    char lastword[MAXITEM];
} Line;

void historyinit(Line *prev);

void storeline(Line *prev, char **newargv, int wordcount, int newargc,
    char* lastword);

void useline(Line *prev, char **newargv, int *wordcount);

int parse(char w[][STORAGE], char **newargv, Line *prev, int com_count);
/*******************************[ EOF: p2.h ]*********************************/
