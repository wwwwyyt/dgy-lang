#ifndef _dgy_stack_h
#define _dgy_stack_h

#include "dgy_all.h"
#include "dgy_error.h"

typedef struct
{
        cell_t *stack;          /* Shouldn't be directly modified */
        i32 sp;                 /* Shouldn't be directly modified */
        size_t size;            /* Shouldn't be directly modified */
} DgyStack;

ErrCode dgyStackInit(DgyStack *s, size_t size);
ErrCode dgyStackResize(DgyStack *s, size_t newSize);
ErrCode dgyStackDestroy(DgyStack *s);
ErrCode dgyStackPop(DgyStack *s);
ErrCode dgyStackPush(DgyStack *s, cell_t data);
ErrCode dgyStackClear(DgyStack *s);

/* They are read-only, so add "const". */
ErrCode dgyStackItemAt(const DgyStack *s, i32 idx, cell_t *data);
ErrCode dgyStackTop(const DgyStack *s, cell_t *data);
bool dgyStackIsEmpty(const DgyStack *s);
ErrCode dgyStackDump(const DgyStack *s, i32 start, i32 end);

#endif /* _dgy_stack_h */
