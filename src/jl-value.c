/**
 * @file jl-value.c
 * @author Joe Wingbermuehle
 */

#include "jl-value.h"
#include "jl-context.h"
#include <string.h>

JLValue *CreateValue(JLContext *context, const char *name, JLValueType tag)
{
   JLValue *result = (JLValue*)GetFree(context);
   result->tag = tag;
   result->next = NULL;
   result->count = 1;
   JLDefineValue(context, name, result);
   return result;
}

JLValue *CopyValue(JLContext *context, const JLValue *other)
{
   JLValue *result = NULL;
   if(other) {
      result = CreateValue(context, NULL, other->tag);
      result->value = other->value;
      switch(result->tag) {
      case JLVALUE_LIST:
      case JLVALUE_LAMBDA:
      case JLVALUE_SCOPE:
         JLRetain(context, result->value.lst);
         break;
      case JLVALUE_STRING:
      case JLVALUE_VARIABLE:
         result->value.str = strdup(result->value.str);
         break;
      default:
         break;
      }
   } else {
      result = CreateValue(context, NULL, JLVALUE_NIL);
   }
   return result;
}

