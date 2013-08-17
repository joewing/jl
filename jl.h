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
#define JLVALUE_SCOPE      6     /**< A scope (internal use). */

/** Values in the JL environment.
 * Note that these are reference counted.
 */
typedef struct JLValue {
   union {
      struct JLValue *lst;
      JLFunction special;
      char *str;
      float number;
      void *scope;
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

/** Increase the reference count of a value. */
#define JLRetain( v ) do { if(v) (v)->count += 1; } while(0)

/** Decrease the reference count of a value.
 * This will destroy the value if the reference count reaches zero.
 * It is safe to pass NULL to this function.
 */
void JLRelease(JLValue *value);

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
 */
void JLDefineSpecial(struct JLContext *context,
                     const char *name,
                     JLFunction func);

/** Define a number.
 * This will add a number to the current scope.
 * @param context The context in which to define the number.
 * @param name The name of the binding.
 * @param value The value to define.
 * @return The value.  This value must be released if not used.
 */
JLValue *JLDefineNumber(struct JLContext *context,
                        const char *name,
                        float value);

/** Parse an expression.
 * Note that only a single expression is parsed.
 * @param context The context.
 * @param line The expression to be parsed.
 * @return The expression.  This value must be released if not used.
 */
JLValue *JLParse(struct JLContext *context, const char **line);

/** Evaluate an expression.
 * @param context The context.
 * @param value The expression to evaluate.
 * @return The result.  This value must be released if not used.
 */
JLValue *JLEvaluate(struct JLContext *context, JLValue *value);

/** Display a value.
 * @param context The context.
 * @param value The value to display.
 */
void JLPrint(const struct JLContext *context, const JLValue *value);

#endif /* JL_H */
