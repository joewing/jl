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

static struct JLValue *PrintFunc(struct JLContext *context,
                                 struct JLValue *args)
{
   struct JLValue *vp;
   for(vp = JLGetNext(args); vp; vp = JLGetNext(vp)) {
      struct JLValue *result = JLEvaluate(context, vp);
      if(JLIsString(result)) {
         printf("%s", JLGetString(result));
      } else {
         JLPrint(context, result);
      }
      JLRelease(context, result);
   }
   return NULL;
}

static struct JLValue *ProcessBuffer(struct JLContext *context,
                                     const char *line)
{
   struct JLValue *result = NULL;
   while(*line) {
      struct JLValue *value = JLParse(context, &line);
      if(value) {
         JLRelease(context, result);
         result = JLEvaluate(context, value);
         JLRelease(context, value);
      }
   }
   return result;
}

int main(int argc, char *argv[])
{
   struct JLContext *context;
   struct JLValue *result;
   char *line = NULL;
   size_t cap = 0;
   char *filename = NULL;

   if(argc == 2) {
      filename = argv[1];
   } else if(argc != 1) {
      printf("usage: %s <file>\n", argv[0]);
      return -1;
   }

   context = JLCreateContext();
   JLDefineSpecial(context, "print", PrintFunc);

   if(filename) {
      FILE *fd = fopen(filename, "r");
      size_t len = 0;
      size_t max_len = 16;
      if(!fd) {
         printf("ERROR: file \"%s\" not found\n", filename);
         return -1;
      }
      line = (char*)malloc(max_len);
      for(;;) {
         const int ch = fgetc(fd);
         if(ch == EOF) {
            line[len] = 0;
            break;
         }
         if(len >= max_len) {
            max_len *= 2;
            line = (char*)realloc(line, max_len);
         }
         line[len] = (char)ch;
         len += 1;
      }
      fclose(fd);
      result = ProcessBuffer(context, line);
      JLRelease(context, result);
   } else {
      for(;;) {
         printf("> "); fflush(stdout);
         const ssize_t len = getline(&line, &cap, stdin);
         if(len <= 0) {
            break;
         }
         result = ProcessBuffer(context, line);
         printf("=> ");
         JLPrint(context, result);
         printf("\n");
         JLRelease(context, result);
      }
   }
   if(line) {
      free(line);
   }
   JLDestroyContext(context);

   return 0;

}

