/**
 * @file jl.c
 * @author Joe Wingbermuehle
 */

#include "jl.h"
#include "jl-context.h"
#include "jl-value.h"
#include "jl-scope.h"
#include "jl-func.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/** Maximum number of evaluations to allow outstanding. */
#define MAX_EVAL_LEVELS    (1 << 15)

static JLValue *EvalLambda(JLContext *context,
                           const JLValue *lambda,
                           JLValue *args);
static JLValue *ParseLiteral(JLContext *context, const char **line);
static void ParseError(JLContext *context, const char *msg, ...);

void JLRetain(JLValue *value)
{
   if(value) {
      value->count += 1;
   }
}

void JLRelease(JLContext *context, JLValue *value)
{
   while(value) {
      value->count -= 1;
      if(value->count == 0) {
         JLValue *next = value->next;
         switch(value->tag) {
         case JLVALUE_LIST:
         case JLVALUE_LAMBDA:
            JLRelease(context, value->value.lst);
            break;
         case JLVALUE_STRING:
            free(value->value.str);
            break;
         case JLVALUE_SCOPE:
            ReleaseScope(context, (ScopeNode*)value->value.scope);
            break;
         default:
            break;
         }
         PutFree(context, value);
         value = next;
      } else {
         break;
      }
   }
}

JLContext *JLCreateContext()
{
   JLContext *context = (JLContext*)malloc(sizeof(JLContext));
   context->scope = NULL;
   context->freelist = NULL;
   context->blocks = NULL;
   context->line = 1;
   context->levels = 0;
   EnterScope(context);
   RegisterFunctions(context);
   JLDefineValue(context, "nil", NULL);
   return context;
}

void JLDestroyContext(JLContext *context)
{
   LeaveScope(context);
   FreeContext(context);
}

void JLDefineValue(JLContext *context, const char *name, JLValue *value)
{
   if(name) {
      BindingNode **root = &context->scope->bindings;
      JLRetain(value);
      while(*root) {
         const int v = strcmp((*root)->name, name);
         if(v < 0) {
            root = &(*root)->left;
         } else if(v > 0) {
            root = &(*root)->right;
         } else {
            /* Overwrite the old binding. */
            JLRelease(context, (*root)->value);
            (*root)->value = value;
            return;
         }
      }

      /* New binding. */
      *root = (BindingNode*)GetFree(context);
      (*root)->name = strdup(name);
      (*root)->value = value;
      (*root)->left = NULL;
      (*root)->right = NULL;

   }
}

void JLDefineSpecial(JLContext *context,
                     const char *name,
                     JLFunction func)
{
   JLValue *result = CreateValue(context, name, JLVALUE_SPECIAL);
   result->value.special = func;
   JLRelease(context, result);
}

JLValue *JLDefineNumber(JLContext *context,
                        const char *name,
                        double value)
{
   JLValue *result = CreateValue(context, name, JLVALUE_NUMBER);
   result->value.number = value;
   return result;
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
             value->value.lst->tag == JLVALUE_VARIABLE) {
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
      }
   } else if(value->tag == JLVALUE_VARIABLE) {
      result = Lookup(context, value->value.str);
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
      if(bp->tag != JLVALUE_VARIABLE) {
         ParseError(context, "invalid lambda argument");
         result = NULL;
         goto done_eval_lambda;
      }
      context->scope = old_scope;
      result = JLEvaluate(context, ap);
      context->scope = new_scope;
      JLDefineValue(context, bp->value.str, result);
      JLRelease(context, result);
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
         JLRelease(context, result);
      }
   }

done_eval_lambda:

   LeaveScope(context);
   context->scope = old_scope;

   return result;

}

JLValue *ParseLiteral(JLContext *context, const char **line)
{

   /* Separators include '(', ')', and white-space.
    * If if token starts with '"', we treat it as a string with escape
    * characters like in C.
    * Otherwise, if a token can be parsed as a number, we treat it as such.
    * Everything else we treat as a string.
    * Note that function lookups happen later, here we only generate
    * strings and floating-point numbers.
    */

   JLValue *result = CreateValue(context, NULL, JLVALUE_INVALID);

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

      /* Attempt to parse the token as a double. */
      result->value.number = strtod(start, &end);

      /* If we couldn't parse the whole thing, treat it as a variable. */
      if(start + len != end) {
         result->tag = JLVALUE_VARIABLE;
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
      *line += 1;
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
         JLRelease(context, result);
         return NULL;
      case ')':
         *line += 1;
         return result;
      case '(':
         *item = JLParse(context, line);
         if(*item) {
            item = &(*item)->next;
         } else {
            JLRelease(context, result);
            return NULL;
         }
         break;
      default:
         *item = ParseLiteral(context, line);
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

char JLIsNumber(JLValue *value)
{
   if(value && value->tag == JLVALUE_NUMBER) {
      return 1;
   } else {
      return 0;
   }
}

double JLGetNumber(JLValue *value)
{
   return value->value.number;
}

char JLIsString(JLValue *value)
{
   if(value && value->tag == JLVALUE_STRING) {
      return 1;
   } else {
      return 0;
   }
}

const char *JLGetString(JLValue *value)
{
   return value->value.str;
}

char JLIsList(JLValue *value)
{
   if(value && value->tag == JLVALUE_LIST) {
      return 1;
   } else {
      return 0;
   }
}

JLValue *JLGetHead(JLValue *value)
{
   return value->value.lst;
}

JLValue *JLGetNext(JLValue *value)
{
   return value->next;
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
   case JLVALUE_VARIABLE:
      printf("%s", value->value.str);
      break;
   default:
      printf("\n?\n");
      break;
   }
}

