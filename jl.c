/**
 * @file jl.c
 * @author Joe Wingbermuehle
 */

#include "jl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/** Maximum number of evaluations to allow outstanding. */
#define MAX_EVAL_LEVELS    1024

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
   unsigned int count;
} ScopeNode;

typedef struct JLContext {
   ScopeNode *scope;
   int line;
   int levels;
} JLContext;

static JLValue *CreateValue(JLContext *context,
                            const char *name,
                            JLValueType tag);
static JLValue *CopyValue(JLContext *context, const JLValue *other);
static void ReleaseScope(ScopeNode *scope);
static void EnterScope(JLContext *context);
static void LeaveScope(JLContext *context);
static JLValue *Lookup(JLContext *context, const char *name);
static JLValue *EvalLambda(JLContext *context,
                           const JLValue *lambda,
                           JLValue *args);
static JLValue *ParseLiteral(const char **line);
static void ParseError(JLContext *context, const char *msg, ...);

static JLValue *CompareFunc(JLContext *context, JLValue *value);
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
   { "=",         CompareFunc },
   { "!=",        CompareFunc },
   { ">",         CompareFunc },
   { ">=",        CompareFunc },
   { "<",         CompareFunc },
   { "<=",        CompareFunc },
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

JLValue *CompareFunc(JLContext *context, JLValue *args)
{
   JLValue *result = NULL;
   JLValue *va = JLEvaluate(context, args->next);
   if(va && va->tag == JLVALUE_NUMBER) {
      JLValue *vb = JLEvaluate(context, args->next->next);
      if(vb && vb->tag == JLVALUE_NUMBER) {
         const float a = va->value.number;
         const float b = vb->value.number;
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
      JLRelease(vb);
   }
   JLRelease(va);
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
      JLRelease(arg);
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
      JLRelease(arg);
      for(vp = vp->next; vp; vp = vp->next) {
         arg = JLEvaluate(context, vp);
         if(arg && arg->tag == JLVALUE_NUMBER) {
            result->value.number -= arg->value.number;
         }
         JLRelease(arg);
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
      JLRelease(arg);
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
      JLRelease(arg);
      for(vp = vp->next; vp; vp = vp->next) {
         arg = JLEvaluate(context, vp);
         if(arg) {
            result->value.number /= arg->value.number;
         }
         JLRelease(arg);
      }
   }
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
      JLRelease(rest);
      JLRelease(value);
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
      result = JLEvaluate(context, vp->next);
      JLRetain(result);
      binding->next = scope->bindings;
      scope->bindings = binding;
      binding->value = result;
      binding->name = strdup(vp->value.str);
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
   JLRelease(vp);
   return result;
}

JLValue *IfFunc(JLContext *context, JLValue *args)
{
   JLValue *vp = args->next;
   if(vp) {
      JLValue *cond = JLEvaluate(context, vp);
      char is_true = 0;
      if(cond) {
         switch(cond->tag) {
         case JLVALUE_NUMBER:
            is_true = cond->value.number != 0.0;
            break;
         case JLVALUE_LIST:
            is_true = cond->value.lst != NULL;
            break;
         default:
            is_true = 1;
            break;
         }
         JLRelease(cond);
      }
      if(is_true) {
         return JLEvaluate(context, vp->next);
      } else if(vp->next) {
         return JLEvaluate(context, vp->next->next);
      }
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
      return NULL;
   }
   name = vp->value.lst;
   value = JLEvaluate(context, vp->value.lst->next);

   EnterScope(context);
   JLDefineValue(context, name->value.str, value);
   result = value;
   for(vp = vp->next; vp; vp = vp->next) {
      JLRelease(result);
      result = JLEvaluate(context, vp);
   }
   LeaveScope(context);

   return result;
}

JLValue *ListFunc(JLContext *context, JLValue *args)
{
   JLValue *result = CreateValue(context, NULL, JLVALUE_LIST);
   if(args->next) {
      result->value.lst = args->next;
      JLRetain(args->next);
   } else {
      result->value.lst = NULL;
   }
   return result;
}

JLValue *RestFunc(JLContext *context, JLValue *args)
{
   JLValue *result = NULL;
   JLValue *vp = JLEvaluate(context, args->next);
   if(vp && vp->tag == JLVALUE_LIST) {
      result = CreateValue(context, NULL, JLVALUE_LIST);
      if(vp->value.lst) {
         result->value.lst = vp->value.lst->next;
         JLRetain(result->value.lst);
      } else {
         result->value.lst = NULL;
      }
   }
   JLRelease(vp);
   return result;
}

JLValue *CreateValue(JLContext *context,
                     const char *name,
                     JLValueType tag)
{
   JLValue *result = (JLValue*)malloc(sizeof(JLValue));
   result->tag = tag;
   result->next = NULL;
   result->count = 1;
   JLDefineValue(context, name, result);
   return result;
}

JLValue *CopyValue(JLContext *context, const JLValue *other)
{
   JLValue *result = CreateValue(context, NULL, other->tag);
   result->value = other->value;
   switch(result->tag) {
   case JLVALUE_LIST:
   case JLVALUE_LAMBDA:
   case JLVALUE_SCOPE:
      JLRetain(result->value.lst);
      break;
   case JLVALUE_STRING:
      result->value.str = strdup(result->value.str);
      break;
   default:
      break;
   }
   return result;
}

void ReleaseScope(ScopeNode *scope)
{
   scope->count -= 1;
   if(scope->count == 0) {
      while(scope->bindings) {
         BindingNode *next = scope->bindings->next;
         free(scope->bindings->name);
         JLRelease(scope->bindings->value);
         free(scope->bindings);
         scope->bindings = next;
      }
      free(scope);
   }
}

void JLRelease(JLValue *value)
{
   while(value) {
      value->count -= 1;
      if(value->count == 0) {
         JLValue *next = value->next;
         switch(value->tag) {
         case JLVALUE_LIST:
         case JLVALUE_LAMBDA:
            JLRelease(value->value.lst);
            break;
         case JLVALUE_STRING:
            free(value->value.str);
            break;
         case JLVALUE_SCOPE:
            ReleaseScope((ScopeNode*)value->value.scope);
            break;
         default:
            break;
         }
         free(value);
         value = next;
      } else {
         break;
      }
   }
}

void EnterScope(JLContext *context)
{
   ScopeNode *scope = (ScopeNode*)malloc(sizeof(ScopeNode));
   scope->count = 1;
   scope->bindings = NULL;
   scope->next = context->scope;
   context->scope = scope;
}

void LeaveScope(JLContext *context)
{
   ScopeNode *scope = context->scope;
   context->scope = scope->next;
   ReleaseScope(scope);
}

JLContext *JLCreateContext()
{
   JLContext *context = (JLContext*)malloc(sizeof(JLContext));
   context->scope = NULL;
   context->line = 1;
   context->levels = 0;
   EnterScope(context);
   return context;
}

void JLDestroyContext(JLContext *context)
{
   while(context->scope) {
      LeaveScope(context);
   }
   free(context);
}

void JLDefineValue(JLContext *context, const char *name, JLValue *value)
{
   if(name) {
      BindingNode *binding = (BindingNode*)malloc(sizeof(BindingNode));
      binding->next = context->scope->bindings;
      context->scope->bindings = binding;
      binding->value = value;
      binding->name = strdup(name);
      JLRetain(value);
   }
}

void JLDefineSpecial(JLContext *context,
                     const char *name,
                     JLFunction func)
{
   JLValue *result = CreateValue(context, name, JLVALUE_SPECIAL);
   result->value.special = func;
   JLRelease(result);
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
   const ScopeNode *scope = context->scope;
   while(scope) {
      const BindingNode *binding = scope->bindings;
      while(binding) {
         if(!strcmp(binding->name, name)) {
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
   context->levels += 1;
   if(value == NULL) {
      result = NULL;
   } else if(context->levels > MAX_EVAL_LEVELS) {
      ParseError(context, "maximum evaluation depth exceeded");
      result = NULL;
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
         result = NULL;
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
      JLRetain(result);
   } else {
      result = value;
      JLRetain(result);
   }
   context->levels -= 1;
   return result;
}

JLValue *EvalLambda(JLContext *context, const JLValue *lambda, JLValue *args)
{

   JLValue *scope;
   JLValue *params;
   JLValue *code;
   ScopeNode *old_scope;
   ScopeNode *new_scope;
   JLValue *bp;
   JLValue *ap;
   JLValue *result;

   /* The value of a lambda is a list containing the following:
    *    - The scope in which to execute.
    *    - A list of positional argument bindings.
    *    - The code to execute (all remaining list items).
    * args should be a list of arguments that is the same length as
    * the number of parameters to the lambda. */

   /* Make sure the lambda is well-defined. */
   if(lambda->value.lst == NULL ||
      lambda->value.lst->tag != JLVALUE_SCOPE ||
      lambda->value.lst->next == NULL ||
      lambda->value.lst->next->tag != JLVALUE_LIST) {
      ParseError(context, "invalid lambda");
      return NULL;
   }
   scope = lambda->value.lst;
   params = lambda->value.lst->next->value.lst;
   code = lambda->value.lst->next->next;

   /* Insert bindings. */
   old_scope = context->scope;
   context->scope = (ScopeNode*)scope->value.scope;
   EnterScope(context);
   new_scope = context->scope;
   bp = params;
   ap = args->next;  /* Skip the name */
   while(bp) {
      if(ap == NULL) {
         ParseError(context, "too few arguments");
         result = NULL;
         goto done_eval_lambda;
      }
      if(bp->tag != JLVALUE_STRING) {
         ParseError(context, "invalid lambda argument");
         result = NULL;
         goto done_eval_lambda;
      }
      context->scope = old_scope;
      result = JLEvaluate(context, ap);
      context->scope = new_scope;
      JLDefineValue(context, bp->value.str, result);
      JLRelease(result);
      bp = bp->next;
      ap = ap->next;
   }
   if(ap) {
      ParseError(context, "too many arguments");
      result = NULL;
      goto done_eval_lambda;
   }

   result = NULL;
   while(code) {
      result = JLEvaluate(context, code);
      code = code->next;
      if(code) {
         JLRelease(result);
      }
   }

done_eval_lambda:

   ReleaseScope(new_scope);
   context->scope = old_scope;

   return result;

}

JLValue *ParseLiteral(const char **line)
{

   /* Separators include '(', ')', and white-space.
    * If if token starts with '"', we treat it as a string with escape
    * characters like in C.
    * Otherwise, if a token can be parsed as a number, we treat it as such.
    * Everything else we treat as a string.
    * Note that function lookups happen later, here we only generate
    * strings and floating-point numbers.
    */

   JLValue *result = CreateValue(NULL, NULL, JLVALUE_INVALID);

   if(**line == '\"') {
      size_t max_len = 16;
      size_t len = 0;
      char in_control = 0;
      char in_hex = 0;
      char in_octal = 0;
      result->value.str = (char*)malloc(max_len);
      *line += 1;
      while(**line && (in_control != 0 || **line != '\"')) {
         if(len + 1 >= max_len) {
            max_len += 16;
            result->value.str = (char*)realloc(result->value.str, max_len);
         }
         if(in_hex) {
            /* In a hex control sequence. */
            if(**line >= '0' && **line <= '9') {
               result->value.str[len] *= 16;
               result->value.str[len] += **line - '0';
               in_hex -= 1;
               *line += 1;
            } else if(**line >= 'a' && **line <= 'f') {
               result->value.str[len] *= 16;
               result->value.str[len] += **line - 'a' + 10;
               in_hex -= 1;
               *line += 1;
            } else if(**line >= 'A' && **line <= 'F') {
               result->value.str[len] *= 16;
               result->value.str[len] += **line - 'A' + 10;
               in_hex -= 1;
               *line += 1;
            } else {
               /* Premature end of hex sequence; reparse this character. */
               in_hex = 0;
            }
         } else if(in_octal) {
            /* In an octal control sequence. */
            if(**line >= '0' && **line <= '7') {
               result->value.str[len] *= 8;
               result->value.str[len] += **line - '0';
               in_octal -= 1;
               *line += 1;
            } else {
               /* Premature end of octal sequence; reparse this character. */
               in_octal = 0;
            }
         } else if(in_control) {
            /* In a control sequence. */
            in_control = 0;
            switch(**line) {
            case 'a':   /* bell */
               result->value.str[len++] = '\a';
               break;
            case 'b':   /* backspace */
               result->value.str[len++] = '\b';
               break;
            case 'f':   /* form-feed */
               result->value.str[len++] = '\f';
               break;
            case 'n':   /* new-line */
               result->value.str[len++] = '\n';
               break;
            case 'r':   /* carriage return */
               result->value.str[len++] = '\r';
               break;
            case 't':   /* tab */
               result->value.str[len++] = '\t';
               break;
            case 'v':   /* vertical tab */
               result->value.str[len++] = '\v';
               break;
            case 'x':   /* Hex control sequence. */
               in_hex = 2;
               break;
            case '0':   /* Octal control sequence. */
               in_octal = 3;
               break;
            default:    /* Literal character */
               result->value.str[len++] = **line;
               break;
            }
            *line += 1;
         } else if(**line == '\\') {
            /* Start of a control sequence. */
            in_control = 1;
            *line += 1;
         } else {
            /* Regular character. */
            result->value.str[len] = **line;
            len += 1;
            *line += 1;
         }
      }
      result->value.str[len] = 0;
      result->tag = JLVALUE_STRING;
      if(**line) {
         /* Skip the terminating '"'. */
         *line += 1;
      }
   } else {

      const char *start = *line;
      char *end;
      size_t len = 0;

      /* Determine how long this token is. */
      while(**line != 0 && **line != '(' && **line != ')' &&
            **line != ' ' && **line != '\t' && **line != '\r' &&
            **line != '\n') {
         len += 1;
         *line += 1;
      }

      /* Attempt to parse the token as a float. */
      result->value.number = strtof(start, &end);

      /* If we couldn't parse the whole thing, treat it as a string. */
      if(start + len != end) {
         result->tag = JLVALUE_STRING;
         result->value.str = (char*)malloc(len + 1);
         memcpy(result->value.str, start, len);
         result->value.str[len] = 0;
      } else {
         result->tag = JLVALUE_NUMBER;
      }

   }

   return result;

}

JLValue *JLParse(JLContext *context, const char **line)
{

   JLValue *result;
   JLValue **item;

   /* Skip any leading white-space. */
   for(;;) {
      if(**line == ';') {
         while(**line && **line != '\n') {
            *line += 1;
         }
      } else if(**line == '\n') {
         context->line += 1;
      } else if(  **line != '\t' &&
                  **line != ' ' &&
                  **line != '\r') {
         break;
      }
      *line += 1;
   }

   if(**line == 0) {
      return NULL;
   }
   if(**line != '(') {
      ParseError(context, "expected '('");
      return NULL;
   }
   *line += 1;

   result = CreateValue(context, NULL, JLVALUE_LIST);
   result->value.lst = NULL;
   item = &result->value.lst;

   for(;;) {

      /* Process the next token. */
      switch(**line) {
      case ';':      /* Start of a comment, skip to the next line. */
         while(**line && **line != '\n') {
            *line += 1;
         }
         break;
      case '\n':
         context->line += 1;
         /* Fall through */
      case ' ':
      case '\t':
      case '\r':
         *line += 1;
         break;
      case 0:
         ParseError(context, "expected ')', got end-of-input");
         JLRelease(result);
         return NULL;
      case ')':
         *line += 1;
         return result;
      case '(':
         *item = JLParse(context, line);
         if(*item) {
            item = &(*item)->next;
         } else {
            JLRelease(result);
            return NULL;
         }
         break;
      default:
         *item = ParseLiteral(line);
         item = &(*item)->next;
         break;
      }
   }

}

void ParseError(JLContext *context, const char *msg, ...)
{
   va_list ap;
   va_start(ap, msg);
   printf("ERROR[%d]: ", context->line);
   vprintf(msg, ap);
   printf("\n");
   va_end(ap);
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
      for(temp = value->value.lst->next; temp; temp = temp->next) {
         JLPrint(context, temp);
         if(temp->next) {
            printf(" ");
         }
      }
      printf(")");
      break;
   case JLVALUE_SPECIAL:
      printf("special@%p", value->value.special);
      break;
   default:
      printf("\n?\n");
      break;
   }
}

