/******************************************************************************
 NAME: Jack Bruce
 USERNAME: cssc0013
 PROJECT: CS570 Program 2
 DUE DATE: 
 INSTRUCTOR: Dr. John Carroll
 FILE: getword.c 
 NOTES: Contains getword method which is a lexical analyzer used to gather one
    word of input from stdin. Specific criteria for a valid word is defined
    below in the function notes. This will be used as our collector for user
    input for the shell we devlop in further projects.
 SOURCES: getword method was based upon Foster's getchar example in inout2.c
    found in ~masc0000/CbyDiscovery/ch2/inout2.c
 *****************************************************************************/

#include "getword.h"
#include "p2.h"

bool real_pipe;

/******************************************************************************
 FUNCTION: getword
 NOTES: gets one word from input stream.
    - word = maximal string consisting of anything except
        space, newline, or EOF
    - let "->" represent the word "returns"
    - EOF when no word collencted -> -1 (empty buffer)
    - '\n' when no word collected -> 0 (empty buffer)
        - ungetc is used when '\n' is encountered at the end of a previous
            word. We do this to isolate the instance of '\n' and let a return
            value of 0 denote the parsing of a full line of input.
    - word "done" collected -> -1 ("done" in buffer)
    - metacharacters end word and are also words in themselves
        - "<", ">", ">&", ">>", ">>&", "|", and "&"
    - '\' is a charachter escape. If '\' precedes a space or metacharacter the
      word is not ended and '\' is not stored in buffer
    - all other words -> # of chars (word collected in buffer)
 I/O: input parameters: a pointer to start of char array. This is our buffer to
                        store collected word. Empty string is stored when no 
                        word is collected.
      output: -1 or # of chars in word
 *****************************************************************************/
int getword(char *w) 
{
    /* taking meta characters out breaks program (hence the duplicate '&')
       I HAVE NO IDEA WHAT THE PROBLEM IS. meta MUST have 5 chars in it*/
    char meta[] = {'<', '>', '|', '&', '&'}; //all single char meta characters
    int size = 0;
    int c; /* storage for each char from stdin. type int b/c getchar() returns
              type int */ 
    int escape = 0; // boolean control variable for character escape
    real_pipe = true;
    for(;;) //loop through stdin (char wise) and store each valid value in w
    {
        c = getchar();
        w[size] = c;
       
        /* Special case for distinguishing '|' from '\|' in p2.c */
        if (c == '|' && escape)
        {
            real_pipe = false;
        }

        if (strchr(meta, c) != NULL && !escape) /* meta character ends word 
                                                   collection (unless preceded
                                                   by '\') */
        {
            if(size == 0) //meta char by itself is a word
            {
                size++;
                if(c == '>')
                {
                    c = getchar();
                    if(c == '&' || c == '>') //">&" or ">>"
                    {
                        w[size++] = c;
                        if((c = getchar()) == '&' && w[size-1] == '>' ) //">>&" 
                            w[size++] = c;
                        else ungetc(c, stdin); /* '>>&' not found. 
                                                  undo destruction of check */
                    }
                    else ungetc(c, stdin); /* compound meta char not found. 
                                              undo destruction of check */
                }
            }
            else
                ungetc(c, stdin); /* return meta char to input stream to be 
                                     counted for next word */
            break;
        }

        if (c == ' ' && !escape) /* space at the end of a word ends collection 
                                    (unless preceded by '\') */
        {
            if (size == 0) continue; //skip preceding spaces
            else break;
        }
        else if (c == '\n') //newline ends word collection
        {
            if (size != 0) ungetc(c, stdin); /* put '\n' back in stream when
                                                encountered at the end of a 
                                                word. We want it isolated for
                                                the next call to this method
                                                to signify we have read one
                                                line of input. */
            break; 
        }
        else if (c == EOF) //EOF ends word collection
        {
            if (size == 0) //EOF with no word collected
            {
                w[0] = '\0';
                return -1;
            }
            break;
        }

        if (c == '\\' && !escape) { /* if current char is '\' activate escape
                                       flag. (unless preceded by '\') */
            escape = 1;
            continue;
        } else escape = 0;
        
        size++;

        if (size >= STORAGE-1) break; //max word size = STORAGE - 1
    }
    w[size] = '\0';
    return strcmp("done", w) ? size : -1; /* if word == "done" return -1 
                                             return size of word otherwise */
} /* End function getword  */

/****************************[ EOF: getword.c ]*******************************/
