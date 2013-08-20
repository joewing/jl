/**
 * @file jl-scope.h
 * @author Joe Wingbermuehle
 */

#ifndef JL_SCOPE_H
#define JL_SCOPE_H

struct JLContext;
struct JLValue;

typedef struct BindingNode {
   char *name;
   struct JLValue *value;
   struct BindingNode *left;
   struct BindingNode *right;
} BindingNode;

typedef struct ScopeNode {
   BindingNode *bindings;
   struct ScopeNode *next;
   unsigned int count;
} ScopeNode;

void EnterScope(struct JLContext *context);

void LeaveScope(struct JLContext *context);

void ReleaseScope(struct JLContext *context, ScopeNode *scope);

struct JLValue *Lookup(struct JLContext *context, const char *name);

#endif /* JL_SCOPE_H */
