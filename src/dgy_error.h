#ifndef _dgy_error_h
#define _dgy_error_h

#include "dgy_all.h"

#define ERR_UNCLOSED_SYMBOL(sym)                \
        L"【错误】: 未闭合符号: '" sym "'\n"
#define ERR_INVALID_SYMBOL(sym)                 \
        L"【错误】: 非法符号: '" sym "'\n"
#define ERR_EXPECT_SYMBOL(sym)                  \
        L"【错误】: 期望符号: '" sym "'\n"
#define ERR_UNRECOGNIZED_CHAR(ch)                  \
        L"【错误】: 未识别的字符: '%lc'\n", ch
#define WARN_UNDEFINED_WORD(word)                       \
        L"【警告】: 未定义的词语: '%ls'\n", word
#define WARN_OUT_OF_BUFFER(len)                                 \
        L"【警告】: 超出缓冲区. 超出长度: %d\n", len

typedef enum
{
        CODE_SUCCESS = 0,
        CODE_FAILURE = 1,
        ERR_NULLPTR = 2,
        ERR_UNDERFLOW = 3,
        ERR_OVERFLOW = 4,
        ERR_OUT_OF_BOUNDS = 5,
        ERR_CNT,
} ErrCode;

void dgySetErr(ErrCode code, const wchar_t *msg);
void dgyGetErr(void);
void dgyPrintErrPos(wchar_t *symbol, int symbolLen, const char *fname, int row, int col);

#endif /* _dgy_error_h */
