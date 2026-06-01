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

ErrCode dgyDictInit(Dictionary *dict, size_t size);
ErrCode dgyDictAdd(const wchar_t *name, int entry, Dictionary *dict);
int dgyDictSearch(const wchar_t *name, Dictionary *dict);
ErrCode dgyDictForget(const wchar_t *name, Dictionary *dict);

#endif /* _dgy_dict_h */
