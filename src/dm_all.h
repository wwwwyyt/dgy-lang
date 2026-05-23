#ifndef _dm_all_h
#define _dm_all_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <errno.h>
#include <wctype.h>

typedef enum
{
    CODE_SUCCESS = EXIT_SUCCESS,
    CODE_FAILURE = EXIT_FAILURE,
    CODE_OVERFLOW,
    CODE_UNDERFLOW,
    CODE_ALLOC_FAIL,
    CODE_FILE_OPEN_FAIL,
} ErrCode;

typedef unsigned long long cell_t;

#endif /* _dm_all_h */