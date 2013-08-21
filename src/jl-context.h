/**
 * @file jl-context.h
 * @author Joe Wingbermuehle
 */

#ifndef JL_CONTEXT_H
#define JL_CONTEXT_H

struct ScopeNode;
struct FreeNode;
struct BlockNode;

typedef struct JLContext {
   struct ScopeNode *scope;
   struct FreeNode *freelist;
   struct BlockNode *blocks;
   unsigned int line;
   unsigned int levels;
   unsigned int max_levels;
   char error;
} JLContext;

void *GetFree(JLContext *context);

void PutFree(JLContext *context, void *value);

void FreeContext(JLContext *context);

#endif /* JL_CONTEXT_H */
