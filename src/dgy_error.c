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
                errmsg[ERR_NULLPTR] = L"空指针";
                errmsg[ERR_UNDERFLOW] = L"栈下溢";
                errmsg[ERR_OVERFLOW] = L"栈上溢";
                errmsg[ERR_OUT_OF_BOUNDS] = L"数组越界";
                errmsg[ERR_DIV_BY_ZERO] = L"除零";
        }
        switch (_errcode)
        {
        case ERR_NULLPTR:
        case ERR_UNDERFLOW:
        case ERR_OVERFLOW:
        case ERR_OUT_OF_BOUNDS:
                wprintf(L"【错误】: %ls: %ls\n",
                        errmsg[_errcode], _detail);
                break;
        default:
                break;
        }
}
