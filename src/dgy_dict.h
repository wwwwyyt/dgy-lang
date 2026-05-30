#ifndef _dgy_dict_h
#define _dgy_dict_h

#include "dgy_all.h"
#include "dgy_object.h"

typedef struct
{
        wchar_t *name;
        int entry;
} DictItem;

typedef struct
{
        DictItem *dict;
        int top;
        size_t size;
} Dictionary;

#endif /* _dgy_dict_h */