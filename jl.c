/**
 * @file jl.c
 * @author Joe Wingbermuehle
 */

#include "jl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct InternalFunctionNode {
   const char *name;
   JLFunction function;
} InternalFunctionNode;

typedef struct BindingNode {
   char *name;
   JLValue *value;
   struct BindingNode *next;
} BindingNode;

typedef struct ScopeNode {
   BindingNode *bindings;
   struct ScopeNode *next;
} ScopeNode;

typedef struct JLContext {
   ScopeNode *scope;
   int line;
} JLContext;

static JLValue *CreateValue(JLContext *context,
                            const char *name,
                            JLValueType tag);
static void DestroyValue(JLValue *value);
static void EnterScope(JLContext *context);
static void LeaveScope(JLContext *context);
static JLValue *Lookup(JLContext *context, const char *name);
static JLValue *EvalLambda(JLContext *context,
                           const JLValue *lambda,
                           JLValue *args);
static JLValue *Parse(JLContext *context, const char **line);
static void ParseError(JLContext *context, const char *msg);
static char IsSymbol(char ch);
static char IsSpace(char ch);

static JLValue *EqFunc(JLContext *context, JLValue *value);
static JLValue *NeFunc(JLContext *context, JLValue *value);
static JLValue *GtFunc(JLContext *context, JLValue *value);
static JLValue *GeFunc(JLContext *context, JLValue *value);
static JLValue *LtFunc(JLContext *context, JLValue *value);
static JLValue *LeFunc(JLContext *context, JLValue *value);
static JLValue *AddFunc(JLContext *context, JLValue *value);
static JLValue *SubFunc(JLContext *context, JLValue *value);
static JLValue *MulFunc(JLContext *context, JLValue *value);
static JLValue *DivFunc(JLContext *context, JLValue *value);
static JLValue *ConsFunc(JLContext *context, JLValue *value);
static JLValue *DefineFunc(JLContext *context, JLValue *value);
static JLValue *HeadFunc(JLContext *context, JLValue *value);
static JLValue *IfFunc(JLContext *context, JLValue *value);
static JLValue *LambdaFunc(JLContext *context, JLValue *value);
static JLValue *LetFunc(JLContext *context, JLValue *value);
static JLValue *ListFunc(JLContext *context, JLValue *value);
static JLValue *RestFunc(JLContext *context, JLValue *value);

static InternalFunctionNode INTERNAL_FUNCTIONS[] = {
   { "=",         EqFunc      },
   { "!=",        NeFunc      },
   { ">",         GtFunc      },
   { ">=",        GeFunc      },
   { "<",         LtFunc      },
   { "<=",        LeFunc      },
   { "+",         AddFunc     },
   { "-",         SubFunc     },
   { "*",         MulFunc     },
   { "/",         DivFunc     },
   { "cons",      ConsFunc    },
   { "define",    DefineFunc  },
   { "head",      HeadFunc    },
   { "if",        IfFunc      },
   { "lambda",    LambdaFunc  },
   { "let",       LetFunc     },
   { "list",      ListFunc    },
   { "rest",      RestFunc    }
};
static size_t INTERNAL_FUNCTION_COUNT = sizeof(INTERNAL_FUNCTIONS)
                                      / sizeof(InternalFunctionNode);

JLValue *EqFunc(JLContext *context, JLValue *args)
{
   JLValue *va;
   va = JLEvaluate(context, args->next);
   if(va && va->tag == JLVALUE_NUMBER) {
      JLValue *vb = JLEvaluate(context, args->next->next);
      if(vb && vb->tag == JLVALUE_NUMBER) {
         JLValue *result = JLDefineNumber(context, NULL, 0.0);
         if(va->value.number == vb->value.number) {
            result->value.number = 1.0;
         }
         return result;
      }
   }
   return NULL;
}

JLValue *NeFunc(JLContext *context, JLValue *args)
{
   JLValue *va;
   va = JLEvaluate(context, args->next);
   if(va && va->tag == JLVALUE_NUMBER) {
      JLValue *vb = JLEvaluate(context, args->next->next);
      if(vb && vb->tag == JLVALUE_NUMBER) {
         JLValue *result = JLDefineNumber(context, NULL, 0.0);
         if(va->value.number != vb->value.number) {
            result->value.number = 1.0;
         }
         return result;
      }
   }
   return NULL;
}

JLValue *GtFunc(JLContext *context, JLValue *args)
{
   JLValue *va;
   va = JLEvaluate(context, args->next);
   if(va && va->tag == JLVALUE_NUMBER) {
      JLValue *vb = JLEvaluate(context, args->next->next);
      if(vb && vb->tag == JLVALUE_NUMBER) {
         JLValue *result = JLDefineNumber(context, NULL, 0.0);
         if(va->value.number > vb->value.number) {
            result->value.number = 1.0;
         }
         return result;
      }
   }
   return NULL;
}

JLValue *GeFunc(JLContext *context, JLValue *args)
{
   JLValue *va;
   va = JLEvaluate(context, args->next);
   if(va && va->tag == JLVALUE_NUMBER) {
      JLValue *vb = JLEvaluate(context, args->next->next);
      if(vb && vb->tag == JLVALUE_NUMBER) {
         JLValue *result = JLDefineNumber(context, NULL, 0.0);
         if(va->value.number >= vb->value.number) {
            result->value.number = 1.0;
         }
         return result;
      }
   }
   return NULL;
}

JLValue *LtFunc(JLContext *context, JLValue *args)
{
   JLValue *va;
   va = JLEvaluate(context, args->next);
   if(va && va->tag == JLVALUE_NUMBER) {
      JLValue *vb = JLEvaluate(context, args->next->next);
      if(vb && vb->tag == JLVALUE_NUMBER) {
         JLValue *result = JLDefineNumber(context, NULL, 0.0);
         if(va->value.number < vb->value.number) {
            result->value.number = 1.0;
         }
         return result;
      }
   }
   return NULL;
}

JLValue *LeFunc(JLContext *context, JLValue *args)
{
   JLValue *va;
   va = JLEvaluate(context, args->next);
   if(va && va->tag == JLVALUE_NUMBER) {
      JLValue *vb = JLEvaluate(context, args->next->next);
      if(vb && vb->tag == JLVALUE_NUMBER) {
         JLValue *result = JLDefineNumber(context, NULL, 0.0);
         if(va->value.number <= vb->value.number) {
            result->value.number = 1.0;
         }
         return result;
      }
   }
   return NULL;
}

JLValue *AddFunc(JLContext *context, JLValue *args)
{
   JLValue *vp;
   JLValue *result = JLDefineNumber(context, NULL, 0.0);
   for(vp = args->next; vp; vp = vp->next) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg) {
         result->value.number += arg->value.number;
      }
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
      for(vp = vp->next; vp; vp = vp->next) {
         arg = JLEvaluate(context, vp);
         if(arg) {
            result->value.number -= arg->value.number;
         }
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
      for(vp = vp->next; vp; vp = vp->next) {
         arg = JLEvaluate(context, vp);
         if(arg) {
            result->value.number /= arg->value.number;
         }
      }
   }
   return result;
}

JLValue *ConsFunc(JLContext *context, JLValue *args)
{
   JLValue *value = JLEvaluate(context, args->next);
   if(value) {
      JLValue *result = (JLValue*)malloc(sizeof(JLValue));
      JLValue *head = (JLValue*)malloc(sizeof(JLValue));
      JLValue *rest = JLEvaluate(context, args->next->next);

      head->value = value->value;
      head->tag   = value->tag;
      if(rest && rest->tag == JLVALUE_LIST) {
         head->next = rest->value.lst;
      } else {
         head->next = NULL;
      }

      result->tag = JLVALUE_LIST;
      result->next = NULL;
      result->value.lst = head;

      return result;
   }
   return NULL;
}

JLValue *DefineFunc(JLContext *context, JLValue *args)
{
   ScopeNode *scope;
   JLValue *vp = args->next;
   JLValue *result = NULL;
   for(scope = context->scope; scope->next; scope = scope->next);
   if(vp && vp->tag == JLVALUE_STRING) {
      BindingNode *binding = (BindingNode*)malloc(sizeof(BindingNode));
      binding->next = scope->bindings;
      scope->bindings = binding;
      result = JLEvaluate(context, vp->next);
      binding->value = result;
      binding->name = (char*)malloc(strlen(vp->value.str) + 1);
      strcpy(binding->name, vp->value.str);
   }
   return result;
}

JLValue *HeadFunc(JLContext *context, JLValue *args)
{
   JLValue *vp = JLEvaluate(context, args->next);
   if(vp && vp->tag == JLVALUE_LIST) {
      return vp->value.lst;
   } else {
      return NULL;
   }
}

JLValue *IfFunc(JLContext *context, JLValue *args)
{
   JLValue *vp = args->next;
   if(vp) {
      JLValue *cond = JLEvaluate(context, vp);
      if(cond && (cond->tag != JLVALUE_NUMBER || cond->value.number != 0.0)) {
         /* true */
         return JLEvaluate(context, vp->next);
      } else if(vp->next) {
         /* false */
         return JLEvaluate(context, vp->next->next);
      }
   }
   return args;
}

JLValue *LambdaFunc(JLContext *context, JLValue *value)
{
   JLValue *result = CreateValue(context, NULL, JLVALUE_LAMBDA);
   result->value.lst = value->next;
   return result;
}

JLValue *LetFunc(JLContext *context, JLValue *args)
{
   JLValue *vp = args->next;
   JLValue *name;
   JLValue *value;
   JLValue *result;
   if(!vp ||
      vp->tag != JLVALUE_LIST ||
      vp->value.lst->tag != JLVALUE_STRING ||
      !vp->value.lst->next) {
      return args;
   }
   name = vp->value.lst;
   value = JLEvaluate(context, vp->value.lst->next);
   EnterScope(context);
   JLDefineValue(context, name->value.str, value);
   result = value;
   for(vp = vp->next; vp; vp = vp->next) {
      result = JLEvaluate(context, vp);
   }
   LeaveScope(context);
   return result;
}

JLValue *ListFunc(JLContext *context, JLValue *args)
{
   JLValue *result = (JLValue*)malloc(sizeof(JLValue));
   result->tag = JLVALUE_LIST;
   result->value.lst = args->next;
   return result;
}

JLValue *RestFunc(JLContext *context, JLValue *args)
{
   JLValue *vp = JLEvaluate(context, args->next);
   if(vp && vp->tag == JLVALUE_LIST && vp->value.lst) {
      JLValue *result = (JLValue*)malloc(sizeof(JLValue));
      result->tag = JLVALUE_LIST;
      result->next = NULL;
      result->value.lst = vp->value.lst->next;
      return result;
   } else {
      return NULL;
   }
}

JLValue *CreateValue(JLContext *context,
                     const char *name,
                     JLValueType tag)
{
   JLValue *result = (JLValue*)malloc(sizeof(JLValue));
   result->tag = tag;
   result->next = NULL;
   JLDefineValue(context, name, result);
   return result;
}

void DestroyValue(JLValue *value)
{
return;
   switch(value->tag) {
   case JLVALUE_LIST:
      DestroyValue(value->value.lst);
      break;
   case JLVALUE_STRING:
      free(value->value.str);
      break;
   case JLVALUE_LAMBDA:
      DestroyValue(value->value.lst);
      break;
   default:
      break;
   }
   free(value);
}

void EnterScope(JLContext *context)
{
   ScopeNode *scope = (ScopeNode*)malloc(sizeof(ScopeNode));
   scope->bindings = NULL;
   scope->next = context->scope;
   context->scope = scope;
}

void LeaveScope(JLContext *context)
{
   if(context->scope) {
      ScopeNode *next_scope = context->scope->next;
      while(context->scope->bindings) {
         BindingNode *next = context->scope->bindings->next;
         if(context->scope->bindings->name) {
            free(context->scope->bindings->name);
         }
         DestroyValue(context->scope->bindings->value);
         context->scope->bindings = next;
      }
      free(context->scope);
      context->scope = next_scope;
   }
}

JLContext *JLCreateContext()
{
   JLContext *context = (JLContext*)malloc(sizeof(JLContext));
   context->scope = NULL;
   context->line = 1;
   EnterScope(context);
   return context;
}

void JLDestroyContext(JLContext *context)
{
   while(context->scope) {
      LeaveScope(context);
   }
}

void JLDefineValue(JLContext *context, const char *name, JLValue *value)
{
   BindingNode *binding = (BindingNode*)malloc(sizeof(BindingNode));
   binding->next = context->scope->bindings;
   context->scope->bindings = binding;
   binding->value = value;
   if(name) {
      binding->name = (char*)malloc(strlen(name) + 1);
      strcpy(binding->name, name);
   } else {
      binding->name = NULL;
   }
}

JLValue *JLDefineSpecial(JLContext *context,
                         const char *name,
                         JLFunction func)
{
   JLValue *result = CreateValue(context, name, JLVALUE_SPECIAL);
   result->value.special = func;
   return result;
}

JLValue *JLDefineNumber(JLContext *context,
                        const char *name,
                        float value)
{
   JLValue *result = CreateValue(context, name, JLVALUE_NUMBER);
   result->value.number = value;
   return result;
}

JLValue *Lookup(JLContext *context, const char *name)
{
   ScopeNode *scope = context->scope;
   while(scope) {
      BindingNode *binding = scope->bindings;
      while(binding) {
         if(binding->name && !strcmp(binding->name, name)) {
            return binding->value;
         }
         binding = binding->next;
      }
      scope = scope->next;
   }
   return NULL;
}

JLValue *JLEvaluate(JLContext *context, JLValue *value)
{
   JLValue *result = NULL;
   if(value == NULL) {
      return NULL;
   } else if(value->tag == JLVALUE_LIST &&
      value->value.lst &&
      value->value.lst->tag == JLVALUE_STRING) {
      JLValue *temp = Lookup(context, value->value.lst->value.str);
      if(temp) {
         switch(temp->tag) {
         case JLVALUE_SPECIAL:
            result = (temp->value.special)(context, value->value.lst);
            break;
         case JLVALUE_LAMBDA:
            result = EvalLambda(context, temp, value->value.lst);
            break;
         default:
            result = JLEvaluate(context, temp);
            break;
         }
      } else {
         int i;
         result = value;
         for(i = 0; i < INTERNAL_FUNCTION_COUNT; i++) {
            if(!strcmp(INTERNAL_FUNCTIONS[i].name,
                       value->value.lst->value.str)) {
               result = (INTERNAL_FUNCTIONS[i].function)(context,
                                                         value->value.lst);
               break;
            }
         }
      }

   } else if(value->tag == JLVALUE_STRING) {
      JLValue *temp = Lookup(context, value->value.str);
      if(temp) {
         result = temp;
      } else {
         result = value;
      }
   } else {
      result = value;
   }
   return result;
}

JLValue *JLParse(JLContext *context, const char *line)
{
   const char **temp = &line;
   return Parse(context, temp);
}

JLValue *EvalLambda(JLContext *context, const JLValue *lambda, JLValue *args)
{

   JLValue *params;
   JLValue *code;
   JLValue *bp;
   JLValue *ap;
   JLValue *result;

   /* The value of a lambda is a list.  The first element of the
    * list is a set of positional argument bindings.  The remaining
    * elements repesent the code to execute with these bindings in place.
    * args should be a list of arguments that is the same length as
    * the number of parameters to the lambda. */

   /* Make sure the lambda is well-defined. */
   if(lambda->value.lst == NULL ||
      lambda->value.lst->tag != JLVALUE_LIST) {
      ParseError(context, "invalid lambda");
      return NULL;
   }
   params = lambda->value.lst->value.lst;
   code = lambda->value.lst->next;

   /* Insert bindings. */
   EnterScope(context);
   bp = params;
   ap = args->next;  /* Skip the name */
   while(bp) {
      if(ap == NULL) {
         ParseError(context, "too few arguments");
         LeaveScope(context);
         return NULL;
      }
      if(bp->tag != JLVALUE_STRING) {
         ParseError(context, "invalid lambda argument");
         LeaveScope(context);
         return NULL;
      }
      JLDefineValue(context, bp->value.str, JLEvaluate(context, ap));
      bp = bp->next;
      ap = ap->next;
   }
   if(ap) {
      ParseError(context, "too many arguments");
      LeaveScope(context);
      return NULL;
   }

   result = JLEvaluate(context, code);

   LeaveScope(context);

   return result;

}

JLValue *Parse(JLContext *context, const char **line)
{

   JLValue *result;
   JLValue **item;
   const char *start;
   char *temp;

   result = (JLValue*)malloc(sizeof(JLValue));
   result->tag = JLVALUE_LIST;
   result->next = NULL;
   result->value.lst = NULL;
   item = &result->value.lst;

   if(**line == 0) {
      return result;
   }
   if(**line != '(') {
      ParseError(context, "expected '('");
      return result;
   }
   *line += 1;

   for(;;) {

      /* Skip white space. */
      while(IsSpace(**line)) {
         if(**line == '\n') {
            context->line += 1;
         }
         *line += 1;
      }

      /* Exit if we hit the end. */
      if(**line == 0) {
         ParseError(context, "expected ')'");
      }
      if(**line == ')') {
         return result;
      }
      switch(**line) {
      case '(':
         *item = Parse(context, line);
         if(*item) {
            item = &(*item)->next;
         }
         if(**line != ')') {
            ParseError(context, "expected ')'");
            return result;
         } else {
            *line += 1;
         }
         break;
      case '\"':
         break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         *item = (JLValue*)malloc(sizeof(JLValue));
         (*item)->tag = JLVALUE_NUMBER;
         (*item)->value.number = strtof(*line, &temp);
         *line = temp;
         item = &(*item)->next;
         break;
      default:
         start = *line;
         temp = NULL;
         while(IsSymbol(**line)) {
            if(**line == '\n') {
               context->line += 1;
            }
            *line += 1;
         }
         if(*line - start > 0) {
            temp = malloc(*line - start + 1);
            memcpy(temp, start, *line - start);
            temp[*line - start + 1] = 0;
         }
         *item = (JLValue*)malloc(sizeof(JLValue));
         (*item)->tag = JLVALUE_STRING;
         (*item)->value.str = temp;
         item = &(*item)->next;
      }
   }

   if(**line != ')') {
      ParseError(context, "expected ')'");
      return result;
   }

   return result;

}

void ParseError(JLContext *context, const char *msg)
{
   printf("ERROR[%d]: %s\n", context->line, msg);
}

char IsSymbol(char ch)
{
   switch(ch) {
   case '\t':
   case '\n':
   case '\r':
   case '(':
   case ')':
   case ' ':
   case 0:
      return 0;
   default:
      return 1;
   }
}

char IsSpace(char ch)
{
   switch(ch) {
   case ' ':
   case '\t':
   case '\r':
   case '\n':
      return 1;
   default:
      return 0;
   }
}

void JLPrint(const JLContext *context, const JLValue *value)
{
   JLValue *temp;
   if(value == NULL) {
      printf("nil");
      return;
   }
   switch(value->tag) {
   case JLVALUE_NUMBER:
      printf("%g", value->value.number);
      break;
   case JLVALUE_STRING:
      printf("\"%s\"", value->value.str);
      break;
   case JLVALUE_LIST:
      printf("(");
      for(temp = value->value.lst; temp; temp = temp->next) {
         JLPrint(context, temp);
         if(temp->next) {
            printf(" ");
         }
      }
      printf(")");
      break;
   case JLVALUE_LAMBDA:
      printf("(lambda ");
      for(temp = value->value.lst; temp; temp = temp->next) {
         JLPrint(context, temp);
         if(temp->next) {
            printf(" ");
         }
      }
      printf(")");
      break;
   case JLVALUE_SPECIAL:
      printf("@%p", value->value.special);
      break;
   default:
      printf("\n?\n");
      break;
   }
}

