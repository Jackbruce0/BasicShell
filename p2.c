/******************************************************************************
 NAME: Jack Bruce
 USERNAME: cssc0013
 PROJECT: CS570 Program 2
 DUE DATE: 10/04/19 
 INSTRUCTOR: Dr. John Carroll
 FILE: p2.c 
 NOTES: 
 SOURCES:
 *****************************************************************************/

#include "p2.h"

int c;
char words[MAXITEM][STORAGE]; /* words collected from stdin */
char *newargv[20];

int main()
{
    char prompt[5] = "%1% ";
    int wordcount = 0, execstatus = 0, kidpid;

    //any necessary set-up, including signal catcher and setpgid();
    //check for the presense of argv[1]; redirect p2's input if appropriate

    for(;;)
    {
        printf("%s", prompt);
        //call parse function, setting [global] flags as needed;
        wordcount = parse(words, newargv);
        //if (getword() returned -1 and line is empty) break
        if (wordcount == -1) break;
        if (wordcount == 0) continue; // reissue prompt if line is empty        
        //handle builtins (done, cd, !!) and continue, or:
        //set up for redirection
        if ((kidpid = fork()) == 0) {
        //  redirect I?O as requested (background children sometimes need
        //  their stdin redirected to /dev/null);
        //  use execvp() to start requested process;
            execstatus = execvp(newargv[0] , newargv);
            if (execstatus == -1) //if exec failed
            {
                perror("exec failed");
                exit(9); //use different exit codes for different errors
            }
        }
        //if appropriate, wait for child to complete;
        wait(NULL);
        //else print the child's pid (and in this casek the child should
        //redirect its stdin to /dev/null [unless '<' specifies a better target]
    }
    /* Required */
    //killpg(getpgrp(), SIGTERM);
    //printf("p2 terminated.\n");
    //exit(0);
    /*************/
    return 0;
}

/******************************************************************************
 FUNCTION: parse
 NOTES: Syntactic analyzer for shell. Uses getword to collect input. Words
    collected are left in the buffer.
 I/O: input parameters: 2d char array for storage of the collected words
      output: # of words read.
 *****************************************************************************/
int parse(char words[][STORAGE], char **newargv)
{
    char s[STORAGE]; //buffer for individual word
    int c = 0;
    int wordcount = 0; // total number of words read
    for(;;)
    {
        c = getword(s);
        strcpy(words[wordcount], s);
        newargv[wordcount] = words[wordcount]; /* populate new argv with
                                                  command and arguments */
        if (c == 0) break;
        if (wordcount == 0 && c == -1) return -1;
        wordcount++;
    }
    newargv[wordcount] = NULL;
    return wordcount;
}
/*******************************[ EOF: p2.c ]*********************************/
