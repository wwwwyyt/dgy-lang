#include "dgy_core.h"
#include "dgy_error.h"

static DgyCore _core;

static void execDataStack(void)
{
        cell_t builtin;
        dgyStackTop(&_core.dataStack, &builtin);
        if (CELL_CODE_BUILTIN_WORD == builtin.type)
        {
                BuiltinCode code = builtin.data.sint;
                dgyExecBuiltin(&_core.dataStack, code);
                dgyGetErr();
        }
}

void dgyCoreInit(void)
{
        dgyStackInit(&_core.codeStack, 16);
        dgyStackInit(&_core.dataStack, 16);
        dgyStackInit(&_core.execStack, 16);
        dgyDictInit(&_core.wordDict, 16);
        dgyBuiltinInit(&_core.codeStack, &_core.wordDict);
}
