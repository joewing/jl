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
static void InvalidArgumentError(JLContext *context, JLValue *args);
static void TooManyArgumentsError(JLContext *context, JLValue *args);
static void TooFewArgumentsError(JLContext *context, JLValue *args);

static JLValue *CompareFunc(JLContext *context, JLValue *args);
static JLValue *AddFunc(JLContext *context, JLValue *args);
static JLValue *SubFunc(JLContext *context, JLValue *args);
static JLValue *MulFunc(JLContext *context, JLValue *args);
static JLValue *DivFunc(JLContext *context, JLValue *args);
static JLValue *ModFunc(JLContext *context, JLValue *args);
static JLValue *AndFunc(JLContext *context, JLValue *args);
static JLValue *OrFunc(JLContext *context, JLValue *args);
static JLValue *NotFunc(JLContext *context, JLValue *args);
static JLValue *BeginFunc(JLContext *context, JLValue *args);
static JLValue *ConsFunc(JLContext *context, JLValue *args);
static JLValue *DefineFunc(JLContext *context, JLValue *args);
static JLValue *HeadFunc(JLContext *context, JLValue *args);
static JLValue *IfFunc(JLContext *context, JLValue *args);
static JLValue *LambdaFunc(JLContext *context, JLValue *args);
static JLValue *ListFunc(JLContext *context, JLValue *args);
static JLValue *RestFunc(JLContext *context, JLValue *args);
static JLValue *SubstrFunc(JLContext *context, JLValue *args);
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
   { "substr",    SubstrFunc     },
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

void InvalidArgumentError(JLContext *context, JLValue *args)
{
   Error(context, "invalid argument to %s", args->value.str);
}

void TooManyArgumentsError(JLContext *context, JLValue *args)
{
   Error(context, "too many arguments to %s", args->value.str);
}

void TooFewArgumentsError(JLContext *context, JLValue *args)
{
   Error(context, "too few arguments to %s", args->value.str);
}

JLValue *CompareFunc(JLContext *context, JLValue *args)
{
   const char *op = args->value.str;
   JLValue *va = NULL;
   JLValue *vb = NULL;
   JLValue *result = NULL;
   char cond = 0;
   if(args->next == NULL || args->next->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   va = JLEvaluate(context, args->next);
   vb = JLEvaluate(context, args->next->next);
   if(va == NULL || vb == NULL || va->tag != vb->tag) {

      if(op[0] == '=') {
         cond = va == vb;
      } else if(op[0] == '!') {
         cond = va != vb;
      } else {
         InvalidArgumentError(context, args);
      }

   } else {

      /* Here we know that va and vb are not nil and are of the same type. */
      double diff = 0.0;
      if(va->tag == JLVALUE_NUMBER) {
         diff = va->value.number - vb->value.number;
      } else if(va->tag == JLVALUE_STRING) {
         diff = strcmp(va->value.str, vb->value.str);
      } else {
         InvalidArgumentError(context, args);
      }

      if(op[0] == '=') {
         cond = diff == 0.0;
      } else if(op[0] == '!') {
         cond = diff != 0.0;
      } else if(op[0] == '<' && op[1] == 0) {
         cond = diff < 0.0;
      } else if(op[0] == '<' && op[1] == '=') {
         cond = diff <= 0.0;
      } else if(op[0] == '>' && op[1] == 0) {
         cond = diff > 0.0;
      } else if(op[0] == '>' && op[1] == '=') {
         cond = diff >= 0.0;
      }

   }

   if(cond) {
      result = JLDefineNumber(context, NULL, 1.0);
   }

   JLRelease(context, va);
   JLRelease(context, vb);
   return result;
}

JLValue *AddFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   double sum = 0.0;
   for(vp = args->next; vp; vp = vp->next) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg == NULL || arg->tag != JLVALUE_NUMBER) {
         InvalidArgumentError(context, args);
         JLRelease(context, arg);
         return NULL;
      }
      sum += arg->value.number;
      JLRelease(context, arg);
   }
   return JLDefineNumber(context, NULL, sum);
}

JLValue *SubFunc(JLContext *context, JLValue *args)
{
   JLValue *vp = args->next;
   JLValue *arg = NULL;
   double total = 0.0;

   arg = JLEvaluate(context, vp);
   if(arg == NULL || arg->tag != JLVALUE_NUMBER) {
      InvalidArgumentError(context, args);
      JLRelease(context, arg);
      return NULL;
   }
   total = arg->value.number;
   JLRelease(context, arg);

   for(vp = vp->next; vp; vp = vp->next) {
      arg = JLEvaluate(context, vp);
      if(arg == NULL || arg->tag != JLVALUE_NUMBER) {
         InvalidArgumentError(context, args);
         JLRelease(context, arg);
         return NULL;
      }
      total -= arg->value.number;
      JLRelease(context, arg);
   }

   return JLDefineNumber(context, NULL, total);

}

JLValue *MulFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   double product = 1.0;
   for(vp = args->next; vp; vp = vp->next) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg == NULL || arg->tag != JLVALUE_NUMBER) {
         InvalidArgumentError(context, args);
         JLRelease(context, arg);
         return NULL;
      }
      product *= arg->value.number;
      JLRelease(context, arg);
   }
   return JLDefineNumber(context, NULL, product);
}

JLValue *DivFunc(JLContext *context, JLValue *args)
{
   JLValue *va = NULL;
   JLValue *vb = NULL;
   JLValue *result = NULL;

   va = JLEvaluate(context, args->next);
   if(va == NULL || va->tag != JLVALUE_NUMBER) {
      InvalidArgumentError(context, args);
      goto div_done;
   }
   vb = JLEvaluate(context, args->next->next);
   if(vb == NULL || vb->tag != JLVALUE_NUMBER) {
      InvalidArgumentError(context, args);
      goto div_done;
   }
   if(args->next->next->next) {
      TooManyArgumentsError(context, args);
      goto div_done;
   }

   result = JLDefineNumber(context, NULL, va->value.number / vb->value.number);

div_done:

   JLRelease(context, va);
   JLRelease(context, vb);
   return result;

}

JLValue *ModFunc(JLContext *context, JLValue *args)
{

   JLValue *va = NULL;
   JLValue *vb = NULL;
   JLValue *result = NULL;
   long temp;

   va = JLEvaluate(context, args->next);
   if(va == NULL || va->tag != JLVALUE_NUMBER) {
      InvalidArgumentError(context, args);
      goto mod_done;
   }
   vb = JLEvaluate(context, args->next->next);
   if(vb == NULL || vb->tag != JLVALUE_NUMBER) {
      InvalidArgumentError(context, args);
      goto mod_done;
   }
   if(args->next->next->next) {
      TooManyArgumentsError(context, args);
      goto mod_done;
   }
   temp = (long)vb->value.number;
   if(temp == 0) {
      goto mod_done;
   }

   result = JLDefineNumber(context, NULL, (long)va->value.number % temp);

mod_done:

   JLRelease(context, va);
   JLRelease(context, vb);
   return result;

}

JLValue *AndFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   for(vp = args->next; vp; vp = vp->next) {
      if(!CheckCondition(context, vp)) {
         return NULL;
      }
   }
   return JLDefineNumber(context, NULL, 1.0);
}

JLValue *OrFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   for(vp = args->next; vp; vp = vp->next) {
      if(CheckCondition(context, vp)) {
         return JLDefineNumber(context, NULL, 1.0);
      }
   }
   return NULL;
}

JLValue *NotFunc(JLContext *context, JLValue *args)
{
   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next != NULL) {
      TooManyArgumentsError(context, args);
      return NULL;
   }
   if(!CheckCondition(context, args->next)) {
      return JLDefineNumber(context, NULL, 1.0);
   } else {
      return NULL;
   }
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
   JLValue *head = NULL;
   JLValue *rest = NULL;
   JLValue *temp = NULL;
   JLValue *result = NULL;

   if(args->next == NULL || args->next->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   rest = JLEvaluate(context, args->next->next);
   if(rest != NULL && rest->tag != JLVALUE_LIST) {
      InvalidArgumentError(context, args);
      JLRelease(context, rest);
      return NULL;
   }

   temp = JLEvaluate(context, args->next);
   head = CopyValue(context, temp);
   JLRelease(context, temp);

   result = CreateValue(context, NULL, JLVALUE_LIST);
   if(rest) {
      head->next = rest->value.lst;
      JLRetain(context, rest->value.lst);
      JLRelease(context, rest);
   }
   result->value.lst = head;

   return result;
}

JLValue *DefineFunc(JLContext *context, JLValue *args)
{
   JLValue *vp = args->next;
   JLValue *result = NULL;
   if(vp == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(vp->tag != JLVALUE_VARIABLE) {
      InvalidArgumentError(context, args);
      return NULL;
   }
   result = JLEvaluate(context, vp->next);
   JLDefineValue(context, vp->value.str, result);
   return result;
}

JLValue *HeadFunc(JLContext *context, JLValue *args)
{
   JLValue *result = NULL;
   JLValue *vp = JLEvaluate(context, args->next);

   if(vp == NULL || vp->tag != JLVALUE_LIST) {
      InvalidArgumentError(context, args);
      goto head_done;
   }

   result = vp->value.lst;
   JLRetain(context, result);

head_done:

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

JLValue *LambdaFunc(JLContext *context, JLValue *args)
{
   JLValue *result;
   JLValue *scope;

   if(args->next == NULL || args->next->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }

   scope = CreateValue(context, NULL, JLVALUE_SCOPE);
   scope->value.scope = context->scope;
   context->scope->count += 1;

   result = CreateValue(context, NULL, JLVALUE_LAMBDA);
   result->value.lst = scope;
   result->value.lst->next = args->next;
   JLRetain(context, args->next);

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

   if(vp == NULL || vp->tag != JLVALUE_LIST) {
      InvalidArgumentError(context, args);
      goto rest_done;
   }

   if(vp->value.lst && vp->value.lst->next) {
      result = CreateValue(context, NULL, JLVALUE_LIST);
      result->value.lst = vp->value.lst->next;
      JLRetain(context, result->value.lst);
   }

rest_done:

   JLRelease(context, vp);
   return result;
}

JLValue *SubstrFunc(JLContext *context, JLValue *args)
{
   JLValue *result   = NULL;
   JLValue *str      = NULL;
   JLValue *sval     = NULL;
   JLValue *lval     = NULL;
   size_t start      = 0;
   size_t len        = (size_t)-1;
   size_t slen;

   str = JLEvaluate(context, args->next);
   if(!str || str->tag != JLVALUE_STRING) {
      InvalidArgumentError(context, args);
      goto substr_done;
   }

   sval = JLEvaluate(context, args->next->next);
   if(sval) {
      if(sval->tag != JLVALUE_NUMBER) {
         InvalidArgumentError(context, args);
         goto substr_done;
      }
      start = (size_t)sval->value.number;
   }

   if(args->next->next) {
      if(args->next->next->next && args->next->next->next->next) {
         TooManyArgumentsError(context, args);
         goto substr_done;
      }
      lval = JLEvaluate(context, args->next->next->next);
      if(lval) {
         if(lval->tag != JLVALUE_NUMBER) {
            InvalidArgumentError(context, args);
            goto substr_done;
         }
         len = (size_t)lval->value.number;
      }
   }

   slen = strlen(str->value.str);
   if(start < slen && len > 0) {
      len = slen - start > len ? len : slen - start;
      result = CreateValue(context, NULL, JLVALUE_STRING);
      result->value.str = (char*)malloc(len + 1);
      memcpy(result->value.str, &str->value.str[start], len);
      result->value.str[len] = 0;
   }

substr_done:

   JLRelease(context, str);
   JLRelease(context, sval);
   JLRelease(context, lval);

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
      if(arg == NULL || arg->tag != JLVALUE_STRING) {
         InvalidArgumentError(context, args);
         JLRelease(context, arg);
         JLRelease(context, result);
         return NULL;
      } else {
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
   JLValue *arg = NULL;
   JLValue *result = NULL;

   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   arg = JLEvaluate(context, args->next);
   if(arg && arg->tag == JLVALUE_NUMBER) {
      result = JLDefineNumber(context, NULL, 1.0);
   }
   JLRelease(context, arg);
   return result;
}

JLValue *IsStringFunc(JLContext *context, JLValue *args)
{
   JLValue *arg = NULL;
   JLValue *result = NULL;

   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   arg = JLEvaluate(context, args->next);
   if(arg && arg->tag == JLVALUE_STRING) {
      result = JLDefineNumber(context, NULL, 1.0);
   }
   JLRelease(context, arg);
   return result;

}

JLValue *IsListFunc(JLContext *context, JLValue *args)
{
   JLValue *arg = NULL;
   JLValue *result = NULL;

   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   arg = JLEvaluate(context, args->next);
   if(arg && arg->tag == JLVALUE_LIST) {
      result = JLDefineNumber(context, NULL, 1.0);
   }
   JLRelease(context, arg);
   return result;
}

JLValue *IsNullFunc(JLContext *context, JLValue *args)
{
   JLValue *arg = NULL;

   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   arg = JLEvaluate(context, args->next);
   if(arg == NULL) {
      return JLDefineNumber(context, NULL, 1.0);
   } else {
      JLRelease(context, arg);
      return NULL;
   }
}

void RegisterFunctions(JLContext *context)
{
   size_t i;
   for(i = 0; i < INTERNAL_FUNCTION_COUNT; i++) {
      JLDefineSpecial(context, INTERNAL_FUNCTIONS[i].name,
                      INTERNAL_FUNCTIONS[i].function);
   }
}

