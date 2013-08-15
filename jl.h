/**
 * @file jl.h
 * @author Joe Wingbermuehle
 */

#ifndef JL_H
#define JL_H

struct JLValue;
struct JLContext;

typedef struct JLValue *(*JLFunction)(struct JLContext *context,
                                      struct JLValue *arglist);

typedef char JLValueType;
#define JLVALUE_INVALID    0     /**< Invalid value. */
#define JLVALUE_NUMBER     1     /**< Literal number. */
#define JLVALUE_STRING     2     /**< Literal string. */
#define JLVALUE_LIST       3     /**< Linked list. */
#define JLVALUE_LAMBDA     4     /**< Lambda function. */
#define JLVALUE_SPECIAL    5     /**< Special form. */

typedef struct JLValue {
   union {
      struct JLValue *lst;
      JLFunction special;
      char *str;
      float number;
   } value;
   struct JLValue *next;
   JLValueType tag;
} JLValue;

struct JLContext *JLCreateContext();

void JLDestroyContext(struct JLContext *context);

void JLDefineValue(struct JLContext *context,
                   const char *name,
                   JLValue *value);

JLValue *JLDefineSpecial(struct JLContext *context,
                         const char *name,
                         JLFunction func);

JLValue *JLDefineNumber(struct JLContext *context,
                        const char *name,
                        float value);

JLValue *JLParse(struct JLContext *context, const char *line);

JLValue *JLEvaluate(struct JLContext *context, JLValue *value);

void JLPrint(const struct JLContext *context, const JLValue *value);

#endif
