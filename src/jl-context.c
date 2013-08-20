/**
 * @file jl-context.c
 * @author Joe Wingbermuehle
 */

#include "jl.h"
#include "jl-context.h"
#include "jl-scope.h"
#include "jl-value.h"

#include <stdlib.h>

typedef struct FreeNode {
   union {
      BindingNode        binding;
      ScopeNode          scope;
      JLValue            value;
      struct FreeNode   *next;
   };
} FreeNode;

void *GetFree(JLContext *context)
{
   FreeNode *node;
   if(context->freelist) {
      node = context->freelist;
      context->freelist = node->next;
   } else {
      node = (FreeNode*)malloc(sizeof(FreeNode));
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
   while(context->freelist) {
      FreeNode *next = context->freelist->next;
      free(context->freelist);
      context->freelist = next;
   }
   free(context);
}

