/**
 * @file repl.c
 * @authoer Joe Wingbermuehle
 *
 * This is a REPL for interfacing with JL on the command line.
 *
 */

#include "jl.h"

#include <stdio.h>
#include <stdlib.h>

JLValue *PrintFunc(struct JLContext *context, JLValue *arglist)
{
   JLValue *vp = arglist->next;
   JLValue *result = arglist;
   printf("\n");
   for(; vp; vp = vp->next) {
      result = JLEvaluate(context, vp);
      JLPrint(context, result);
      printf("\n");
   }
   return result;
}

int main()
{
   char *line = NULL;
   struct JLContext *context;
   JLValue *value;
   size_t cap = 0;

   context = JLCreateContext();
   JLDefineSpecial(context, "print", PrintFunc);

   for(;;) {
      printf("> "); fflush(stdout);
      ssize_t len = getline(&line, &cap, stdin);
      if(len <= 0) {
         break;
      }
      printf("=> ");
      value = JLParse(context, line);
      JLPrint(context, JLEvaluate(context, value));
      printf("\n");
   }
   if(line) {
      free(line);
   }
   JLDestroyContext(context);

   return 0;

}

