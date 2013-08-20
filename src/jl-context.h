/**
 * @file jl-context.h
 * @author Joe Wingbermuehle
 */

#ifndef JL_CONTEXT_H
#define JL_CONTEXT_H

struct ScopeNode;
struct FreeNode;

typedef struct JLContext {
   struct ScopeNode *scope;
   struct FreeNode *freelist;
   int line;
   int levels;
} JLContext;

void *GetFree(JLContext *context);

void PutFree(JLContext *context, void *value);

void FreeContext(JLContext *context);

#endif /* JL_CONTEXT_H */
