#include "dgy_error.h"

static ErrCode _errcode;
static wchar_t _detail[512];

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
