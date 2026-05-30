#ifndef _dgy_object_h
#define _dgy_object_h

#include "dgy_all.h"

/* Operation code */
typedef enum
{
        ADD,
        INC,
        SUB,
        DEC,
        MUL,
        DIV,
        AND,
        OR,
        NOT,
        NOR,
} OperationCode;

/* Basic arithmetic and logical operations */
void dgyAdd(cell_t *dstack, int bp, int *sp); /* 相加 */
void dgyInc(cell_t *dstack, int bp, int *sp); /* 加一 */
void dgySub(cell_t *dstack, int bp, int *sp); /* 相减 */
void dgyDec(cell_t *dstack, int bp, int *sp); /* 减一 */
void dgyMul(cell_t *stack, int bp, int *sp);  /* 相乘 */
void dgyDiv(cell_t *stack, int bp, int *sp);  /* 相除 */
void dgyAnd(cell_t *stack, int bp, int *sp);  /* 按位与 */
void dgyOr(cell_t *stack, int bp, int *sp);   /* 按位或 */
void dgyNot(cell_t *stack, int bp, int *sp);  /* 按位取反 */
void dgyNor(cell_t *stack, int bp, int *sp);  /* 按位异或 */

/* Debug operations */
void dgyGetFormat(cell_t *stack, int bp, int *sp);  /* 格式的 */
void dgyPrintf(cell_t *stack, int bp, int *sp);     /* 打印 */
void dgyPrintStack(cell_t *stack, int bp, int *sp); /* 打印栈 */

#endif /* _dgy_object_h */