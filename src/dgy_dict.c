#include "dgy_dict.h"

static ErrCode resize(size_t newSize, Dictionary *dict)
{
        if (!dict)
        {
                dgySetErr(ERR_NULLPTR, L"dgy_dict: resize");
                return CODE_FAILURE;
        }
        DictItem *newDict = (DictItem *)realloc(dict->dict, newSize * sizeof(DictItem)); // Allocate Dictionary.dict
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
        if (!dict)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDictInit");
                return CODE_FAILURE;
        }        
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
        if (!name || !dict)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDictAdd");
                return CODE_FAILURE;
        }
        size_t nameLen = wcslen(name);
        DictItem newItem;
        newItem.name = (wchar_t *)malloc(nameLen * sizeof(wchar_t)); // Allocate DictItem.name
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
        if (!name || !dict)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDictSearch");
                return -1;
        }
        int entry = -1;
        /* Start the search with the dictionary's latest entry */
        for (int i = dict->top - 1; i >= 0; --i)
        {
                if (wcscmp(dict->dict[i].name, name) == 0)
                {
                        entry = dict->dict[i].entry;
                        break;
                }
        }
        return entry;
}

ErrCode dgyDictForget(const wchar_t *name, Dictionary *dict)
{
        if (!name || !dict)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDictForget");
                return CODE_FAILURE;
        }        
        int entry = dgyDictSearch(name, dict);
        if (entry == -1)
        {
                return CODE_SUCCESS;
        }
        free(dict->dict[entry].name); // Free DictItem.name        
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

ErrCode dgyDictDestroy(Dictionary *dict)
{
        if (!dict)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDictDestroy");
                return CODE_FAILURE;
        }                
        for (int i = 0; i < dict->top; ++i)
        {
                free(dict->dict[i].name); // Free DictItem.name
                memset(&(dict->dict[i]), 0, sizeof(DictItem));
        }
        free(dict->dict); // Free Dictionary.dict
        memset(dict, 0, sizeof(Dictionary));
        return CODE_SUCCESS;
}


ErrCode dgyDictDump(Dictionary *dict)
{
        if (!dict)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDictDump");
                return CODE_FAILURE;
        }
        for (int i = 0; i < dict->top; ++i)
        {
                DictItem item = dict->dict[i];
                wprintf(L"%ls : %d\n", item.name, item.entry);
        }
        return CODE_SUCCESS;
}
