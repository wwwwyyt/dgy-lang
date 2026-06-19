#ifndef _dgy_dict_h
#define _dgy_dict_h

#include "dgy_all.h"
#include "dgy_error.h"

typedef struct
{
        wchar_t *name;
        i32 entry;
} DictItem;

typedef struct
{
        DictItem *dict;
        i32 top;
        size_t size;
} DgyDict;

ErrCode dgyDictInit(DgyDict *dict, size_t size);
ErrCode dgyDictAdd(DgyDict *dict, const wchar_t *name, i32 entry);
i32 dgyDictSearch(const DgyDict *dict, const wchar_t *name);
ErrCode dgyDictForget(DgyDict *dict, const wchar_t *name);
ErrCode dgyDictDestroy(DgyDict *dict);
ErrCode dgyDictDump(const DgyDict *dict);

#endif /* _dgy_dict_h */
