#ifndef _dgy_analyser_h
#define _dgy_analyser_h

#include <stdio.h>
#include "dgy_parser.h"

typedef union
{
        wchar_t word[S_MAX_WORD_LEN];
        wchar_t str[S_MAX_STR_LEN];
        u64 uint;
        i64 sint;
        wchar_t ch;
} SymbolVal;

/** @brief 语义分析层 */
typedef struct
{
        /** @brief 语法分析器 */
        DgyParser parser;

        /** @brief 词语分析栈 */
        DgyDict analysDict;

        /** @brief 字节码流 */
        FILE *bcstream;

        /** @brief 词典指针 */
        DgyDict *wordDict;

        /** @brief 代码栈指针 */
        DgyStack *codeStack;

        /** @brief 层级，记录词语所在的层级 */
        i32 level;

        /** @brief 用于存放多单元符号（字符串、词语）的缓冲区 */

} DgyAnalyser;

ErrCode dgyAnalyserInit(DgyAnalyser *analyser, DgyDict *wordDict, DgyStack *codeStack);
ErrCode dgyAnalyserDestroy(DgyAnalyser *analyser);
ErrCode dgyDoAnalyserOnce(DgyAnalyser *analyser, FILE *in);

#endif
