/**
 * @file jl.h
 * @author Joe Wingbermuehle
 *
 * Header file for functions to interface with the JL interpreter.
 *
 */

#ifndef JL_H
#define JL_H

struct JLValue;
struct JLContext;

/** The type of special functions.
 * @param context The JL context.
 * @param args A list of arguments to the function, including its name.
 * @return The result, which should be retained (it will be freed if
 *         not needed).
 */
typedef struct JLValue *(*JLFunction)(struct JLContext *context,
                                      struct JLValue *args);

/** Possible value types. */
typedef char JLValueType;
#define JLVALUE_INVALID    0     /**< Invalid value. */
#define JLVALUE_NUMBER     1     /**< Literal number. */
#define JLVALUE_STRING     2     /**< Literal string. */
#define JLVALUE_LIST       3     /**< Linked list. */
#define JLVALUE_LAMBDA     4     /**< Lambda function. */
#define JLVALUE_SPECIAL    5     /**< Special form. */

/** Values in the JL environment.
 * Note that these are reference counted.
 */
typedef struct JLValue {
   union {
      struct JLValue *lst;
      JLFunction special;
      char *str;
      float number;
   } value;
   struct JLValue *next;
   unsigned int count;
   JLValueType tag;
} JLValue;

/** Create a context for running JL programs.
 * @return The context.
 */
struct JLContext *JLCreateContext();

/** Destroy a JL context.
 * @param context The context to be destroyed.
 */
void JLDestroyContext(struct JLContext *context);

/** Define a value.
 * This will add a binding to the current scope.
 * @param context The context in which to define the value.
 * @param name The name of the binding.
 * @param value The value to insert.
 */
void JLDefineValue(struct JLContext *context,
                   const char *name,
                   JLValue *value);

/** Define a special function.
 * A special function is simply a function that is implemented outside
 * of the JL environment.
 * This will add the special function to the current scope.
 * @param context The context in which to define the function.
 * @param name The name of the function.
 * @param func The function code.
 * @return The function value.  This must be retained if used.
 */
JLValue *JLDefineSpecial(struct JLContext *context,
                         const char *name,
                         JLFunction func);

/** Define a number.
 * This will add a number to the current scope.
 * @param context The context in which to define the number.
 * @param name The name of the binding.
 * @param value The value to define.
 * @return The value.  This must be retained if used.
 */
JLValue *JLDefineNumber(struct JLContext *context,
                        const char *name,
                        float value);

/** Parse an expression.
 * Note that only a single expression is parsed.
 * @param context The context.
 * @param line The expression to be parsed.
 * @return The expression.  This value is retained and must be destroyed.
 */
JLValue *JLParse(struct JLContext *context, const char **line);

/** Evaluate an expression.
 */
JLValue *JLEvaluate(struct JLContext *context, JLValue *value);

/** Display a value.
 * @param context The context.
 * @param value The value to display.
 */
void JLPrint(const struct JLContext *context, const JLValue *value);

#endif /* JL_H */
