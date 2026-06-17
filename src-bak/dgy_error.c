#include "dgy_error.h"

static ErrCode _errcode;
static wchar_t _detail[512];
static wchar_t _errBuffer[1024];

void dgySetErr(ErrCode code, const wchar_t *msg)
{
        _errcode = code;
        wcsncpy(_detail, msg, 512);
}

void dgyGetErr(void)
{
        static wchar_t *errmsg[ERR_CNT];
        i32 init = 0;
        if (init == 0)
        {
                init = 1;
                errmsg[ERR_NULLPTR] = L"Null pointer";
                errmsg[ERR_UNDERFLOW] = L"Stack underflow";
                errmsg[ERR_OVERFLOW] = L"Stack overflow";
                errmsg[ERR_OUT_OF_BOUNDS] = L"Array out of bounds";
        }
        switch (_errcode)
        {
        case ERR_NULLPTR:
        case ERR_UNDERFLOW:
        case ERR_OVERFLOW:
        case ERR_OUT_OF_BOUNDS:
                wprintf(L"DGYError: %ls: %ls\n",
                        errmsg[_errcode], _detail);
                break;
        default:
                break;
        }
}

void dgyPrintErrPos(wchar_t *symbol, size_t symbolLen, const char *fname, i32 row, i32 col)
{
        const wchar_t *fmt[] = {
                L"Symbol: '%ls' ",
                L"In file: '%s' ",
                L"At row:%d ",
                L"At col:%d "
        };
        if (symbol)
        {
                symbol[symbolLen] = L'\0';
                wprintf(fmt[0], symbol);
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
