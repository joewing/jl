/**
 * @file jl-scope.c
 * @author Joe Wingbermuehle
 */

#include "jl-scope.h"
#include "jl-context.h"
#include "jl-value.h"

#include <stdlib.h>
#include <string.h>

static unsigned int CountScopeBindings(BindingNode *binding, ScopeNode *scope);
static void ReleaseBindings(JLContext *context, BindingNode *binding);

unsigned int CountScopeBindings(BindingNode *binding, ScopeNode *scope)
{
   unsigned int count = 0;
   if(binding) {
      count += CountScopeBindings(binding->left, scope);
      count += CountScopeBindings(binding->right, scope);
      if(binding->value &&
         binding->value->tag == JLVALUE_LAMBDA &&
         binding->value->count == 1) {
         if(binding->value->value.lst->value.scope == scope) {
            count += 1;
         }
      }
   }
   return count;
}

void ReleaseBindings(JLContext *context, BindingNode *binding)
{
   if(binding) {
      ReleaseBindings(context, binding->left);
      ReleaseBindings(context, binding->right);
      free(binding->name);
      JLRelease(context, binding->value);
      PutFree(context, binding);
   }
}

void EnterScope(JLContext *context)
{
   ScopeNode *scope = (ScopeNode*)GetFree(context);
   scope->count = 1;
   scope->bindings = NULL;
   scope->next = context->scope;
   context->scope = scope;
}

void LeaveScope(JLContext *context)
{
   ScopeNode *scope = context->scope;
   context->scope = scope->next;
   ReleaseScope(context, scope);
}

void ReleaseScope(JLContext *context, ScopeNode *scope)
{
   const unsigned int new_count = scope->count - 1
                                - CountScopeBindings(scope->bindings, scope);
   if(new_count == 0) {
      ReleaseBindings(context, scope->bindings);
      PutFree(context, scope);
   } else {
      scope->count -= 1;
   }
}

JLValue *Lookup(JLContext *context, const char *name)
{
   const ScopeNode *scope = context->scope;
   while(scope) {
      const BindingNode *binding = scope->bindings;
      while(binding) {
         const int v = strcmp(binding->name, name);
         if(v < 0) {
            binding = binding->left;
         } else if(v > 0) {
            binding = binding->right;
         } else {
            return binding->value;
         }
      }
      scope = scope->next;
   }
   return NULL;
}

