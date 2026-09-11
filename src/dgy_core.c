#include "dgy_core.h"
#include "dgy_error.h"

ErrCode dgyCoreInit(DgyCore *core, FILE *in, FILE *out)
{
        if (!core)
        {
                dgySetErr(ERR_NULLPTR, L"dgyCoreInit");
                return CODE_FAILURE;
        }
        memset(core, 0, sizeof(DgyCore));
        /* 初始化数据栈 */
        dgyStackInit(&(core->dataStack), 16);
        /* 初始化词典 */
        dgyDictInit(&(core->wordDict), 16);
        /* 初始化语义分析器 */
        dgyAnalyserInit(&(core->analyser), &(core->wordDict), &(core->codeStack));
        /* 初始化输入和输出流 */
        core->in = in;
        core->out = out;
        return CODE_SUCCESS;
}

ErrCode dgyCoreDestroy(DgyCore *core)
{
        if (!core)
        {
                dgySetErr(ERR_NULLPTR, L"dgyCoreDestroy");
                return CODE_FAILURE;
        }
        /* 销毁数据栈 */
        dgyStackDestroy(&(core->dataStack));
        /* 销毁词典 */
        dgyDictDestroy(&(core->wordDict));
        /* 初始化语义分析器 */
        dgyAnalyserDestroy(&(core->analyser));
        return CODE_SUCCESS;
}