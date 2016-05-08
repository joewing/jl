/**
 * @file jl-value.h
 * @author Joe Wingbermuehle
 */

#ifndef JL_VALUE_H
#define JL_VALUE_H

#include "jl.h"

/** Possible value types. */
typedef char JLValueType;
#define JLVALUE_NIL        0     /**< Nil. */
#define JLVALUE_NUMBER     1     /**< Literal number. */
#define JLVALUE_STRING     2     /**< Literal string. */
#define JLVALUE_LIST       3     /**< Linked list. */
#define JLVALUE_LAMBDA     4     /**< Lambda function. */
#define JLVALUE_SPECIAL    5     /**< Special form. */
#define JLVALUE_SCOPE      6     /**< A scope (internal use). */
#define JLVALUE_VARIABLE   7     /**< A variable. */

/** Special function and extra parameter. */
typedef struct SpecialFunction {
   JLFunction func;
   void *extra;
} SpecialFunction;

/** Values in the JL environment.
 * Note that these are reference counted.
 */
typedef struct JLValue {
   union {
      struct JLValue *lst;
      SpecialFunction special;
      char *str;
      double number;
      void *scope;
   } value;
   struct JLValue *next;
   unsigned int count;
   JLValueType tag;
} JLValue;

JLValue *CreateValue(struct JLContext *context,
                     const char *name, JLValueType tag);

JLValue *CopyValue(struct JLContext *context, const JLValue *other);

#endif /* JL_VALUE_H */
