#ifndef _dm_all_h
#define _dm_all_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

typedef enum
{
    CODE_SUCCESS,
    CODE_OVERFLOW,
    CODE_UNDERFLOW,
    CODE_ALLOC_FAIL,
} ErrCode;

typedef unsigned long long cell_t;

#endif /* _dm_all_h */