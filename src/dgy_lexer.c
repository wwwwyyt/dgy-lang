#include "dgy_lexer.h"
#include "dgy_all.h"
#include "dgy_error.h"
#include "dgy_stack.h"

static bool isCharAt(wchar_t wc, const wchar_t *wcs, i32 idx);
static bool isValidWordChar(wchar_t wc);
static void sym_escape_seq(FILE *in, i32 *len, wchar_t *buffer);
static SymbolType sym_Immd(FILE *in, wint_t wc, wchar_t *buffer);
static SymbolType sym_Str(FILE *in, wint_t wc, wchar_t *buffer);
static SymbolType sym_Comment(FILE *in, wint_t wc, wchar_t *buffer);
static SymbolType sym_Reserved(FILE *in, wint_t wc, wchar_t *buffer);
static SymbolType sym_Op(FILE *in, wint_t wc, wchar_t *buffer);
static SymbolType sym_Word(FILE *in, wint_t wc, wchar_t *buffer);
static SymbolType sym_Cell(FILE *in, wint_t wc, wchar_t *buffer);

static void matched_Immd(const wchar_t buffer[], SymbolType type, DgyStack *out);
static void matched_Str(const wchar_t buffer[], SymbolType type, DgyStack *out);
static void matched_Comment(const wchar_t buffer[], SymbolType type, DgyStack *out);
static void matched_Reserved(const wchar_t buffer[], SymbolType type, DgyStack *out);
static void matched_Op(const wchar_t buffer[], SymbolType type, DgyStack *out);
static void matched_Word(const wchar_t buffer[], SymbolType type, DgyStack *out);
static void matched_Cell(const wchar_t buffer[], SymbolType type, DgyStack *out);

static i32 (*_matchSymbol[SYM_CNT])(FILE *, wint_t, wchar_t *) =
{
        /* Must be the same order of _matchedProcess. */
        sym_Immd,
        sym_Str,
        sym_Comment,
        sym_Reserved,
        sym_Op,
        sym_Word,
        sym_Cell,
};

static void (*_matchedProcess[SYM_CNT])(const wchar_t[], SymbolType, DgyStack *) =
{
        /* Must be the same order of _matchSymbol. */
        matched_Immd,
        matched_Str,
        matched_Comment,
        matched_Reserved,
        matched_Op,
        matched_Word,
        matched_Cell,
};

static const wchar_t *_reservedSymTable[RESERVED_SYM_CNT] =
{
        // Must be sorted in descending order of length
        L"重复执行",            /* S_CHONG_FU_ZHI_XING */
        L"否则结束",            /* S_FOU_ZE_JIE_SHU */
        L"结果存",              /* S_JIE_GUO_CUN */
        L"无结果",              /* S_WU_JIE_GUO */
        L"不成立",              /* S_BU_CHENG_LI */
        L"无条件",              /* S_WU_TIAO_JIAN */
        L"这里是",              /* S_ZHE_LI_SHI */
        L"检测",                /* S_JIAN_CE */
        L"条件",                /* S_TIAO_JIAN */
        L"成立",                /* S_CHENG_LI */
        L"直到",                /* S_ZHI_DAO */
        L"否则",                /* S_FOU_ZE */
        L"如果",                /* S_RU_GUO */
        L"存",                  /* S_CUN */
        L"到",                  /* S_DAO */
        L"令",                  /* S_LING */
        L"求",                  /* S_QIU */
        L"去",                  /* S_QU */
        L"就",                  /* S_JIU */
        L"设",                  /* S_SHE */
        L"次",                  /* S_CI */
};

static const wchar_t * _opSymTable[OP_SYM_CNT] =
{
        // Must be sorted in descending order of length
        L"<=",                  /* S_BEQ */
        L">=",                  /* S_AEQ */
        L"/=",                  /* S_NEQ */
        L"<",                   /* S_BELOW */
        L">",                   /* S_ABOVE */
        L"=",                   /* S_EQ */
        L"且",                  /* S_AND */
        L"或",                  /* S_OR */
        L"非",                  /* S_NOT */
        L"/",                  /* S_SLASH */
        L"\\",                 /* S_BACKSLASH */
        L"~",                  /* S_TILDE */
};

static wint_t getWideChar(wint_t *wc, FILE *in)
{
        *wc = fgetwc(in);
        if (*wc == WEOF)
        {
                if (ferror(in))
                {
                        if (errno == EILSEQ)
                                wprintf(L"Character encoding error while reading.\n");
                        else
                                wprintf(L"I/O error when reading\n");
                }
        }
        return *wc;
}

static void ungetWideChar(wint_t wc, FILE *in)
{
        ungetwc(wc, in);
}

static bool isCharAt(wchar_t wc, const wchar_t *wcs, i32 idx)
{
        if (idx >= wcslen(wcs))
                return 0;
        else
                return wc == wcs[idx];
}

static bool isValidWordChar(wchar_t wc)
{
        if ((wc >= 0x4E00 && wc <= 0x62FF) ||
            (wc >= 0x7700 && wc <= 0x9FFF)) // Chinese character
                return 1;
        else if (iswalpha(wc)) // English character
                return 1;
        else if (iswdigit(wc)) // 0-9
                return 1;
        else if (wc == L'_') // underline
                return 1;
        else
                return 0;
}

static void sym_escape_seq(FILE *in, i32 *len, wchar_t *buffer)
{
        wint_t wc;
        if (getWideChar(&wc, in) != WEOF)
        {
                switch (wc)
                {
                case L'n':
                        buffer[(*len)++] = L'\n';
                        break;
                case L'\\':
                case L'*':
                default:
                        buffer[(*len)++] = wc;
                        break;
                }
        }
}

static SymbolType sym_Immd(FILE *in, wint_t wc, wchar_t *buffer)
{
        enum
        {
                START,
                INT_1,
                INT_2,
                INT_END,
                HEX_1,
                HEX_2,
                HEX_3,
                HEX_END,
        };
        enum
        {
                MAX_BUF_SIZE = MAX_IMMD_LEN,
        };
        SymbolType matched = S_UNDEFINED;
        i32 bufIdx = 0;
        i32 outOfBuf = 0;
        i32 status = START;
        if (wc == L'+' || wc == L'-')
        {
                status = INT_1; // integer
                buffer[bufIdx++] = wc;
        }
        else if (wc != L'0' && iswdigit(wc))
        {
                status = INT_2; // integer
                buffer[bufIdx++] = wc;
        }
        else if (wc == L'0')
        {
                status = HEX_1; // hexdecimal
                buffer[bufIdx++] = wc;
        }
        else
        {
                goto end; // Not an immediate number
        }
        while (getWideChar(&wc, in) != WEOF)
        {
                switch (status)
                {
                case INT_1:
                        if (iswdigit(wc))
                        {
                                status = INT_2;
                                buffer[bufIdx++] = wc;
                        }
                        else
                        {
                                ungetWideChar(wc, in);
                                goto end; // Expect digits
                        }
                        break;
                case INT_2:
                        if (iswdigit(wc) || wc == L'_')
                        {
                                if (bufIdx < MAX_BUF_SIZE)
                                {
                                        if (wc != L'_')
                                        {
                                                buffer[bufIdx++] = wc;
                                        }
                                }
                                else
                                {
                                        outOfBuf++;
                                }
                        }
                        else
                        {
                                status = INT_END;
                                ungetWideChar(wc, in);
                                goto end;
                        }
                        break;
                case HEX_1:
                        if (wc == L'x' || wc == L'X')
                        {
                                status = HEX_2;
                                buffer[bufIdx++] = wc;
                        }
                        else
                        {
                                status = INT_2;
                                ungetWideChar(wc, in);
                        }
                        break;
                case HEX_2:
                        if (iswxdigit(wc))
                        {
                                status = HEX_3;
                                buffer[bufIdx++] = wc;
                        }
                        else
                        {
                                ungetWideChar(wc, in);
                                goto end; // Expect xdigits
                        }
                        break;
                case HEX_3:
                        if (iswxdigit(wc) || wc == L'_')
                        {
                                if (bufIdx < MAX_BUF_SIZE)
                                {
                                        if (wc != L'_')
                                        {
                                                buffer[bufIdx++] = wc;
                                        }
                                }
                                else
                                {
                                        outOfBuf++;
                                }
                        }
                        else
                        {
                                status = HEX_END;
                                ungetWideChar(wc, in);
                                goto end;
                        }
                        break;
                }
        }
        if (wc == WEOF)
        {
                if (status == INT_2)
                {
                        status = INT_END;
                }
        }
end:
        switch (status)
        {
        case INT_1:
                wprintf(ERR_EXPECT_SYMBOL("digits"));
                break;
        case HEX_2:
                wprintf(ERR_EXPECT_SYMBOL("xdigits"));
                break;
        case INT_END:
        case HEX_END:
                matched = S_IMMD;
                buffer[bufIdx] = L'\0';
                if (outOfBuf > 0)
                {
                        wprintf(WARN_OUT_OF_BUFFER(outOfBuf));
                }
                break;
        }
        return matched;
}

static SymbolType sym_Str(FILE *in, wint_t wc, wchar_t *buffer)
{
        enum
        {
                START,
                STR_1,
                STR_2,
                STR_END,
        };
        enum
        {
                MAX_BUF_SIZE = MAX_STR_LEN,
        };
        SymbolType matched = S_UNDEFINED;
        i32 bufIdx = 0;
        i32 status = START;
        i32 outOfBuf = 0;
        if (wc == L'*')
        {
                status = STR_1;
        }
        else
        {
                goto end;
        }
        while (getWideChar(&wc, in) != WEOF)
        {
                if (wc == L'\n')
                {
                        status = STR_2;
                        goto end;
                }
                switch (wc)
                {
                case L'*':
                        status = STR_END;
                        goto end;
                case L'\\':
                        sym_escape_seq(in, &bufIdx, buffer);
                        break;
                default:
                        if (bufIdx < MAX_BUF_SIZE)
                        {
                                buffer[bufIdx++] = wc;
                        }
                        else
                        {
                                outOfBuf++;
                        }
                        break;
                }
        }
end:
        switch (status)
        {
        case STR_1:
                wprintf(ERR_UNCLOSED_SYMBOL("*"));
                break;
        case STR_2:
                wprintf(ERR_INVALID_SYMBOL("\\n"));
                break;
        case STR_END:                
                if (bufIdx == 1)
                        matched = S_CHAR;
                else
                        matched = S_STR;
                buffer[bufIdx] = L'\0';
                if (outOfBuf > 0)
                {
                        wprintf(WARN_OUT_OF_BUFFER(outOfBuf));
                }
                break;
        }
        return matched;
}

static SymbolType sym_Comment(FILE *in, wint_t wc, wchar_t *buffer)
{
        enum
        {
                START,
                CMT_1,
                CMT_2,
                CMT_3,
                CMT_END
        };
        SymbolType matched = S_UNDEFINED;
        i32 status = START;
        if (wc == L'/')
        {
                status = CMT_1;
                if (getWideChar(&wc, in) != WEOF)
                {
                        if (wc == L'*')
                        {
                                status = CMT_2;
                        }
                        else
                        {
                                ungetWideChar(wc, in);
                                goto end;
                        }
                }
                else
                {
                        ungetWideChar(wc, in);
                        goto end;
                }
        }
        else
        {
                goto end;
        }
        while (getWideChar(&wc, in) != WEOF)
        {
                if (status == CMT_2)
                {
                        if (wc == L'*')
                        {
                                status = CMT_3;
                        }
                }
                else if (status == CMT_3)
                {
                        if (wc == L'/')
                        {
                                status = CMT_END;
                                goto end;
                        }
                        else
                        {
                                status = CMT_2;
                        }
                }
        }
end:
        switch (status)
        {
        case CMT_2:
                wprintf(ERR_UNCLOSED_SYMBOL("/*"));
                break;
        case CMT_3:
                wprintf(ERR_UNCLOSED_SYMBOL("/"));
                break;
        case CMT_END:
                matched = S_COMMENT;
                break;
        default:
                break;
        }
        return matched;
}

static SymbolType sym_Reserved(FILE *in, wint_t wc, wchar_t *buffer)
{
        enum
        {
                START = 1
        };
        i32 bufIdx = 0;
        i32 idx = 0;
        SymbolType type = S_RESERVED_UNDEFINED; // Init with an invalid type
        SymbolType matched = S_UNDEFINED;
        for (i32 i = START; i < RESERVED_SYM_CNT; ++i)
        {
                idx = 0;
                if (isCharAt(wc, _reservedSymTable[i], idx))
                {
                        idx++;
                        i32 length = wcslen(_reservedSymTable[i]);
                        while (idx < length)
                        {
                                buffer[bufIdx++] = wc;
                                if (getWideChar(&wc, in) != WEOF)
                                {
                                        if (isCharAt(wc, _reservedSymTable[i], idx))
                                        {
                                                idx++;
                                        }
                                        else
                                        {
                                                goto while_end; // match failed
                                        }
                                }
                                else
                                {
                                        goto while_end; // match failed
                                }
                        }
                while_end:
                        if (idx == length) // matched
                        {
                                buffer[bufIdx++] = wc;
                                if (getWideChar(&wc, in) == WEOF || iswspace(wc)) // check end
                                {
                                        type = i;
                                        goto end;
                                }
                                else // check end failed
                                {
                                        while (bufIdx > 0)
                                        {
                                                ungetWideChar(wc, in);
                                                wc = buffer[--bufIdx];
                                        }
                                        continue;
                                }
                        }
                        else // unmatched (idx < length)
                        {
                                while (bufIdx > 0)
                                {
                                        ungetWideChar(wc, in);
                                        wc = buffer[--bufIdx];
                                }
                                continue;
                        }
                }
        }
end:
        if (type != S_RESERVED_UNDEFINED)
        {
                matched = type;
        }
        return matched;
}

static SymbolType sym_Op(FILE *in, wint_t wc, wchar_t *buffer)
{
        enum
        {
                START = 1
        };
        i32 bufIdx = 0;
        i32 idx = 0;
        SymbolType type = S_OP_UNDEFINED; // Init with an invalid type
        SymbolType matched = S_UNDEFINED;
        for (i32 i = START; i < OP_SYM_CNT; ++i)
        {
                idx = 0;
                if (isCharAt(wc, _opSymTable[i], idx))
                {
                        idx++;
                        i32 length = wcslen(_opSymTable[i]);
                        while (idx < length)
                        {
                                buffer[bufIdx++] = wc;
                                if (getWideChar(&wc, in) != WEOF)
                                {
                                        if (isCharAt(wc, _opSymTable[i], idx))
                                        {
                                                idx++;
                                        }
                                        else
                                        {
                                                goto while_end; // match failed
                                        }
                                }
                                else
                                {
                                        goto while_end; // match failed
                                }
                        }
                while_end:
                        if (idx == length) // matched
                        {
                                type = i;
                                goto end;
                        }
                        else // unmatched (idx < length)
                        {
                                while (bufIdx > 0)
                                {
                                        ungetWideChar(wc, in);
                                        wc = buffer[--bufIdx];
                                }
                                continue;
                        }
                }
        }
end:
        if (type != S_OP_UNDEFINED)
        {
                matched = type;
        }
        return matched;
}

static SymbolType sym_Word(FILE *in, wint_t wc, wchar_t *buffer)
{
        enum
        {
                START,
                WORD_1,
                WORD_END,
        };
        enum
        {
                MAX_BUF_SIZE = MAX_WORD_LEN,
        };
        i32 status = START;
        i32 bufIdx = 0;
        SymbolType matched = S_UNDEFINED;
        i32 isExtern = 0; // Flag of Extern Word
        i32 outOfBuf = 0;
        if ((!iswdigit(wc) && isValidWordChar(wc)) || wc == L'@')
        {
                if (wc == L'@')
                {
                        isExtern = 1;
                }
                else
                {
                        buffer[bufIdx++] = wc;
                }
                status = WORD_1;
        }
        else
        {
                goto end;
        }
        while (getWideChar(&wc, in) != WEOF)
        {
                if (status == WORD_1)
                {
                        if (isValidWordChar(wc))
                        {
                                if (bufIdx < MAX_BUF_SIZE)
                                {
                                        buffer[bufIdx++] = wc;
                                }
                                else
                                {
                                        outOfBuf++;
                                }
                        }
                        else
                        {
                                ungetWideChar(wc, in);
                                status = WORD_END;
                                goto end;
                        }
                }
        }
        if (wc == WEOF)
        {
                if (status == WORD_1)
                {
                        status = WORD_END;
                }
        }
end:
        if (status == WORD_END)
        {
                buffer[bufIdx] = L'\0';
                SymbolType type = isExtern ? S_EXTERN_WORD : S_WORD;
                matched = type;
                if (outOfBuf > 0)
                {
                        wprintf(WARN_OUT_OF_BUFFER(outOfBuf));
                }
        }
        return matched;
}

static SymbolType sym_Cell(FILE *in, wint_t wc, wchar_t *buffer)
{
        enum
        {
                START,
                CELL_1,
                CELL_2,
                END,
        };
        static const SymbolType matchedType[2][2] =
        {
                {S_WORD_CELL, S_WORD_REG},
                {S_IMMD_CELL, S_IMMD_REG},
        };        
        SymbolType matched = S_UNDEFINED;
        i32 isReg = 0; // Flag of Register
        i32 isImmd = 0;
        i32 status = START;
        enum
        {
                MAX_BUF_SIZE = MAX_WORD_LEN + 2,
        };
        if (wc == L'#' || wc == L'%')
        {
                if (wc == L'%')
                {
                        isReg = 1;
                }
                status = CELL_1;
                if (getWideChar(&wc, in) != WEOF)
                {
                        status = CELL_2;
                        if (sym_Immd(in, wc, buffer))
                        {
                                isImmd = 1;
                                status = END;
                                goto end;
                        }
                        else if (sym_Word(in, wc, buffer))
                        {
                                isImmd = 0;
                                status = END;
                                goto end;
                        }
                        else
                        {
                                // status = CELL_2
                                goto end; // Expect "Word" or "Immd"
                        }
                }
                else
                {
                        // status = CELL_1
                        goto end; // Expect "Word" or "Immd"
                }
        }
        else
        {
                goto end;
        }
end:
        if (status == CELL_1 || status == CELL_2)
        {
                wprintf(ERR_EXPECT_SYMBOL("<Word> or <Immd>"));
        }
        else if (status == END)
        {
                matched = matchedType[isReg][isImmd];
        }
        return matched;
}

static void matched_Immd(const wchar_t buffer[], SymbolType type, DgyStack *out)
{
        i64 immd = wcstoll(buffer, NULL, 0);
        cell_t data = {.data.sint=immd, .type=CELL_LEXER_IMMD};
        dgyStackPush(out, data);
}

static void matched_Str(const wchar_t buffer[], SymbolType type, DgyStack *out)
{
        if (type == S_CHAR)
        {
                cell_t data = {.data.wchar=buffer[0], .type=CELL_LEXER_CHAR};
                dgyStackPush(out, data);
                cell_t len = {.data.sint=1, .type=CELL_FLAG_LEN};
                dgyStackPush(out, len);                
        }
        else
        {
                i32 i;
                for (i = 0; buffer[i] != L'\0' && i < MAX_STR_LEN; ++i)
                {
                        cell_t data = {
                                .data.wchar=buffer[i],
                                .type=CELL_LEXER_STR
                        };
                        dgyStackPush(out, data);
                }
                cell_t len = {.data.sint=i, .type=CELL_FLAG_LEN};
                dgyStackPush(out, len);
        }
}

static void matched_Comment(const wchar_t buffer[], SymbolType type, DgyStack *out)
{
        // Dummy
}

static void matched_Reserved(const wchar_t buffer[], SymbolType type, DgyStack *out)
{
        cell_t data = {.data.sint=type, .type=CELL_LEXER_RESERVED};
        dgyStackPush(out, data);
}

static void matched_Op(const wchar_t buffer[], SymbolType type, DgyStack *out)
{
        cell_t data = {.data.sint=type, .type=CELL_LEXER_OP};
        dgyStackPush(out, data);        
}

static void matched_Word(const wchar_t buffer[], SymbolType type, DgyStack *out)
{
        CellType cellType;
        if (type == S_WORD)
        {
                cellType = CELL_LEXER_WORD;
        }
        else if (type == S_EXTERN_WORD)
        {
                cellType = CELL_LEXER_EXTERN_WORD;
        }
        else
        {
                wprintf(L"dgy_lexer: matched_Word failed.\n");
                return;
        }
        i32 i;        
        for (i = 0; buffer[i] != L'\0' && i < MAX_WORD_LEN; ++i)
        {                
                cell_t data = {
                        .data.wchar=buffer[i],
                        .type=cellType
                };
                dgyStackPush(out, data);
        }        
        cell_t len = {.data.sint=i, .type=CELL_FLAG_LEN};
        dgyStackPush(out, len);        
}

static void matched_Cell(const wchar_t buffer[], SymbolType type, DgyStack *out)
{
        CellType cellType;
        switch (type)
        {
        case S_WORD_CELL:
                cellType = CELL_LEXER_WORD_CELL;
                break;
        case S_WORD_REG:
                cellType = CELL_LEXER_WORD_REG;
                break;
        case S_IMMD_CELL:
                cellType = CELL_LEXER_IMMD_CELL;                
                break;
        case S_IMMD_REG:
                cellType = CELL_LEXER_IMMD_REG;
                break;
        default:
                wprintf(L"dgy_lexer: matched_Cell failed.\n");
                return;
        }
        if (CELL_LEXER_WORD_CELL == cellType ||
            CELL_LEXER_WORD_REG == cellType)
        {
                i32 i;
                for (i = 0; buffer[i] != L'\0' && i < MAX_WORD_LEN; ++i)
                {                
                        cell_t data = {
                                .data.wchar=buffer[i],
                                .type=cellType
                        };
                        dgyStackPush(out, data);
                }        
                cell_t len = {.data.sint=i, .type=CELL_FLAG_LEN};
                dgyStackPush(out, len);                
        }
        else
        {
                i64 immd = wcstoll(buffer, NULL, 0);
                cell_t data = {.data.sint=immd, .type=cellType};
                dgyStackPush(out, data);                
        }
}

ErrCode dgyDoLexerOnce(FILE *in, DgyStack *out)
{
        enum
        {
                MAX_BUF_SIZE = MAX_WORD_LEN
        };
        if (!in || !out)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDoLexerOnce");
                return CODE_FAILURE;
        }
        setlocale(LC_ALL, "zh_CN.utf8"); /* Set locale */
        wint_t wc;
        wchar_t buffer[MAX_BUF_SIZE];
        for (; getWideChar(&wc, in) != WEOF;)
        {
                if (iswspace(wc))
                {
                        continue;
                }
                else
                {
                        break;
                }
        }
        if (wc == WEOF)
        {
                return CODE_FAILURE;
        }
        SymbolType matchedType = S_UNDEFINED;
        for (i32 i = 0; i < SYM_CNT; ++i)
        {
                if (S_UNDEFINED != (matchedType = _matchSymbol[i](in, wc, buffer)))
                {
                        _matchedProcess[i](buffer, matchedType, out);
                        break;
                }
        }
        if (S_UNDEFINED == matchedType)
        {
                wprintf(L"Error: Unrecognized character: '%lc'\n", wc);
                return CODE_FAILURE;
        }
        return CODE_SUCCESS;
}
