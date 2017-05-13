#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "LineParser.h"
#include <linux/limits.h>
#include <unistd.h>

#ifndef NULL
    #define NULL 0
#endif

#define FREE(X) if(X) free((void*)X)


int printCommandLine(cmdLine* cmdLine){

  int i;
  
  printf("arguments: ");
  for(i = 0; i < cmdLine->argCount; i++){

    printf("%s ", cmdLine->arguments[i]);
  }
  puts("");

  printf("argCount: %d\n", cmdLine->argCount);
  printf("inputRedirect: %s\n", cmdLine->inputRedirect);
  printf("outputRedirect: %s\n", cmdLine->outputRedirect);
  printf("blocking: %c\n", cmdLine->blocking);
  printf("idx: %d\n", cmdLine->idx);

  return 0;
}

int printCurrentPath(){
  
  char buf[PATH_MAX];

  getcwd(buf, PATH_MAX);

  printf("current path: %s\n", buf);  
  
  return 0;  
}



int execute(cmdLine *pCmdLine){

  if(execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1){
    
    perror("execv");
    freeCmdLines(pCmdLine);
    exit(EXIT_FAILURE);
  }
  printf("blabla\n");

  return 0;

}


int main(){


  while(1){
    
    printCurrentPath();

    char userInput[2048];

    printf("Please enter your input: \n");
    fgets(userInput, 2048, stdin);

    cmdLine* cmdLine = parseCmdLines(userInput);

    // printCommandLine(cmdLine);

    execute(cmdLine);
  }



  return 0;
}





static char *cloneFirstWord(char *str)
{
    char *start = NULL;
    char *end = NULL;
    char *word;

    while (!end) {
        switch (*str) {
            case '>':
            case '<':
            case 0:
                end = str - 1;
                break;
            case ' ':
                if (start)
                    end = str - 1;
                break;
            default:
                if (!start)
                    start = str;
                break;
        }
        str++;
    }

    if (start == NULL)
        return NULL;

    word = (char*) malloc(end-start+2);
    strncpy(word, start, ((int)(end-start)+1)) ;
    word[ (int)((end-start)+1)] = 0;

    return word;
}

static void extractRedirections(char *strLine, cmdLine *pCmdLine)
{
    char *s = strLine;

    while ( (s = strpbrk(s,"<>")) ) {
        if (*s == '<') {
            FREE(pCmdLine->inputRedirect);
            pCmdLine->inputRedirect = cloneFirstWord(s+1);
        }
        else {
            FREE(pCmdLine->outputRedirect);
            pCmdLine->outputRedirect = cloneFirstWord(s+1);
        }

        *s++ = 0;
    }
}

static char *strClone(const char *source)
{
    char* clone = (char*)malloc(strlen(source) + 1);
    strcpy(clone, source);
    return clone;
}

static int isEmpty(const char *str)
{
  if (!str)
    return 1;
  
  while (*str)
    if (!isspace(*(str++)))
      return 0;
    
  return 1;
}

static cmdLine *parseSingleCmdLine(const char *strLine)
{
    char *delimiter = " ";
    char *line, *result;
    
    if (isEmpty(strLine))
      return NULL;
    
    cmdLine* pCmdLine = (cmdLine*)malloc( sizeof(cmdLine) ) ;
    memset(pCmdLine, 0, sizeof(cmdLine));
    
    line = strClone(strLine);
         
    extractRedirections(line, pCmdLine);
    
    result = strtok( line, delimiter);    
    while( result && pCmdLine->argCount < MAX_ARGUMENTS-1) {
        ((char**)pCmdLine->arguments)[pCmdLine->argCount++] = strClone(result);
        result = strtok ( NULL, delimiter);
    }

    FREE(line);
    return pCmdLine;
}

static cmdLine* _parseCmdLines(char *line)
{
	char *nextStrCmd;
	cmdLine *pCmdLine;
	char pipeDelimiter = '|';
	
	if (isEmpty(line))
	  return NULL;
	
	nextStrCmd = strchr(line , pipeDelimiter);
	if (nextStrCmd)
	  *nextStrCmd = 0;
	
	pCmdLine = parseSingleCmdLine(line);
	if (!pCmdLine)
	  return NULL;
	
	if (nextStrCmd)
	  pCmdLine->next = _parseCmdLines(nextStrCmd+1);

	return pCmdLine;
}

cmdLine *parseCmdLines(const char *strLine)
{
	char* line, *ampersand;
	cmdLine *head, *last;
	int idx = 0;
	
	if (isEmpty(strLine))
	  return NULL;
	
	line = strClone(strLine);
	if (line[strlen(line)-1] == '\n')
	  line[strlen(line)-1] = 0;
	
	ampersand = strchr( line,  '&');
	if (ampersand)
	  *(ampersand) = 0;
		
	if ( (last = head = _parseCmdLines(line)) )
	{	
	  while (last->next)
	    last = last->next;
	  last->blocking = ampersand? 0:1;
	}
	
	for (last = head; last; last = last->next)
		last->idx = idx++;
			
	FREE(line);
	return head;
}


void freeCmdLines(cmdLine *pCmdLine)
{
  int i;
  if (!pCmdLine)
    return;

  FREE(pCmdLine->inputRedirect);
  FREE(pCmdLine->outputRedirect);
  for (i=0; i<pCmdLine->argCount; ++i)
      FREE(pCmdLine->arguments[i]);

  if (pCmdLine->next)
	  freeCmdLines(pCmdLine->next);

  FREE(pCmdLine);
}

int replaceCmdArg(cmdLine *pCmdLine, int num, const char *newString)
{
  if (num >= pCmdLine->argCount)
    return 0;
  
  FREE(pCmdLine->arguments[num]);
  ((char**)pCmdLine->arguments)[num] = strClone(newString);
  return 1;
}
