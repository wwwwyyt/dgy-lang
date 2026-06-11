#include "dgy_core.h"

static Core _core;

static ErrCode wordDictAndCodeStackInit(void)
{ 
        ErrCode code = dgyDictInit(&_core.wordDict, 16);
        if (code != CODE_SUCCESS)
                return code;
        code = dgyStackInit(&_core.codeStack, 16);
        if (code != CODE_SUCCESS)
                return code;
        for  (BuiltInWordCode i = W_NOP; i < BUILT_IN_WORD_CNT; ++i)
        {
                /* The length of each built-in word is 1 */
                code = dgyStackPush(&_core.codeStack, 1);
                if (code != CODE_SUCCESS)
                        return code;
                /* The entry points to length */
                int entry = _core.codeStack.sp - 1;
                /* Built-in word op code */
                code = dgyStackPush(&_core.codeStack, i);
                if (code != CODE_SUCCESS)
                        return code;
                /* Add dictionary item */
                code = dgyDictAdd(dgyGetBuiltInWordName(i),
                                  entry,
                                  &_core.wordDict);
                if (code != CODE_SUCCESS)
                        return code;
        }        
        return code;
}

static void init()
{
        ErrCode code;

        /* Set locale */
        setlocale(LC_ALL, "zh_CN.utf8");

        /* Initialize data stack */
        code = dgyStackInit(&_core.dataStack, 16);
        if (code != CODE_SUCCESS)
                dgyError();

        /* Initialize register stack */
        code = dgyStackInit(&_core.regStack, 16);
        if (code != CODE_SUCCESS)
                dgyError();

        /* Initialize name dictionary and code stack */
        code = wordDictAndCodeStackInit();
        if (code != CODE_SUCCESS)
                dgyError();
}

void dgyExec(void)
{
        DgyStack *stack = &(_core.dataStack);
        if (!dgyStackEmpty(&(_core.dataStack)))
        {
                cell_t entry = dgyStackTop(stack);
                dgyStackPop(stack);
                BuiltInWordCode op = _core.codeStack.stack[entry];
                dgyExecBuiltIn(op, stack);
        }
}

ErrCode dgyDo()
{
        init();

        if (0)
        {
                DgyStack *stack = &_core.dataStack;

                dgyStackPush(stack, L'整');
                dgyStackDump(stack, -1, -1);
                dgyStackPush(stack, L'数');
                dgyStackDump(stack, -1, -1);
                dgyStackPush(stack, W_FORMAT);
                dgyStackDump(stack, -1, -1);
                dgyExec();
                dgyStackDump(stack, -1, -1);
                dgyStackPush(stack, 10);
                dgyStackDump(stack, -1, -1);
                dgyStackPush(stack, 20);
                dgyStackDump(stack, -1, -1);
                dgyStackPush(stack, W_ADD);
                dgyStackDump(stack, -1, -1);
                dgyExec();        
                dgyStackDump(stack, -1, -1);
                dgyStackPush(stack, W_PRINTF);
                dgyStackDump(stack, -1, -1);
                dgyExec();
                dgyStackDump(stack, -1, -1);
        }
        
        return CODE_SUCCESS;
}

ErrCode dgyTestDo()
{
        ErrCode code = CODE_FAILURE;
        dgyParserSetDict(&_core.wordDict);
        dgyDictDump(&_core.wordDict);
        int bp = _core.codeStack.sp;
        while (1)
        {
                code = dgyDoParserOnce(stdin, &_core.codeStack);
                if (CODE_SUCCESS != code)
                        return code;
                dgyStackDump(&_core.codeStack, bp,_core.codeStack.sp);
        }
        return code;
}

void dgyQuit(void)
{
        dgyStackDestroy(&_core.dataStack);
        dgyStackDestroy(&_core.codeStack);
        dgyDictDestroy(&_core.wordDict);
}
