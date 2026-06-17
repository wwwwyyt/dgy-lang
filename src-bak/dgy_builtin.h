#ifndef _dgy_builtin_h
#define _dgy_builtin_h

#include "dgy_all.h"
#include "dgy_stack.h"

/* Built-in word code */
typedef enum
{
        W_NOP = 0, /* No operation */
        W_ADD,
        W_INC,
        W_SUB,
        W_DEC,
        W_MUL,
        W_DIV,
        W_AND,
        W_OR,
        W_NOT,
        W_NOR,

        W_FORMAT,
        W_PRINTF,
        
        BUILT_IN_WORD_CNT,
} BuiltInWordCode;

/* Basic arithmetic and logical operations */
void dgyAdd(DgyStack *stack); /* 相加 */
void dgyInc(DgyStack *stack); /* 加一 */
void dgySub(DgyStack *stack); /* 相减 */
void dgyDec(DgyStack *stack); /* 减一 */
void dgyMul(DgyStack *stack);  /* 相乘 */
void dgyDiv(DgyStack *stack);  /* 相除 */
void dgyAnd(DgyStack *stack);  /* 按位与 */
void dgyOr(DgyStack *stack);   /* 按位或 */
void dgyNot(DgyStack *stack);  /* 按位取反 */
void dgyNor(DgyStack *stack);  /* 按位异或 */

/* Debug operations */
void dgyGetFormat(DgyStack *stack);  /* 格式的 */
void dgyPrintf(DgyStack *stack);     /* 打印 */

const wchar_t *dgyGetBuiltInWordName(BuiltInWordCode code);
ErrCode dgyExecBuiltIn(BuiltInWordCode op, DgyStack *stack);

#endif /* _dgy_builtin_h */
