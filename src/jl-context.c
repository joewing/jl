/**
 * @file jl-context.c
 * @author Joe Wingbermuehle
 */

#include "jl.h"
#include "jl-context.h"
#include "jl-scope.h"
#include "jl-value.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define BLOCK_SIZE   8192

typedef struct FreeNode {
   union {
      BindingNode        binding;
      ScopeNode          scope;
      JLValue            value;
      struct FreeNode   *next;
   };
} FreeNode;

typedef struct BlockNode {
   FreeNode nodes[BLOCK_SIZE];
   struct BlockNode *next;
} BlockNode;

void *GetFree(JLContext *context)
{
   FreeNode *node;
   if(context->freelist == NULL) {
      BlockNode *block = (BlockNode*)malloc(sizeof(BlockNode));
      size_t i;
      block->next = context->blocks;
      context->blocks = block;
      for(i = 1; i < BLOCK_SIZE; i++) {
         block->nodes[i].next = context->freelist;
         context->freelist = &block->nodes[i];
      }
      node = &block->nodes[0];
   } else {
      node = context->freelist;
      context->freelist = node->next;
   }
   return node;
}

void PutFree(JLContext *context, void *value)
{
   FreeNode *temp = (FreeNode*)value;
   temp->next = context->freelist;
   context->freelist = temp;
}

void FreeContext(JLContext *context)
{
   while(context->blocks) {
      BlockNode *next = context->blocks->next;
      free(context->blocks);
      context->blocks = next;
   }
   free(context);
}

void Error(JLContext *context, const char *msg, ...)
{
   va_list ap;
   va_start(ap, msg);
   context->error = 1;
   printf("ERROR[%d]: ", context->line);
   vprintf(msg, ap);
   printf("\n");
   va_end(ap);
}

