/******************************************************************************
 NAME: Jack Bruce
 USERNAME: cssc0013
 PROJECT: CS570 Program 2
 DUE DATE: 10/04/19 
 INSTRUCTOR: Dr. John Carroll
 FILE: p2.h
 NOTES: Header file for shell code in p2.c. Includes work from p0 and p1 with
    getword.c and getword.h
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "getword.h"
#define MAXITEM 100 /* max number of words per line */
#define MAXARGS 20 /* max number of args for command */

typedef enum { false, true } bool;

void sighandler(int signum);

void change_directory(char **newargv);

void setinput(void);

void setoutput(void);

/* This struct will be used for storing previous lines of input
   It basically saves the operating state of the program
   To be used with `!!` and `history` features */
typedef struct Line
{
    char *newargv[MAXARGS];
    int wordcount;
    char infile[MAXITEM];
    char outfile[MAXITEM];
    bool redirect_out;
    bool redirect_in;
    bool redirect_out_err;
    bool background;
    bool error;
} Line;

void historyinit(Line *prev);

void storeline(Line *prev, char w[][STORAGE], char **newargv, int wordcount);

void useline(Line *prev, char **newargv, int *wordcount);

int parse(char w[][STORAGE], char **newargv, Line *prev);
/*******************************[ EOF: p2.h ]*********************************/
