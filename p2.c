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

bool redirect_out, redirect_in, redirect_out_err;
FILE *outfile, *infile;

void useline(Line *prev, char **newargv, int *wordcount)
{
    *newargv = *prev->newargv;
    *wordcount = prev->wordcount;
    //*outfile = *prev->outfile;
    //*infile = *prev->infile;
    redirect_out = prev->redirect_out;
    redirect_in = prev->redirect_in;
    redirect_out_err = prev->redirect_out_err;
}


/******************************************************************************
 FUNCTION: store line 
 NOTES: Populates Line struct with current line's values
 I/O: input parameters: Line *prev for storage, and necessary values gathered
                        from parse  
      output: void 
 *****************************************************************************/
void storeline(Line *prev, char w[][STORAGE], char **newargv, int wordcount)
{
    *prev->newargv = *newargv;
    prev->wordcount = wordcount;
    //*prev->outfile = *outfile;
    //*prev->infile = *infile;
    prev->redirect_out = redirect_out;
    prev->redirect_in = redirect_in; 
    prev->redirect_out_err = redirect_out_err;
}

/******************************************************************************
 FUNCTION: history init 
 NOTES: Initializes contents of Line struct to the contents of an empty line.
    Values are used to pretend that the first previous entry was empty.
 I/O: input parameters:  
      output: void 
 *****************************************************************************/
void historyinit(Line *prev)
{
    //strcpy(prev->words[0], " ");
    prev->newargv[0] = NULL;
    prev->wordcount = 0;
    prev->infile = NULL;
    prev->outfile = NULL;
    prev->redirect_out = false;
    prev->redirect_in = false;
    prev->redirect_out_err = false;
}

/******************************************************************************
 FUNCTION: change directory 
 NOTES: Uses chdir system with input gathered from parse(). Checks for
    too many arguments error. Uses $HOME as path if no arguments supplied. 
 I/O: input parameters: newargv populated by parse() 
      output: void 
 *****************************************************************************/
void change_directory(char **newargv)
{
    if (newargv[2])
    {
        //perror("Too many arguments");
        fprintf(stderr, "Too many arguments\n");
        return;
    }
    if (newargv[1] == NULL)
        chdir(getenv("HOME"));
    chdir(newargv[1]);
} /* End function change directory */

/******************************************************************************
 FUNCTION: set input 
 NOTES: Handles redirection of input 
 I/O: input parameters: void 
      output: void 
 *****************************************************************************/
void setinput(void)
{
    if (redirect_in)
    {
        if (!infile)
        {
            perror("invalid input file");
            exit(9); //put a proper exit status here
        }
        dup2(fileno(infile), STDIN_FILENO); /* you should check the 
                                               return status */
     } else 
        dup2(open("/dev/null", "r"), STDIN_FILENO);
} /* End function set input */

/******************************************************************************
 FUNCTION: set output 
 NOTES: Handles redirection of output 
 I/O: input parameters: void 
      output: void 
 *****************************************************************************/
void setoutput(void)
{
    if (redirect_out | redirect_out_err)
    {
        if (!outfile)
        {
            perror("invalid output file");
            exit(9); //put a proper exit status here
        }
        dup2(fileno(outfile), STDOUT_FILENO); /* you should check the 
                                                 return status */
        if (redirect_out_err) 
            dup2(fileno(outfile), STDERR_FILENO);
    }
} /* End function set output */

/******************************************************************************
 FUNCTION: parse
 NOTES: Syntactic analyzer for shell. Uses getword to collect input. Words
    collected are left in the buffer.
    return value codex:
        -2 -> !! (repeat last command)
        -1 -> done (exit shell)
 I/O: input parameters: 2d char array for storage of the collected words
      output: # of words read.
 *****************************************************************************/
int parse(char words[][STORAGE], char **newargv, Line *prev)
{
    char s[STORAGE] = {' '}; //buffer for individual word
    int c = 0;
    int wordcount = 0, newargc = 0;
    bool prevline = false;
    for(;;)
    {
        c = getword(s);
        if (!strcmp(s, "!!")) prevline = true; /* when `!!` is encountered
                                                  we will use contents of last
                                                  parse */
        if (prevline && c > 0) continue; // do not collect any words when `!!'
        if (!prevline) strcpy(words[wordcount], s);
        if (c == 0) break; //end parse when \n is read
        if (wordcount == 0 && c == -1) return -1;
        
        /* output redirect preparation */
        if(!strcmp(s, ">"))
        {
            redirect_out = true;
            wordcount++;
            continue;
        }
        if (!strcmp(s, ">&"))
        {
            redirect_out_err = true;
            wordcount++;
            continue;
        }
        if (redirect_out && !strcmp(words[wordcount-1], ">")
            || redirect_out_err && !strcmp(words[wordcount-1], ">&"))
        {
            outfile = fopen(words[wordcount++], "w");
            continue;
        }
        /*******************************/ 
        /* input redirect preparation */
        if (!strcmp(s, "<"))
        {
            redirect_in = true;
            wordcount++;
            continue;
        }
        if (redirect_in && !strcmp(words[wordcount-1], "<"))
        {
            infile = fopen(words[wordcount++], "r");
            continue;
        }
        /******************************/ 
        newargv[newargc] = words[wordcount]; /* populate new argv with
                                                command and arguments */
        wordcount++;
        newargc++;
    }

    if (prevline) return -2; //return status for `!!`

    newargv[newargc] = NULL;
    storeline(prev, words, newargv, wordcount);
    return wordcount;
} /* End function parse */

int main()
{
    char words[MAXITEM][STORAGE]; /* words collected from stdin */
    char *newargv[MAXARGS]; 
    char prompt[5] = "%1% ";
    int wordcount = 0, execstatus = 0, kidpid;
    Line *prev = malloc(sizeof(Line));
    historyinit(prev);
    //any necessary set-up, including signal catcher and setpgid();
    //check for the presense of argv[1]; redirect p2's input if appropriate

    for(;;)
    {
        outfile = NULL;
        infile = NULL;
        redirect_out = false;
        redirect_in = false;
        redirect_out_err = false;

        printf("%s", prompt);
        //call parse function, setting [global] flags as needed;
        wordcount = parse(words, newargv, prev);
        //if (getword() returned -1 and line is empty) break
        if (wordcount == -2)
        {
            useline(prev, newargv, &wordcount);
        }
        if (wordcount == -1) break;
        if (wordcount == 0) continue; // reissue prompt if line is empty        
        //handle builtins (done, cd, !!) and continue, or:

        if (!strcmp("cd", newargv[0]))
        {
            change_directory(newargv);         
            continue;
        }
        //set up for redirection
        if ((kidpid = fork()) == 0) {
             //  redirect i?o as requested (background children sometimes need
            setoutput();
            setinput();
            // use execvp() to start requested process;
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
} /* End function main */

/*******************************[ EOF: p2.c ]*********************************/
