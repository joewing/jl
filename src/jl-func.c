/**
 * @file jl-func.h
 * @author Joe Wingbermuehle
 */

#include "jl.h"
#include "jl-func.h"
#include "jl-value.h"
#include "jl-context.h"
#include "jl-scope.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct InternalFunctionNode {
   const char *name;
   JLFunction function;
} InternalFunctionNode;

static char CheckCondition(JLContext *context, JLValue *value);

static JLValue *CompareFunc(JLContext *context, JLValue *value);
static JLValue *AddFunc(JLContext *context, JLValue *value);
static JLValue *SubFunc(JLContext *context, JLValue *value);
static JLValue *MulFunc(JLContext *context, JLValue *value);
static JLValue *DivFunc(JLContext *context, JLValue *value);
static JLValue *ModFunc(JLContext *context, JLValue *value);
static JLValue *AndFunc(JLContext *context, JLValue *value);
static JLValue *OrFunc(JLContext *context, JLValue *value);
static JLValue *NotFunc(JLContext *context, JLValue *value);
static JLValue *BeginFunc(JLContext *context, JLValue *value);
static JLValue *ConsFunc(JLContext *context, JLValue *value);
static JLValue *DefineFunc(JLContext *context, JLValue *value);
static JLValue *HeadFunc(JLContext *context, JLValue *value);
static JLValue *IfFunc(JLContext *context, JLValue *value);
static JLValue *LambdaFunc(JLContext *context, JLValue *value);
static JLValue *ListFunc(JLContext *context, JLValue *value);
static JLValue *RestFunc(JLContext *context, JLValue *value);
static JLValue *CharFunc(JLContext *context, JLValue *args);
static JLValue *ConcatFunc(JLContext *context, JLValue *args);
static JLValue *IsNumberFunc(JLContext *context, JLValue *args);
static JLValue *IsStringFunc(JLContext *context, JLValue *args);
static JLValue *IsListFunc(JLContext *context, JLValue *args);
static JLValue *IsNullFunc(JLContext *context, JLValue *args);

static InternalFunctionNode INTERNAL_FUNCTIONS[] = {
   { "=",         CompareFunc    },
   { "!=",        CompareFunc    },
   { ">",         CompareFunc    },
   { ">=",        CompareFunc    },
   { "<",         CompareFunc    },
   { "<=",        CompareFunc    },
   { "+",         AddFunc        },
   { "-",         SubFunc        },
   { "*",         MulFunc        },
   { "/",         DivFunc        },
   { "mod",       ModFunc        },
   { "and",       AndFunc        },
   { "or",        OrFunc         },
   { "not",       NotFunc        },
   { "begin",     BeginFunc      },
   { "cons",      ConsFunc       },
   { "define",    DefineFunc     },
   { "head",      HeadFunc       },
   { "if",        IfFunc         },
   { "lambda",    LambdaFunc     },
   { "list",      ListFunc       },
   { "rest",      RestFunc       },
   { "char",      CharFunc       },
   { "concat",    ConcatFunc     },
   { "number?",   IsNumberFunc   },
   { "string?",   IsStringFunc   },
   { "list?",     IsListFunc     },
   { "null?",     IsNullFunc     }
};
static size_t INTERNAL_FUNCTION_COUNT = sizeof(INTERNAL_FUNCTIONS)
                                      / sizeof(InternalFunctionNode);

char CheckCondition(JLContext *context, JLValue *value)
{
   JLValue *cond = JLEvaluate(context, value);
   char rc = 0;
   if(cond) {
      switch(cond->tag) {
      case JLVALUE_NUMBER:
         rc = cond->value.number != 0.0;
         break;
      case JLVALUE_LIST:
         rc = cond->value.lst != NULL;
         break;
      default:
         rc = 1;
         break;
      }
      JLRelease(context, cond);
   }
   return rc;
}

JLValue *CompareFunc(JLContext *context, JLValue *args)
{
   JLValue *result = NULL;
   JLValue *va = JLEvaluate(context, args->next);
   if(va && va->tag == JLVALUE_NUMBER) {
      JLValue *vb = JLEvaluate(context, args->next->next);
      if(vb && vb->tag == JLVALUE_NUMBER) {
         const double a = va->value.number;
         const double b = vb->value.number;
         result = JLDefineNumber(context, NULL, 0.0);
         switch(args->value.str[0]) {
         case '=':
            result->value.number = a == b;
            break;
         case '!':
            result->value.number = a != b;
            break;
         case '<':
            if(args->value.str[1] == '=') {
               result->value.number = a <= b;
            } else {
               result->value.number = a < b;
            }
            break;
         case '>':
            if(args->value.str[1] == '=') {
               result->value.number = a >= b;
            } else {
               result->value.number = a > b;
            }
            break;
         default:
            break;
         }
      }
      JLRelease(context, vb);
   }
   JLRelease(context, va);
   return result;
}

JLValue *AddFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   for(vp = args->next; vp; vp = vp->next) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg && arg->tag == JLVALUE_NUMBER) {
         result->value.number += arg->value.number;
      }
      JLRelease(context, arg);
   }
   return result;
}

JLValue *SubFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   vp = args->next;
   if(vp) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg && arg->tag == JLVALUE_NUMBER) {
         result->value.number = arg->value.number;
      }
      JLRelease(context, arg);
      for(vp = vp->next; vp; vp = vp->next) {
         arg = JLEvaluate(context, vp);
         if(arg && arg->tag == JLVALUE_NUMBER) {
            result->value.number -= arg->value.number;
         }
         JLRelease(context, arg);
      }
   }
   return result;
}

JLValue *MulFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   JLValue *result = JLDefineNumber(context, NULL, 1.0);
   for(vp = args->next; vp; vp = vp->next) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg) {
         result->value.number *= arg->value.number;
      }
      JLRelease(context, arg);
   }
   return result;
}

JLValue *DivFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   vp = args->next;
   if(vp) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg && arg->tag == JLVALUE_NUMBER) {
         result->value.number = arg->value.number;
      }
      JLRelease(context, arg);
      for(vp = vp->next; vp; vp = vp->next) {
         arg = JLEvaluate(context, vp);
         if(arg) {
            result->value.number /= arg->value.number;
         }
         JLRelease(context, arg);
      }
   }
   return result;
}

JLValue *ModFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   vp = args->next;
   if(vp) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg && arg->tag == JLVALUE_NUMBER) {
         result->value.number = arg->value.number;
      }
      JLRelease(context, arg);
      for(vp = vp->next; vp; vp = vp->next) {
         arg = JLEvaluate(context, vp);
         if(arg) {
            const long d = (long)arg->value.number;
            if(d) {
               result->value.number = (long)result->value.number % d;
            } else {
               result->value.number = 0.0;
            }
         }
         JLRelease(context, arg);
      }
   }
   return result;
}

JLValue *AndFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   JLValue *result = JLDefineNumber(context, NULL, 1.0);
   for(vp = args->next; vp; vp = vp->next) {
      if(!CheckCondition(context, vp)) {
         result->value.number = 0.0;
         break;
      }
   }
   return result;
}

JLValue *OrFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   for(vp = args->next; vp; vp = vp->next) {
      if(CheckCondition(context, vp)) {
         result->value.number = 1.0;
         break;
      }
   }
   return result;
}

JLValue *NotFunc(JLContext *context, JLValue *args)
{
   JLValue *vp = JLEvaluate(context, args->next);
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   if(vp && vp->tag == JLVALUE_NUMBER) {
      result->value.number = vp->value.number == 0.0 ? 1.0 : 0.0;
   }
   JLRelease(context, vp);
   return result;
}

JLValue *BeginFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   JLValue *result = NULL;
   EnterScope(context);
   for(vp = args->next; vp; vp = vp->next) {
      JLRelease(context, result);
      result = JLEvaluate(context, vp);
   }
   LeaveScope(context);
   return result;
}

JLValue *ConsFunc(JLContext *context, JLValue *args)
{
   JLValue *value = JLEvaluate(context, args->next);
   if(value) {
      JLValue *head = CopyValue(context, value);
      JLValue *result = CreateValue(context, NULL, JLVALUE_LIST);
      JLValue *rest = JLEvaluate(context, args->next->next);
      if(rest && rest->tag == JLVALUE_LIST) {
         head->next = rest->value.lst;
         JLRetain(rest->value.lst);
      }
      JLRelease(context, rest);
      JLRelease(context, value);
      result->value.lst = head;
      return result;
   }
   return NULL;
}

JLValue *DefineFunc(JLContext *context, JLValue *args)
{
   JLValue *vp = args->next;
   JLValue *result = NULL;
   if(vp && vp->tag == JLVALUE_VARIABLE) {
      result = JLEvaluate(context, vp->next);
      JLDefineValue(context, vp->value.str, result);
   }
   return result;
}

JLValue *HeadFunc(JLContext *context, JLValue *args)
{
   JLValue *result = NULL;
   JLValue *vp = JLEvaluate(context, args->next);
   if(vp && vp->tag == JLVALUE_LIST) {
      result = vp->value.lst;
      JLRetain(result);
   }
   JLRelease(context, vp);
   return result;
}

JLValue *IfFunc(JLContext *context, JLValue *args)
{
   JLValue *vp = args->next;
   if(CheckCondition(context, vp)) {
      return JLEvaluate(context, vp->next);
   } else if(vp->next) {
      return JLEvaluate(context, vp->next->next);
   }
   return NULL;
}

JLValue *LambdaFunc(JLContext *context, JLValue *value)
{
   JLValue *result = CreateValue(context, NULL, JLVALUE_LAMBDA);
   JLValue *scope = CreateValue(context, NULL, JLVALUE_SCOPE);
   scope->value.scope = context->scope;
   context->scope->count += 1;
   result->value.lst = scope;
   result->value.lst->next = value->next;
   JLRetain(value->next);
   return result;
}

JLValue *ListFunc(JLContext *context, JLValue *args)
{
   JLValue *result = NULL;
   if(args->next) {
      result = CreateValue(context, NULL, JLVALUE_LIST);
      JLValue **item = &result->value.lst;
      JLValue *vp;
      for(vp = args->next; vp; vp = vp->next) {
         JLValue *arg = JLEvaluate(context, vp);
         JLValue *temp = CopyValue(context, arg);
         *item = temp;
         item = &temp->next;
         JLRelease(context, arg);
      }
   }
   return result;
}

JLValue *RestFunc(JLContext *context, JLValue *args)
{
   JLValue *result = NULL;
   JLValue *vp = JLEvaluate(context, args->next);
   if(vp && vp->tag == JLVALUE_LIST) {
      if(vp->value.lst && vp->value.lst->next) {
         result = CreateValue(context, NULL, JLVALUE_LIST);
         result->value.lst = vp->value.lst->next;
         JLRetain(result->value.lst);
      }
   }
   JLRelease(context, vp);
   return result;
}

JLValue *CharFunc(JLContext *context, JLValue *args)
{
   JLValue *result = NULL;
   JLValue *str = JLEvaluate(context, args->next);
   if(str && str->tag == JLVALUE_STRING) {
      JLValue *index = JLEvaluate(context, args->next->next);
      if(index && index->tag == JLVALUE_NUMBER) {
         const size_t len = strlen(str->value.str);
         const size_t i = (size_t)index->value.number;
         if(i < len) {
            result = CreateValue(context, NULL, JLVALUE_STRING);
            result->value.str = (char*)malloc(2);
            result->value.str[0] = str->value.str[i];
            result->value.str[1] = 0;
         }
      }
      JLRelease(context, index);
   }
   JLRelease(context, str);
   return result;
}

JLValue *ConcatFunc(JLContext *context, JLValue *args)
{
   JLValue *result = CreateValue(context, NULL, JLVALUE_STRING);
   JLValue *vp;
   size_t len = 0;
   size_t max_len = 8;
   result->value.str = (char*)malloc(max_len);
   for(vp = args->next; vp; vp = vp->next) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg && arg->tag == JLVALUE_STRING) {
         const size_t l = strlen(arg->value.str);
         const size_t new_len = len + l;
         if(new_len >= max_len) {
            max_len = new_len + 1;
            result->value.str = (char*)realloc(result->value.str, max_len);
         }
         memcpy(&result->value.str[len], arg->value.str, l);
         len = new_len;
      }
      JLRelease(context, arg);
   }
   result->value.str[len] = 0;
   return result;
}

JLValue *IsNumberFunc(JLContext *context, JLValue *args)
{
   JLValue *arg = JLEvaluate(context, args->next);
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   result->value.number = (arg && arg->tag == JLVALUE_NUMBER) ? 1.0 : 0.0;
   JLRelease(context, arg);
   return result;
}

JLValue *IsStringFunc(JLContext *context, JLValue *args)
{
   JLValue *arg = JLEvaluate(context, args->next);
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   result->value.number = (arg && arg->tag == JLVALUE_STRING) ? 1.0 : 0.0;
   JLRelease(context, arg);
   return result;
}

JLValue *IsListFunc(JLContext *context, JLValue *args)
{
   JLValue *arg = JLEvaluate(context, args->next);
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   result->value.number = (arg && arg->tag == JLVALUE_LIST) ? 1.0 : 0.0;
   JLRelease(context, arg);
   return result;
}

JLValue *IsNullFunc(JLContext *context, JLValue *args)
{
   JLValue *arg = JLEvaluate(context, args->next);
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   result->value.number = arg == NULL ? 1.0 : 0.0;
   JLRelease(context, arg);
   return result;
}

void RegisterFunctions(JLContext *context)
{
   size_t i;
   for(i = 0; i < INTERNAL_FUNCTION_COUNT; i++) {
      JLDefineSpecial(context, INTERNAL_FUNCTIONS[i].name,
                      INTERNAL_FUNCTIONS[i].function);
   }
}

