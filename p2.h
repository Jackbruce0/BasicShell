/******************************************************************************
 NAME: Jack Bruce
 USERNAME: cssc0013
 PROJECT: CS570 Program 2
 DUE DATE: 10/04/19 
 INSTRUCTOR: Dr. John Carroll
 FILE: p2.h
 NOTES: 
 SOURCES:
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "getword.h"
#define MAXITEM 100 /* max number of words per line */

typedef enum { false, true } bool;

int parse(char w[][STORAGE], char **newargv);

/*******************************[ EOF: p2.h ]*********************************/
