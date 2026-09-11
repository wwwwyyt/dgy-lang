#ifndef _dgy_core_h
#define _dgy_core_h

#include "dgy_error.h"
#include "dgy_stack.h"
#include "dgy_dict.h"
#include "dgy_builtin.h"
#include "dgy_analyser.h"

enum
{
        /* 最大寄存器数量 */
        CORE_MAX_REG_CNT = 16
};

/** @brief DGY 语言执行状态 */
typedef struct
{
        /** @brief 数据栈，存放执行时的数据 */
        DgyStack dataStack;

        /** @brief 代码栈，存放编译好的词语的字节码 */
        DgyStack codeStack;

        /** @brief 词典，存放词语位于代码栈中的索引等信息 */
        DgyDict wordDict;

        /** @brief 寄存器组，存放临时变量 */
        cell_t regGroup[CORE_MAX_REG_CNT];

        /** @brief 语义分析器，获取字节码流 */
        DgyAnalyser *analyser;

        /** @brief 输入输出流 */
        FILE *in, *out;
} DgyCore;

ErrCode dgyCoreInit(DgyCore *core, FILE *in, FILE *out);
ErrCode dgyCoreDestroy(DgyCore *core);

#endif
