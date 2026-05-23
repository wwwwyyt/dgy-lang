#ifndef _dm_object_h
#define _dm_object_h

#include "dm_all.h"

void dmAdd(cell_t *dstack, int bp, int *sp);       /* 相加 */
void dmInc(cell_t *dstack, int bp, int *sp);       /* 加一 */
void dmSub(cell_t *dstack, int bp, int *sp);       /* 相减 */
void dmDec(cell_t *dstack, int bp, int *sp);       /* 减一 */
void dmMul(cell_t *stack, int bp, int *sp);        /* 相乘 */
void dmDiv(cell_t *stack, int bp, int *sp);        /* 相除 */
void dmGetFormat(cell_t *stack, int bp, int *sp);  /* 格式的 */
void dmPrintf(cell_t *stack, int bp, int *sp);     /* 打印 */
void dmPrintStack(cell_t *stack, int bp, int *sp); /* 打印栈 */

#endif /* _dm_object_h */