/*
 * parse.c - Parse source file.
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "errormsg.h"
#include "parse.h"
#include "prabsyn.h"

extern int yyparse(void);
extern A_exp absyn_root;

/* parse source file fname; 
   return abstract syntax data structure */
A_exp parse(string fname) 
{
	EM_reset(fname);
	if (yyparse() == 0) /* parsing worked */
	return absyn_root;
	else return NULL;
}

int main(int argc, char **argv){

  int syntax_only = 1;
  
  if(argc == 3)
	syntax_only = atoi(argv[2]);

  if(argc < 2){
    fprintf(stderr,"usage: a.out filename\n");
    exit(1);
  }

  parse(argv[1]);
  if(syntax_only == 0)
	pr_exp(stderr,absyn_root,0);
  fprintf(stderr,"\n");
  return 0;
}
