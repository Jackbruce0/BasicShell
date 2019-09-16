/******************************************************************************
 NAME: Jack Bruce
 USERNAME: cssc0013
 PROJECT: CS570 Program 2
 DUE DATE: 
 INSTRUCTOR: Dr. John Carroll
 FILE: p2.c 
 NOTES: 
 SOURCES:
 *****************************************************************************/

#include "p2.h"

int c;
char words[MAXITEM][STORAGE]; /* words collected from stdin */

int main()
{

    char prompt[5] = "%1% ";
    int wordcount = 0;
    char *newargv[20];

    //any necessary set-up, including signal catcher and setpgid();
    //check for the presense of argv[1]; redirect p2's input if appropriate

    for(;;)
    {
        printf("%s", prompt);
        //call parse function, setting [global] flags as needed;
        wordcount = parse(words);
        //if (getword() returned -1 and line is empty) break
        
        // parse check
        int i;
        for(i = 0; i < wordcount; i++)
        {
            printf("word %d: [%s]\n", i ,words[i]);
            newargv[i] = words[i];
        }
        if (wordcount == -1) break;

        execvp(newargv[0] , newargv);
        //if (line is empty) continue;
        //handle builtins (done, cd, !!) and continue, or:
        //set up for redirection
//        if (fork() == 0) {
        //  redirect I?O as requested (background children sometimes need
        //  their stdin redirected to /dev/null);
        //  use execvp() to start requested process;
        //    execvp(newargv[0] , newargv);
        //    if (execvp("/bin/cat" , "./p2.c") == -1)
        //    {
        //        perror("exec failed");
        //        exit(9);
        //    }
        //  if the execvp() failed {
        //      print an error message;
        //      exit(9); [choose different exit values for different errors]
        //  }
//        }
        //if appropriate, wait for child to complete;
        //else print the chidl's pid (and in this casek the child should
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
int parse(char words[][STORAGE])
{
    char s[STORAGE]; //buffer for individual word
    int c = 0;
    int wordcount = 0; // total number of words read
    for(;;)
    {
            //(void) printf("n=%d, s=[%s]\n", c = getword(s), s);
            c = getword(s);
            strcpy(words[wordcount], s);
            
            if (wordcount != 0 && c <= 0) break;
            if (wordcount == 0 && c == -1) return -1;
            wordcount++;
    }
    return wordcount;
}
/*******************************[ EOF: p2.c ]*********************************/
