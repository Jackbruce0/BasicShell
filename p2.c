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

bool redirect_out, redirect_in, redirect_out_err, background, error,
prematureEOF;
int outfile_fd, infile_fd;
char outfile[MAXITEM], infile[MAXITEM];

//don't really know what to do with this, but it catches signals...
void sighandler(int signum)
{
    return;
}

/******************************************************************************
 FUNCTION: use line 
 NOTES: Populates current control values with the previous values stored in 
    Line struct 
 I/O: input parameters: Line *prev for populating values, and necessary values
                        to populate
      output: void 
 *****************************************************************************/
void useline(Line *prev, char **newargv, int *wordcount)
{
    *newargv = *prev->newargv;
    *wordcount = prev->wordcount;
    *infile = *prev->infile;
    *outfile = *prev->outfile;
    redirect_out = prev->redirect_out;
    redirect_in = prev->redirect_in;
    redirect_out_err = prev->redirect_out_err;
    background = prev->background;
    //add new error variable
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
    *prev->infile = *infile;
    *prev->outfile = *outfile;
    prev->redirect_out = redirect_out;
    prev->redirect_in = redirect_in; 
    prev->redirect_out_err = redirect_out_err;
    prev->background = background;
    //add new error variable
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
    prev->newargv[0] = NULL;
    prev->wordcount = 0;
    strcpy(prev->infile, " ");
    strcpy(prev->outfile, " ");
    prev->redirect_out = false;
    prev->redirect_in = false;
    prev->redirect_out_err = false;
    prev->background = false;
    //add new error variable
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
        int inflags = O_RDONLY;
        if ((infile_fd = open(infile, inflags)) < 0)
        {
            fprintf(stderr, "%s: No such file or directory.\n", infile);
            dup2(open("/dev/null", O_RDONLY), STDIN_FILENO);
        }
    }
    if (redirect_in && infile_fd > 0)
    {
        dup2(infile_fd, STDIN_FILENO); /* you should check the 
                                               return status */
     } else if (background)
        dup2(open("/dev/null", O_RDONLY), STDIN_FILENO);
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
        int outflags = O_WRONLY | O_CREAT | O_EXCL;
        if ((outfile_fd = open(outfile, outflags, S_IRUSR | S_IWUSR)) < 0)
        {
            //perror("open failed");
            fprintf(stderr, "%s: File exists.\n", outfile);
            dup2(open("/dev/null", O_WRONLY), STDOUT_FILENO);
        }
        else
            dup2(outfile_fd, STDOUT_FILENO); /* you should check the 
                                                 return status */
        if (redirect_out_err) 
            dup2(outfile_fd, STDERR_FILENO);
    } 
} /* End function set output */

/******************************************************************************
 FUNCTION: parse
 NOTES: Syntactic analyzer for shell. Uses getword to collect input. Words
    collected are left in the buffer.
    return value codex:
        -3 -> error
        -2 -> !! (repeat last command)
        -1 -> done (exit shell)
 I/O: input parameters: 2d char array for storage of the collected words
      output: # of words read.
 *****************************************************************************/
int parse(char words[][STORAGE], char **newargv, Line *prev)
{
    char s[STORAGE]; /* buffer for individual word */
    int c = 0;
    int wordcount = 0, newargc = 0;
    bool prevline = false;
    for(;;)
    {
        c = getword(s);
        if (!strcmp(s, "!!") && wordcount == 0) 
            prevline = true; /* when `!!` is the first word, we will use 
                                contents of last parse */
        if (prevline && c > 0) continue; /* do not collect trailing words when 
                                            `!!' was encountered */
        if (!prevline) strcpy(words[wordcount], s);
        if (c == 0) //\n collected
        { 
            /* check for last word collected being '&' */
            if (newargc > 0 && !strcmp(newargv[newargc - 1], "&"))
            {
                background = true;
                newargc--; /* decrementing counter will result in '&' not
                              being stored and passed to child */
            }
            break; /* end parse when \n is read */
        }
        if (wordcount == 0 && c == -1) return -1;
        if (wordcount != 0 && c == -1 && strcmp(s, "done"))
        {
            prematureEOF = true;
            break;
        }
        
        /* output redirect preparation */
        if(!strcmp(s, ">") || !strcmp(s, ">&"))
        {
            if (redirect_out || redirect_out_err) /* cannot have multiple '>' 
                                                     or '>&' in one statement */
            {
                fprintf(stderr, "Syntax error: Cannot redirect output to \
multiple files.\n");
                error = true;
            }
            else if (!strcmp(s, ">")) redirect_out = true;
            else redirect_out_err = true;
            wordcount++;
            continue;
        }
        /* any word following '>' or '>&' will be accepted as a file name */
        if (redirect_out && !strcmp(words[wordcount-1], ">")
            || redirect_out_err && !strcmp(words[wordcount-1], ">&"))
        {
            strcpy(outfile, words[wordcount++]);
            continue;
        }
        /*******************************/ 
        /* input redirect preparation */
        if (!strcmp(s, "<"))
        {
            if (redirect_in) /* cannot have multiple '<' in one statement */
            {
                fprintf(stderr, "Syntax error: Cannot accept input from \
multiple files.\n");
                error = true;
            } else redirect_in = true;
            wordcount++;
            continue;
        }
        /* any word following '<' will be accepted as a file name */
        if (redirect_in && !strcmp(words[wordcount-1], "<"))
        {
            strcpy(infile, words[wordcount++]);
            continue;
        }
        /******************************/ 
        newargv[newargc] = words[wordcount]; /* populate new argv with
                                                command and arguments */
        wordcount++;
        newargc++;
    }

    if (prevline) return -2; /* return status for `!!` */

    newargv[newargc] = NULL;
    storeline(prev, words, newargv, wordcount);
    if (newargv[0] == NULL && wordcount > 0) /* If there is no command */
    {
        fprintf(stderr, "Syntax error: No command given.\n");
        error = true;
    }
    return wordcount;
} /* End function parse */

int main(int argc, char **argv)
{
    char words[MAXITEM][STORAGE]; /* words collected from stdin */
    char *newargv[MAXARGS]; 
    char prompt[5] = "%1% ";
    int wordcount = 0, execstatus = 0, kidpid;
    Line *prev = malloc(sizeof(Line));
    prematureEOF = false;
    historyinit(prev);
    //any necessary set-up, including signal catcher and setpgid();
    /* SIGNAL HANDLING */
    setpgid(0,0);
    (void) signal(SIGTERM, sighandler);
    /******************/
    if (argv[1] != NULL) /* if valid file is present as first arg. Use that
                            as input */
    {
        int inflags = O_RDONLY;
        int commands_fd = -1;
        if ((commands_fd = open(argv[1], inflags)) < 0)
        {
            perror("open failed");
            exit(9);
        }
        else dup2(commands_fd, STDIN_FILENO);
    }
    for(;;)
    {
        redirect_out = false;
        redirect_in = false;
        redirect_out_err = false;
        background = false;
        error = false;

        printf("%s", prompt);
        //call parse function, setting [global] flags as needed;
        wordcount = parse(words, newargv, prev);
        //if (getword() returned -1 and line is empty) break
        if (wordcount == -2) //!! handling
        {
            useline(prev, newargv, &wordcount);
        }
        if (wordcount == -1) break;
        if (wordcount == 0) continue; // reissue prompt if line is empty        
        //handle builtins (done, cd, !!) and continue, or:

        if (error) continue;

        if (!strcmp("cd", newargv[0]))
        {
            change_directory(newargv);         
            continue;
        }
        //set up for redirection
        if ((kidpid = fork()) == 0) {
            // redirect i?o as requested (background children sometimes need
            setoutput();
            setinput();
            // use execvp() to start requested process;
            execstatus = execvp(newargv[0] , newargv);
            if (execstatus == -1) //if exec failed
            {
                perror("exec failed");
                exit(9); //use different exit codes for different errors
            }
            if (outfile_fd > 0)
                close(outfile_fd);
            if (infile_fd > 0)
                close(infile_fd);
        }
        //if appropriate, wait for child to complete;
        if (background) printf("%s [%d]\n", newargv[0], kidpid);
        //else print the child's pid (and in this casek the child should
        else
        {
            for (;;)
            {
                pid_t pid;
                pid = wait(NULL);
                if (pid == kidpid) break;
            }
        }

        if (prematureEOF)
        {
            printf("%s", prompt); //This is sloppy, but it makes output match
            break;
        }
    }
    /* Required */
    killpg(getpgrp(), SIGTERM);
    printf("p2 terminated.\n");
    exit(0);
    /*************/
} /* End function main */

/*******************************[ EOF: p2.c ]*********************************/
