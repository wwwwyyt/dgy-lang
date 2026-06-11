#include "dgy_error.h"

static ErrCode _errcode;
static wchar_t _detail[512];

void dgySetErr(ErrCode code, const wchar_t *msg)
{
        _errcode = code;
        wcsncpy(_detail, msg, 512);
}

void dgyError(void)
{
        static wchar_t *errmsg[ERR_CNT];
        errmsg[ERR_NULLPTR] = L"Null pointer";
        switch (_errcode)
        {
        case ERR_NULLPTR:
                wprintf(L"DGYError: %ls: %ls\n",
                        errmsg[_errcode], _detail);
                break;
        default:
                break;
        }
}

void dgyPrintErrPos(wchar_t *symble, int symbleLen, const char *fname, int row, int col)
{
        const wchar_t *fmt[] = {
                L"Symble: '%ls' ",
                L"In file: '%s' ",
                L"At row:%d ",
                L"At col:%d "
        };                
        if (symble)
        {
                symble[symbleLen] = L'\0';
                wprintf(fmt[0], symble);
        }
        if (fname)
        {       
                wprintf(fmt[1], fname);
        }
        if (row)
        {                
                wprintf(fmt[2], row);
        }
        if (col)
        {
                wprintf(fmt[3], col);
        }
        wprintf(L"\n");
}
