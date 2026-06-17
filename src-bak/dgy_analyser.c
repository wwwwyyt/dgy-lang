#include "dgy_analyser.h"

static void (*_analysers[ANALYSE_OP_CNT])(DgyStack *, Dictionary *);

static void discard(DgyStack *stack, Dictionary *_)
{
        wint_t length = dgyStackTop(stack);
        stack->sp -= length + 1;
}

static void getValue_Immd(DgyStack *stack, int bp, wchar_t *s)
{
        cell_t val;
        val = wcstoll(s, NULL, 0);
        stack->stack[bp] = val;
        stack->sp = bp + 1;        
}

static void getValue_Word(DgyStack *stack, Dictionary *dict, int bp, wchar_t *word)
{
        cell_t entry = dgyDictSearch(word, dict);
        if (entry == -1)
        {
                dgyError();
                wprintf(WARN_UNDEFINED_WORD(word));
                return;
        }
        stack->stack[bp] = entry;
        stack->sp = bp + 1;        
}

static void getValue_ExternWord(DgyStack *stack, Dictionary *dict, int bp, wchar_t *word)
{
        cell_t entry = dgyDictSearch(word, dict);
        if (entry == -1)
        {
                dgyError();
                wprintf(WARN_UNDEFINED_WORD(word));
                return;
        }
        stack->stack[bp] = entry;
        stack->sp = bp + 1;        
}

static void getValue(DgyStack *stack, Dictionary *dict)
{
        wint_t length = dgyStackTop(stack);
        dgyStackPop(stack);
        SymbolType type = dgyStackTop(stack);
        int bp = stack->sp - length;
        /* Construct the wchar_t string of symbols */
        wchar_t s[length];
        for (int i = 0; i < length - 1; ++i)
        {
                s[i] = stack->stack[bp + i];
        }
        s[length - 1] = L'\0';
        /* Matching symbol type */
        switch (type)
        {
        case S_IMMD:
                getValue_Immd(stack, bp, s);
                break;
        case S_STR:
                (stack->sp)--;
                break;
        case S_CHAR:
                (stack->sp)--;
                break;
        case S_WORD:
                getValue_Word(stack, dict, bp, s);
                break;
        case S_EXTERN_WORD:
                getValue_ExternWord(stack, dict, bp, s);
                break;
        case S_CELL:
                break;
        case S_REG:
                break;
        default:
                break;
        }
}

ErrCode dgyAnalyse(DgyStack *codeStack, Dictionary *dict, AnalyseOp op)
{
        static int init = 0;
        if (init == 0)
        {
                init = 1;
                _analysers[A_DISCARD] = discard;
                _analysers[A_GET_VALUE] = getValue;
        }
        if (op >= 0 && op < ANALYSE_OP_CNT)
        {
                _analysers[op](codeStack, dict);
        }
        else
        {
                dgySetErr(ERR_OUT_OF_BOUNDS, L"dgyAnalyse");
                return CODE_FAILURE;
        }
        return CODE_SUCCESS;
}
