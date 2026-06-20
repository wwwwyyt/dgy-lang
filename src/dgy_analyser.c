#include "dgy_analyser.h"
#include "dgy_all.h"
#include "dgy_dict.h"

static inline void append_Immd(DgyStack *codeStack, cell_t data)
{
        cell_t code = data;
        code.type = CELL_CODE_DATA;
        dgyStackPush(codeStack, code);
}

static inline void append_Char(DgyStack *codeStack, cell_t data)
{
        cell_t code = data;
        code.type = CELL_CODE_DATA;
        dgyStackPush(codeStack, code);
}

static inline void append_ImmdCell(DgyStack *codeStack, cell_t data)
{
        cell_t code = data;
        code.type = CELL_CODE_DATA_ADDR;
        dgyStackPush(codeStack, code);
}

static inline void append_ImmdReg(DgyStack *codeStack, cell_t data)
{
        cell_t code = data;
        code.type = CELL_CODE_REG_ADDR;
        dgyStackPush(codeStack, code);
}

static inline void append_Word(DgyAnalyser *analyser,
                               DgyStack *symbolStack,
                               i32 length)
{
        /* Get word's name */
        cell_t *s = symbolStack->stack;
        i32 wordEnd = symbolStack->sp - 2;
        wchar_t wordName[length + 1];
        wordName[length] = L'\0';
        for (i32 i = length - 1; i >= 0; --i)
        {
                wordName[i] = s[wordEnd--].data.wchar;
        }
        /* Search the wordDict for word's entry */
        i32 wordEntry = dgyDictSearchIn(analyser->wordDict, wordName, analyser->parentEntry);
        /* Append the code of word to codeStack */
        if (wordEntry != -1)
        {
                cell_t code = {.data.sint=wordEntry, .type=CELL_CODE_WORD};
                dgyStackPush(analyser->codeStack, code);
        }
}

static inline void append_ExternWord(DgyAnalyser *analyser,
                                     DgyStack *symbolStack,
                                     i32 length)
{

}

static inline void append_Str(DgyAnalyser *analyser,
                              DgyStack *symbolStack,
                              i32 length)
{
        
}

static inline void append_WordCell(DgyAnalyser *analyser,
                                   DgyStack *symbolStack,
                                   i32 length)
{
        
}

static inline void append_WordReg(DgyAnalyser *analyser,
                                  DgyStack *symbolStack,
                                  i32 length)
{
        
}

ErrCode dgyAppendData(DgyAnalyser *analyser, DgyStack *symbolStack)
{
        DgyStack *codeStack = analyser->codeStack;
        cell_t top;
        dgyStackTop(symbolStack, &top);
        if (top.type != CELL_FLAG_LEN)
        {
                /* Single-cell symbol */
                switch (top.type)
                {
                case CELL_LEXER_IMMD:
                        append_Immd(codeStack, top);
                        break;
                case CELL_LEXER_CHAR:
                        append_Char(codeStack, top);
                        break;
                case CELL_LEXER_IMMD_CELL:
                        append_ImmdCell(codeStack, top);
                        break;
                case CELL_LEXER_IMMD_REG:
                        append_ImmdReg(codeStack, top);
                        break;
                default:
                        break;
                }
                /* Restore the state of symbolStack */
                dgyStackPop(symbolStack);
        }
        else
        {
                /* Multi-cell symbol */
                i32 length = top.data.sint;
                cell_t second;                
                dgyStackItemAt(symbolStack, 2, &second);
                switch (second.type)
                {
                case CELL_LEXER_WORD:
                        append_Word(analyser, symbolStack, length);
                        break;
                case CELL_LEXER_EXTERN_WORD:
                        append_ExternWord(analyser, symbolStack, length);
                        break;
                case CELL_LEXER_STR:
                        append_Str(analyser, symbolStack, length);
                        break;                        
                case CELL_LEXER_WORD_CELL:
                        append_WordCell(analyser, symbolStack, length);
                        break;
                case CELL_LEXER_WORD_REG:
                        append_WordReg(analyser, symbolStack, length);
                        break;
                default:
                        break;
                }
                /* Restore the state of symbolStack */
                for (i32 i = 0; i < length + 1; ++i)
                {
                        dgyStackPop(symbolStack);
                }                
        }
        return CODE_SUCCESS;
}

ErrCode dgyAppendAddr(DgyAnalyser *analyser, DgyStack *symbolStack)
{
        DgyStack *codeStack = analyser->codeStack;
        // DgyDict *wordDict = analyser->wordDict;
        cell_t top;
        dgyStackTop(symbolStack, &top);
        if (top.type != CELL_FLAG_LEN)
        {
                /* Single-cell symbol */
                switch (top.type)
                {
                case CELL_LEXER_IMMD_CELL:
                        append_ImmdCell(codeStack, top);
                        break;
                case CELL_LEXER_IMMD_REG:
                        append_ImmdReg(codeStack, top);
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
                switch (second.type)
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
        analyser->level = 0;             /* Default level is 0 */
        analyser->parentEntry = -1;      /* In the beginning, there
                                            is no parent word block.
                                            So parentEntry is -1. */
        return CODE_SUCCESS;
}
