#include "dgy_analyser.h"
#include "dgy_all.h"
#include "dgy_dict.h"
#include "dgy_stack.h"

static inline void append_Immd(DgyStack *codeStack, cell_t data)
{

}

ErrCode dgyAppendData(DgyAnalyser *analyser, DgyStack *symbolStack)
{
        DgyStack *codeStack = analyser->codeStack;
        DgyDict *wordDict = analyser->wordDict;
        cell_t top;
        dgyStackTop(symbolStack, &top);
        if (top.type != CELL_FLAG_LEN)
        {
                /* Single-cell symbol */
                switch (top.type)
                {
                case CELL_LEXER_IMMD:
                        break;
                case CELL_LEXER_CHAR:
                        break;
                case CELL_LEXER_IMMD_CELL:
                        break;
                case CELL_LEXER_IMMD_REG: 
                        break;
                default:
                        break;
                }                
        }
        else
        {
                /* Multi-cell symbol */
                cell_t second;
                dgyStackItemAt(symbolStack, 2, &second);
                switch (top.type)
                {
                case CELL_LEXER_WORD:
                        break;
                case CELL_LEXER_EXTERN_WORD:
                        break;
                case CELL_LEXER_STR:
                        break;                        
                case CELL_LEXER_WORD_CELL:
                        break;
                case CELL_LEXER_WORD_REG:
                        break;
                default:
                        break;
                }                
        }
        return CODE_SUCCESS;
}

ErrCode dgyAppendAddr(DgyAnalyser *analyser, DgyStack *symbolStack)
{
        DgyStack *codeStack = analyser->codeStack;
        DgyDict *wordDict = analyser->wordDict;
        cell_t top;
        dgyStackTop(symbolStack, &top);
        if (top.type != CELL_FLAG_LEN)
        {
                /* Single-cell symbol */
                switch (top.type)
                {
                case CELL_LEXER_IMMD_CELL:
                        break;
                case CELL_LEXER_IMMD_REG:
                        break;
                default:
                        break;
                }                
        }
        else
        {
                /* Multi-cell symbol */
                cell_t second;
                dgyStackItemAt(symbolStack, 2, &second);
                switch (top.type)
                {
                case CELL_LEXER_WORD:
                        break;
                case CELL_LEXER_EXTERN_WORD:
                        break;
                case CELL_LEXER_WORD_CELL:
                        break;
                case CELL_LEXER_WORD_REG:
                        break;
                default:
                        break;
                }                
        }
        return CODE_SUCCESS;
}

ErrCode dgyAppendInstr(DgyAnalyser *analyser, StatType statType)
{
        switch (statType)
        {
        case ST_WORD_BEGIN:
                break;
        case ST_WORD_END:
                break;
        case ST_MOV:
                break;
        case ST_SET_REG:
                break;
        case ST_EXEC:
                break;
        case ST_IF:
                break;
        case ST_ELSE:
                break;
        case ST_ELSE_END:
                break;
        case ST_HEREIS:
                break;
        case ST_GOTO:
                break;
        case ST_LOOP_BEGIN:
                break;
        case ST_LOOP_CHECK:
                break;
        case ST_LOOP_END:
        default:
                break;
        }
        return CODE_SUCCESS;
}

ErrCode dgyDiscard(DgyAnalyser *analyser)
{
        DgyStack *codeStack = analyser->codeStack;
        while (codeStack->sp > analyser->oldsp)
        {
                dgyStackPop(codeStack);
        }
        return CODE_SUCCESS;
}

ErrCode dgyAnalyserInit(DgyAnalyser *analyser,
                        DgyStack *codeStack,
                        DgyDict *wordDict)
{
        if (!analyser || !codeStack || !wordDict)
        {
                dgySetErr(ERR_NULLPTR, L"dgyAnalyserInit");
                return CODE_FAILURE;
        }
        analyser->codeStack = codeStack;
        analyser->wordDict = wordDict;
        analyser->oldsp = codeStack->sp; /* Record old sp at initialization */
        return CODE_SUCCESS;
}
