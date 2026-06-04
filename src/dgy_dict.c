#include "dgy_dict.h"

static ErrCode resize(size_t newSize, Dictionary *dict)
{
        DictItem *newDict = (DictItem *)realloc(dict->dict, newSize * sizeof(DictItem));
        if (newDict == NULL)
        {
                perror("dgy_dict: resize: realloc() failed");
                return CODE_FAILURE;
        }
        dict->dict = newDict;
        dict->size = newSize;
        return CODE_SUCCESS;
}

ErrCode dgyDictInit(Dictionary *dict, size_t size)
{
        memset(dict, 0, sizeof(Dictionary));
        dict->size = size;
        dict->top = 0;
        dict->dict = (DictItem *)malloc(dict->size * sizeof(DictItem));
        if (dict->dict == NULL)
        {
                perror("dgyDictInit: malloc() failed");
                return CODE_FAILURE;
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

ErrCode dgyDictForget(const wchar_t *name, Dictionary *dict)
{
        int entry = dgyDictSearch(name, dict);
        if (entry == -1)
        {
                return CODE_SUCCESS;
        }
        for (int i = entry; i < dict->top - 1; ++i)
        {
                dict->dict[i].name = dict->dict[i + 1].name;
                dict->dict[i].entry = dict->dict[i + 1].entry;
        }
        (dict->top)--;
        if (dict->top == dict->size / 4 && (CODE_SUCCESS != resize(dict->size / 2, dict)))
        {
                return CODE_FAILURE;
        }        
        return CODE_SUCCESS;
}
