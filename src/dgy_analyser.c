#include "dgy_analyser.h"

static SymbolVal getSymAt(DgyAnalyser *analyser, i32 idx)
{
        SymbolVal data;
        cell_t c;
        dgyStackGetItemAt(&(analyser->parser.symbolStack), idx, &c, NULL);
        switch (c.type)
        {
        case CELL_FLAG_LEN:
                for (i32 i = 1; i <= c.data.sint; ++i)
                {
                        cell_t t;
                        dgyStackGetItemAt(&(analyser->parser.symbolStack), idx - i, &t, NULL);
                        switch (t.type)
                        {
                        case CELL_LEXER_STR:
                                data.str[i - 1] = t.data.wchar;
                                break;
                        case CELL_LEXER_WORD:
                        case CELL_LEXER_EXTERN_WORD:
                                data.word[i - 1] = t.data.wchar;
                                break;
                                // TODO: 词语单元
                        case CELL_LEXER_WORD_CELL:
                        case CELL_LEXER_WORD_REG:
                        default:
                                break;
                        }
                }
                break;
        case CELL_LEXER_CHAR:
                data.ch = c.data.wchar;
                break;
        case CELL_LEXER_IMMD:
        case CELL_LEXER_RESERVED:
        case CELL_LEXER_OP:
        case CELL_LEXER_IMMD_CELL:
        case CELL_LEXER_IMMD_REG:
                data.uint = c.data.uint;
                break;
        default:
                break;
        }
        return data;
}

static cell_t getCellAt(DgyAnalyser *analyser, i32 idx)
{
        cell_t data;
        dgyStackGetItemAt(&(analyser->parser.symbolStack), idx, &data, NULL);
        return data;
}

/** @brief 复杂词语声明开始
 *
 * 词语名称和数据栈栈顶入分析栈，层级加一
 */
static void parse_WordBegin(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
        DictItem item = {
            .name = getSymAt(analyser, statement->data.WordBegin.word).word,
            .entry = analyser->codeStack->sp, // 此处用于存储代码栈栈顶
        };
        dgyDictAdd(&(analyser->analysDict), &item);
        analyser->level++;
}

/** @brief 复杂词语声明结束
 *
 * 将当前词语名与分析栈顶的词语名比较，不同则报错，相同则：
 * 1. 记录此时的代码栈栈顶，与分析栈顶记录的栈顶相减，得到该词语的长度
 * 2. 将层级减一
 * 3. 将该词语的名称、长度、层级、索引（当前代码栈栈顶 - 1）加入词典
 * 4. 弹出分析栈栈顶元素
 */
static void parse_WordEnd(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
        DictItem begin;
        dgyDictGetLatest(&(analyser->analysDict), &begin);
        if (wcscmp(statement->data.WordEnd.word, begin.name) == 0)
        {
                i32 entry = analyser->codeStack->sp;
                i32 length = entry - begin.entry;
                analyser->level--;
                DictItem item = {
                    .name = begin.name,
                    .entry = entry,
                    .level = analyser->level,
                    .length = length,
                };
                dgyDictAdd(analyser->wordDict, &item);
                dgyDictForgetLatest(&(analyser->analysDict));
        }
        else
        {
                wprintf(ERR_UNCLOSED_SYMBOL(statement->data.WordEnd.word));
        }
}

/** @brief 存值语句 */
static void parse_Mov(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
}

/** @brief 简单词语声明
 *
 * 直接将该词语的名称、长度（为 1 ）、层级（当前层级）、
 * 索引（当前数据栈栈顶）加入词典；将之后的单元压入代码栈
 */
static void parse_SimpWord(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
        DictItem item = {
            .name = getSymAt(analyser, statement->data.SimpWord.word).word,
            .entry = analyser->codeStack->sp,
            .level = analyser->level,
            .length = 1,
        };
        dgyDictAdd(analyser->wordDict, &item);
        dgyStackPush(analyser->codeStack, getCellAt(analyser, statement->data.SimpWord.src));
}

static void parse_Exec(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
}

static void parse_If(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
}

static void parse_Else(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
}

static void parse_ElseEnd(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
}

static void parse_Hereis(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
}

static void parse_Goto(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
}

static void parse_LoopBegin(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
}

static void parse_LoopCheck(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
}

static void parse_LoopEnd(DgyAnalyser *analyser)
{
        DgyStatement *statement = &(analyser->parser.statement);
}

static void (*_parserFuncList[])(DgyAnalyser *) = {
    /* 必须与 StatType 中的枚举值顺序相同 */
    parse_WordBegin,
    parse_WordEnd,
    parse_Mov,
    parse_SimpWord,
    parse_Exec,
    parse_If,
    parse_Else,
    parse_ElseEnd,
    parse_Hereis,
    parse_Goto,
    parse_LoopBegin,
    parse_LoopCheck,
    parse_LoopEnd,
};

ErrCode dgyDoAnalyserOnce(DgyAnalyser *analyser, FILE *in)
{
        dgyDoParserOnce(&(analyser->parser), in);
        DgyStatement *statement = &(analyser->parser.statement);
        _parserFuncList[statement->type](analyser);
        return CODE_SUCCESS;
}

ErrCode dgyAnalyserInit(DgyAnalyser *analyser, DgyDict *wordDict, DgyStack *codeStack)
{
        if (!analyser)
        {
                dgySetErr(ERR_NULLPTR, L"dgyAnalyserInit");
                return CODE_FAILURE;
        }
        memset(analyser, 0, sizeof(DgyAnalyser));
        /* 初始化语法分析器 */
        dgyParserInit(&(analyser->parser));
        /* 初始化词语分析栈 */
        dgyDictInit(&(analyser->analysDict), 16);
        /* 初始化字节码流 */
        analyser->bcstream = tmpfile();
        /* 获取词典的指针 */
        analyser->wordDict = wordDict;
        /* 获取代码栈指针 */
        analyser->codeStack = codeStack;
        return CODE_SUCCESS;
}

ErrCode dgyAnalyserDestroy(DgyAnalyser *analyser)
{
        if (!analyser)
        {
                dgySetErr(ERR_NULLPTR, L"dgyAnalyserDestroy");
                return CODE_FAILURE;
        }
        /* 销毁语法分析器 */
        dgyParserDestroy(&(analyser->parser));
        /* 关闭字节码流 */
        fclose(analyser->bcstream);
        return CODE_SUCCESS;
}
