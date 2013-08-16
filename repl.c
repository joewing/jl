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
   for(; vp; vp = vp->next) {
      result = JLEvaluate(context, vp);
      JLPrint(context, result);
      printf("\n");
   }
   return result;
}

int main(int argc, char *argv[])
{
   char *line = NULL;
   const char *to_parse;
   struct JLContext *context;
   JLValue *value;
   JLValue *result;
   size_t cap = 0;
   char *filename = NULL;
   FILE *fd;

   if(argc == 2) {
      filename = argv[1];
   } else if(argc != 1) {
      printf("usage: %s <file>\n", argv[0]);
      return -1;
   }

   context = JLCreateContext();
   JLDefineSpecial(context, "print", PrintFunc);

   if(filename) {
      fd = fopen(filename, "r");
      if(!fd) {
         printf("ERROR: file \"%s\" not found\n", filename);
         return -1;
      }
   } else {
      fd = stdin;
   }

   while(!feof(fd)) {
      if(!filename) {
         printf("> "); fflush(stdout);
      }
      ssize_t len = getline(&line, &cap, fd);
      if(len <= 0) {
         break;
      }
      to_parse = line;
      while(*to_parse) {
         value = JLParse(context, &to_parse);
         if(value == NULL) {
            break;
         }
         result = JLEvaluate(context, value);
         if(!filename) {
            printf("=> ");
            JLPrint(context, result);
            printf("\n");
         }
      }
   }
   if(line) {
      free(line);
   }
   if(filename) {
      fclose(fd);
   }
   JLDestroyContext(context);

   return 0;

}

