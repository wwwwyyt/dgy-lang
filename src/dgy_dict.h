#ifndef _dgy_dict_h
#define _dgy_dict_h

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dgy_type.h"
#include "dgy_error.h"

typedef struct
{
        wchar_t *name;
        i32 entry;
        i32 level;
        i32 length;
} DictItem;

typedef struct
{
        DictItem *dict;
        i32 top;
        size_t size;
} DgyDict;

/** @brief 词典初始化 */
ErrCode dgyDictInit(DgyDict *dict, size_t size);

/** @brief 新增词典项目 */
ErrCode dgyDictAdd(DgyDict *dict, DictItem *item);

/** @brief 弹出最新加入的词典项 */
ErrCode dgyDictForgetLatest(DgyDict *dict);

/** @brief 获取最新加入的词典项 */
ErrCode dgyDictGetLatest(DgyDict *dict, DictItem *item);

/** @brief 判断词典是否为空 */
bool dgyDictIsEmpty(DgyDict *dict);

i32 dgyDictSearchIn(const DgyDict *dict, const wchar_t *name, i32 parentEntry);
i32 dgyDictSearchEx(const DgyDict *dict, const wchar_t *name, i32 parentEntry);

/** @brief 销毁词典 */
ErrCode dgyDictDestroy(DgyDict *dict);

/** @brief 打印所有词典项，调试用 */
ErrCode dgyDictDump(const DgyDict *dict);

#endif /* _dgy_dict_h */
