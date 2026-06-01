#include "dgy_error.h"

static ErrCode errcode;
static wchar_t detail[512];

void dgySetErr(ErrCode code, const wchar_t *msg)
{
        errcode = code;
        wcsncpy(detail, msg, 512);
}

void dgyError(void)
{
        static wchar_t *errmsg[ERR_CNT];
        errmsg[ERR_NULLPTR] = L"Null pointer";        
        switch (errcode)
        {
        case ERR_NULLPTR:
                wprintf(L"DGYError: %ls: %ls\n", \
                        errmsg[errcode], detail);
                break;
        default:
                break;
        }
}
