#ifndef _dgy_parser_h
#define _dgy_parser_h

#include "dgy_type.h"
#include "dgy_cell.h"
#include "dgy_error.h"
#include "dgy_lexer.h"
#include "dgy_stack.h"
#include "dgy_dict.h"
#include "dgy_stat.h"

enum
{
        MATCH_COMPLETED = -1
};

/** @brief 语法分析层，从输入流获取符号，输出语句的中间表示 */
typedef struct
{
        /** @brief 用于语法分析，仅保存符号类型 */
        DgyStack analyStack;

        /** @brief 用于存储词法分析返回的符号信息 */
        DgyStack symbolStack;

        /** @brief 用于存储一个语句的中间表示 */
        DgyStatement statement;

        /** @brief 当前分析的符号在符号栈中的索引 */
        i32 _symbolIdx;
} DgyParser;

ErrCode dgyParserInit(DgyParser *parser);
ErrCode dgyParserDestroy(DgyParser *parser);
ErrCode dgyDoParserOnce(DgyParser *parser, FILE *in);

#endif /* _dgy_parser_h */
