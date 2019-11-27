/******************************************************************************
 NAME: Jack Bruce
 USERNAME: cssc0013
 PROJECT: CS570 Program 4
 DUE DATE: 11/29/19 
 INSTRUCTOR: Dr. John Carroll
 FILE: p2.c 
 NOTES: Basic shell that handles commmand execution with input/output
    redirection, cd (change directory), and running processes in the
    background. Uses getword function from getword.c for parsing.
 SOURCES: See ~/Two/gradernotes
 *****************************************************************************/

#include "p2.h"

bool redirect_out, redirect_in, redirect_out_err, append_out, append_err,
    background, error;
int outfile_fd, infile_fd;
char outfile[MAXITEM], infile[MAXITEM];
Line *history[HISTLEN + 1] = { 0 }; /* xtra slot will be used for !! when
                                       # of commands exceeds 10 
                                       THIS DOES NOT NEED TO BE GLOBAL...*/
int pipe_nx; /* index of newargv where pipe args are stored */
bool script; /* flag for using a a file as shel input */

/******************************************************************************
 FUNCTION: signal handler 
 NOTES: Function required for use with signal command. Empty b/c we just want
    to avoid default behavior for SIGTERM signal
 I/O: input parameters: value for SIGTERM
      output: void 
 *****************************************************************************/
void sighandler(int signum)
{
    return;
}

/******************************************************************************
 FUNCTION: exec failure handler
 NOTES: Called after exec to check status and perform necessary cleanup upon
    failure.
 I/O: input parameters: status of exec call, name of command
      output: void
 *****************************************************************************/
void exec_fail_handler(int status, char *command)
{
    if (status == -1) /* exec failure case */
    {
        fprintf(stderr, "%s: Command not found or failed to execute.\n", 
            command);
        /* close all open files */
        if (outfile_fd > 0)
            close(outfile_fd);
        if (infile_fd > 0)
            close(infile_fd);
        exit(errno); /* errno was set in last call to execvp */
    }
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
    int i;
    for (i = 0; i < prev->newargc; i++)
    {
        /* special case to account for char[][] vs **char */
        if (!strcmp(prev->newargv[i], "\0")) newargv[i] = NULL; 
        else 
        {
            if (newargv[i] == NULL) newargv[i] = strdup(prev->newargv[i]);
            else strcpy(newargv[i], prev->newargv[i]); //segfault occurs wehn newargv = null
        }
    }       
    newargv[prev->newargc] = NULL; 
    *wordcount = prev->wordcount;
    *infile = *prev->infile;
    *outfile = *prev->outfile;
    redirect_out = prev->redirect_out;
    redirect_in = prev->redirect_in;
    redirect_out_err = prev->redirect_out_err;
    append_out = prev->append_out;
    append_err = prev->append_err;
    background = prev->background;
    error = prev->error;
    pipe_nx = prev->pipe_nx;
}

/******************************************************************************
 FUNCTION: store line 
 NOTES: Populates Line struct with current line's values
 I/O: input parameters: Line *prev for storage, and necessary values gathered
                        from parse  
      output: void  
 *****************************************************************************/
void storeline(Line *prev, char **newargv, int wordcount, int newargc)
{
    int i;
    for (i = 0; i < newargc; i++)
    {
        /* special case to account for char[][] vs **char */
        if (newargv[i] == NULL) strcpy(prev->newargv[i], "\0");
        else strcpy(prev->newargv[i], newargv[i]);
    }
    prev->wordcount = wordcount;
    *prev->infile = *infile;
    *prev->outfile = *outfile;
    prev->redirect_out = redirect_out;
    prev->redirect_in = redirect_in; 
    prev->redirect_out_err = redirect_out_err;
    prev->append_out = append_out;
    prev->append_err = append_err;
    prev->background = background;
    prev->error = error;
    prev->newargc = newargc;
    prev->pipe_nx = pipe_nx; 
    strcpy(prev->lastword, newargv[newargc - 1]);
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
    strcpy(prev->newargv[0], "\0");
    prev->wordcount = 0;
    strcpy(prev->infile, " ");
    strcpy(prev->outfile, " ");
    prev->redirect_out = false;
    prev->redirect_in = false;
    prev->redirect_out_err = false;
    prev->append_out = false;
    prev->append_err = false;
    prev->background = false;
    prev->error = false;
    prev->newargc = 0;
    prev->pipe_nx = -1; /* no pipe by default */
    strcpy(prev->lastword, "\0");
}

/******************************************************************************
 FUNCTION: change directory 
 NOTES: Uses chdir sys call with input gathered from parse(). Checks for
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
    else { 
        if (chdir(newargv[1]) != 0) {
            fprintf(stderr, "%s: %s\n", newargv[1], strerror(errno));
        }
    }
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
            /* File must exist in order to be read as input */
            if (infile[0] == '\0') fprintf(stderr, "Missing name for redirect.\n");
            else fprintf(stderr, "%s: No such file or directory.\n", infile);
            dup2(open("/dev/null", O_RDONLY), STDIN_FILENO);
            error = true;
            exit(-1);
        }
    }
    if (redirect_in && infile_fd > 0)
    {
        dup2(infile_fd, STDIN_FILENO);
    } else if (background)
        dup2(open("/dev/null", O_RDONLY), STDIN_FILENO); /* bg processes can not
                                                            compete for input */
} /* End function set input */

/******************************************************************************
 FUNCTION: set output 
 NOTES: Handles redirection of output 
 I/O: input parameters: void 
      output: void 
 *****************************************************************************/
void setoutput(void)
{
    if (append_out | append_err)
    {
        int outflags = O_WRONLY | O_APPEND;
        if ((outfile_fd = open(outfile, outflags, S_IRUSR | S_IWUSR)) < 0)
        {
            /* File MUST exist previously to be written to */
            if (outfile[0] == '\0') fprintf(stderr, "Missing name for redirect.\n");
            else fprintf(stderr, "%s: file does not exist.\n", outfile);
            dup2(open("/dev/null", O_WRONLY), STDOUT_FILENO);
            error = true;
            exit(-1); /* Do not execute command if open fails */
        }
        else
            dup2(outfile_fd, STDOUT_FILENO); 
        if (append_err) 
            dup2(outfile_fd, STDERR_FILENO);
    }
    else if (redirect_out | redirect_out_err)
    {
        int outflags = O_WRONLY | O_CREAT | O_EXCL;
        if ((outfile_fd = open(outfile, outflags, S_IRUSR | S_IWUSR)) < 0)
        {
            /* File CANNOT exist previously to be written to
               (no clobber implementation) */
            if (outfile[0] == '\0') fprintf(stderr, "Missing name for redirect.\n");
            else fprintf(stderr, "%s: file exists.\n", outfile);
            dup2(open("/dev/null", O_WRONLY), STDOUT_FILENO);
            error = true;
            exit(-1); /* Do not execute command if open fails */
        }
        else
            dup2(outfile_fd, STDOUT_FILENO); 
        if (redirect_out_err) 
            dup2(outfile_fd, STDERR_FILENO);
    } 
} /* End function set output */

/******************************************************************************
 FUNCTION: parse
 NOTES: Syntactic analyzer for shell. Uses getword to collect input. Words
    collected are left in the buffer.
    return value codex:
        -11-(-19) -> !# (where # = a digit that denotes a prev command)
            + The absolute value of this number - 10 will be the index in
              our history table for the command to be executed
        -2 -> !! (repeat last command)
        -1 -> done (exit shell)
 I/O: input parameters: 2d char array for storage of the collected words
                        ptr to newargv array, ptr to struct for previous line 
      output: # of words read or exit code from above
 *****************************************************************************/
int parse(char words[][STORAGE], char **newargv, Line *prev, int com_count)
{
    char s[STORAGE]; /* buffer for individual word */
    int c = 0;
    int wordcount = 0, newargc = 0, pipe_argc = 0;
    bool bang = false; /* flag for all 2 char commands that begin w/ '!' */
    char bang_buf[3] = "\0"; /* buffer for chars considered for '!' command */
    char cmp[3]; /* string used for command comparison */
    int i;
    for(;;)
    {
        c = getword(s);
        
        /* Comment handler -> ignores all words in between '#' and '\n' */
        if (script && !strcmp(s, "#") && c == 1) {
            while ((c = getword(s)) != 0);
        }

        if (!strcmp(s,"|") && real_pipe) {  
            newargv[newargc] = NULL;
            pipe_nx = newargc + 1;
            wordcount++;
            newargc++; /*new argc isn't really newargc any more and probably has
                         the same value as wordcount as we keep incrementing it regardless
                         of a pipe or not. A cleaner solution would not do this and just
                         use wordcount as an index */
            continue;
        }
        if (!strncmp(s, "!", 1) && wordcount == 0 && strlen(s) == 2 && strncmp(s, "!$", 2)) {
            bang = true; /* when `!*` is the first word, we will use 
                                contents of last parse */
            strncpy(bang_buf, s, 3);
        }
        if (bang && c > 0) continue; /* do not collect trailing words when 
                                            `!*` was encountered */
        if (!bang) strcpy(words[wordcount], s);
        if (c == 0) /* \n collected */
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
            /* A premature EOF should still execute command that was entered
               prior */
            break;
        }
        
        /* APPENDING output redirect prepation */
        if (!strcmp(s, ">>") || !strcmp(s, ">>&"))
        {
            /* Cannont redirect ouptut to mnultiple places */
            if (redirect_out || redirect_out_err || append_out || append_err) 
            {
                fprintf(stderr, "Syntax error: Cannot redirect output to \
multiple files.\n");
                error = true;
            }
            else if (!strcmp(s, ">>")) append_out = true;
            else append_err = true;
            wordcount++;
            continue;
        }
        /* output redirect preparation */
        else if (!strcmp(s, ">") || !strcmp(s, ">&"))
        {
            /* Cannont redirect ouptut to mnultiple places */
            if (redirect_out || redirect_out_err ||append_out || append_err) 
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
            || redirect_out_err && !strcmp(words[wordcount-1], ">&")
            || append_out && !strcmp(words[wordcount-1], ">>")
            || append_err && !strcmp(words[wordcount-1], ">>&"))
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
        /* if "!$" is present, replace current word with last word
           of previous command */
        if (!strncmp(words[wordcount], "!$", 2)) 
            strcpy(newargv[newargc], history[com_count - 1]->lastword);
        else 
            newargv[newargc] = words[wordcount]; /* populate new argv with
                                                    command and arguments */
        wordcount++;
        if (pipe_nx != -1) pipe_argc++;
        newargc++;
    }
    fflush(NULL); /* Clear words that weren't parsed  */
    if (bang) /* On the outside of loop becuase `!` commands are ENTIRE lines */
    {
        /* something about !$ will be in here too i guess */
        if (!strcmp(bang_buf, "!!")) return -2; /* return status for `!*` */
        for (i = 1; i < 10; i++) 
        {
            sprintf(cmp, "!%d", i); /* cmp will = !1 - !9 */
            if (!strncmp(bang_buf, cmp, 2)) {
                /* Check history request is not out of range */
                if (i > com_count) {
                    fprintf(stderr, "%d: Event not found.\n", i);
                    error = true;
                    return wordcount;
                }
                else return -10 - i; /* return values specified above */
            }
        }
    }

    newargv[newargc] = NULL;
    storeline(prev, newargv, wordcount, newargc);
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
    char prompt[6];
    Line tmp[HISTLEN];
    int wordcount = 0, execstatus = 0, kidpid, g_kidpid, com_count = 0;
    int pipe_fd[2];
    script = false;
    pipe_nx = -1; /* value of -1 indicates no pipe */
    /* initialize next entry in history array */
    /* Will have issues in blank line case */
    history[com_count] = &tmp[com_count];
    historyinit(history[com_count]);
    /******************************************/
    /* SIGNAL HANDLING */
    setpgid(0,0);
    (void) signal(SIGTERM, sighandler);
    /*******************/
    if (argv[1] != NULL) /* if valid file is present as first arg. Use that
                            as input (extra args are ignored, no error) */
    {
        int inflags = O_RDONLY;
        int commands_fd = -1;
        script = true;
        if ((commands_fd = open(argv[1], inflags)) < 0)
        {
            fprintf(stderr, "%s: %s\n", argv[1], strerror(errno));
            exit(errno); /* errno was set last call to open */
        }
        else dup2(commands_fd, STDIN_FILENO);
    }
    for(;;)
    {
        redirect_out = false;
        redirect_in = false;
        redirect_out_err = false;
        append_out = false;
        append_err = false;
        background = false;
        error = false;
        outfile[0] = '\0';
        infile[0] = '\0';
        pipe_nx = -1;

        sprintf(prompt, "%%%d%% ",com_count + 1);
        if (!script) printf("%s", prompt); /* Do no print prompt during script
                                              execution */
        /* parse stdin, setting [global] flags as needed */

        /* IMPORTANT: if com_count > 9 we shall pass history[9]
         * and for !! we will always use history[9] after 10 commands have passed 
         * NO CHECKS FOR OVERFLOW CURRENTLY IN PLACE */

        wordcount = parse(words, newargv, history[com_count], com_count);
        if (wordcount < -10 && wordcount > -20) 
        {
            int index = abs(wordcount) - 10 - 1; /* -1 for 0-8 indexing */
            useline(history[index],newargv, &wordcount);

            /*Copy this line's pointer to next slot as well to account
             * for hole in history table */
            history[com_count] = history[index];
        }
        else if (wordcount == -2) /* handle !* built-in, use prev line */
        {
            if (com_count == 0) /* 1st command ever */
                useline(history[0], newargv, &wordcount);
            else
            {
                useline(history[com_count-1], newargv, &wordcount);
                /*Copy previous pointer to next slot as well to account
                 * for hole in history table*/
                history[com_count] = history[com_count-1];
            }
        }
        
        if (wordcount == -1)
            break;
        if (wordcount == 0)
            continue; /* reissue prompt if line is empty */
        if (error) 
            continue;
        com_count++; /* com_count is not incremented for empty lines or errors 
                        ask about `!!` on an empty line */
        if (!strcmp("cd", newargv[0])) /* handle cd built-in */
        {
            change_directory(newargv);         
            /* Initialize next entry in history */
            history[com_count] = &tmp[com_count];
            historyinit(history[com_count]);
            continue;
        }
        /* flush all open I/O streams */
        fflush(NULL);
        if ((kidpid = fork()) == 0) {
            if (pipe_nx != -1) /* Lets lay some pipe! */
            {
                pipe(pipe_fd);//check for fail
                /* flush all open I/O streams */
                fflush(NULL);
                if ((g_kidpid = fork()) == 0)
                {
                    /* g_child handles left command and writes to pipe */
                    setinput();
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    close(pipe_fd[0]);
                    close(pipe_fd[1]);
                    /* start requested process */
                    execstatus = execvp(newargv[0], newargv); 
                    exec_fail_handler(execstatus, newargv[0]);
                }

                /* child handles right command and reads from pipe */
                setoutput();
                dup2(pipe_fd[0], STDIN_FILENO);
                close(pipe_fd[0]);
                close(pipe_fd[1]);
                /* start requested process */
                execstatus = execvp(newargv[pipe_nx], newargv+pipe_nx);
                exec_fail_handler(execstatus, newargv[pipe_nx]);
            }

            /* redirect I/O as requested */
            setoutput();
            setinput();
            /* start requested process */
            execstatus = execvp(newargv[0] , newargv);
            exec_fail_handler(execstatus, newargv[0]);
        }
        /* print pid of child if bg process, otherwise wait for child process
           to complete */
        if (background) printf("%s [%d]\n", newargv[0], kidpid);
        else
        {
            for (;;)
            {
                pid_t pid;
                pid = wait(NULL);
                if (pid == kidpid) break;
            }
        }
        
        /* Initialize next entry in history */
        history[com_count] = &tmp[com_count];
        historyinit(history[com_count]);

    }
    /* Terminate any children that are still running.
       Last 3 lines are used from program2 instructions verbatim */
    killpg(getpgrp(), SIGTERM);
    if (!script) printf("p2 terminated.\n"); /* Do not print message after 
                                                script execution */
    exit(0);
    /*************/

} /* End function main */

/*******************************[ EOF: p2.c ]*********************************/
