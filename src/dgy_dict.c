#include "dgy_dict.h"

static Dictionary objectDict;

static ErrCode resize(size_t newSize, Dictionary *dict)
{
        DictItem *newDict = (DictItem *)realloc(dict->dict, newSize * sizeof(DictItem));
        if (newDict == NULL)
        {
                return CODE_ALLOC_FAIL;
        }
        dict->dict = newDict;
        dict->size = newSize;
        return CODE_SUCCESS;
}

ErrCode dgyDictInit()
{
        objectDict.size = 16;
        objectDict.top = 0;
        objectDict.dict = (DictItem *)malloc(objectDict.size * sizeof(DictItem));
        if (objectDict.dict == NULL)
        {
                return CODE_ALLOC_FAIL;
        }
        return CODE_SUCCESS;
}

ErrCode dgyDictAdd(const wchar_t *name, int entry, Dictionary *dict)
{
        size_t nameLen = wcslen(name);
        DictItem newItem;
        newItem.name = (wchar_t *)malloc(nameLen * sizeof(wchar_t));
        wcscpy(newItem.name, name);
        newItem.entry = entry;

        if (dict->top == dict->size / 2 && (CODE_SUCCESS != resize(2 * dict->size, dict)))
        {
                return CODE_FAILURE;
        }
        dict->dict[(dict->top)++] = newItem;
        return CODE_SUCCESS;
}

int dgyDictSearch(const wchar_t *name, Dictionary *dict)
{
        int entry = -1;
        for (int i = dict->top - 1; i >= 0; --i)
        {
                if (dict->dict[i].name == name)
                {
                        entry = dict->dict[i].entry;
                        break;
                }
        }
        return entry;
}