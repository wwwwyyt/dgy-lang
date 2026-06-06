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
