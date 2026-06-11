#include "dgy_lexer.h"

static const char *_fname = NULL;
static int isCharAt(wchar_t wc, const wchar_t *wcs, int idx);
static int isValidWordChar(wchar_t wc);

static void sym_escape_seq(FILE *in, int *len, wchar_t *buffer);

static int sym_Immd(FILE *in, wint_t wc, wchar_t *out);
static int sym_Str(FILE *in, wint_t wc, wchar_t *out);
static int sym_Comment(FILE *in, wint_t wc, wchar_t *out);
static int sym_Reserved(FILE *in, wint_t wc, wchar_t *out);
static int sym_Op(FILE *in, wint_t wc, wchar_t *out);
static int sym_Word(FILE *in, wint_t wc, wchar_t *out);
static int sym_Cell(FILE *in, wint_t wc, wchar_t *out);

static wint_t getWideChar(wint_t *wc, FILE *in)
{
        *wc = fgetwc(in);
        return *wc;
}

static void ungetWideChar(wint_t wc, FILE *in)
{
        ungetwc(wc, in);
}

static int isCharAt(wchar_t wc, const wchar_t *wcs, int idx)
{
        if (idx >= wcslen(wcs))
                return 0;
        else
                return wc == wcs[idx];
}

static int isValidWordChar(wchar_t wc)
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

static void sym_escape_seq(FILE *in, int *len, wchar_t *buffer)
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

static int sym_Immd(FILE *in, wint_t wc, wchar_t *out)
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
        int matched = 0;
        wchar_t buffer[MAX_BUF_SIZE]; // 20 is the length of maximum number -2^63
        int bufIdx = 0;
        int outOfBuf = 0;
        int status = START;
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
                dgyPrintErrPos(buffer, bufIdx, _fname, 0, 0);
                break;
        case HEX_2:
                wprintf(ERR_EXPECT_SYMBOL("xdigits"));
                dgyPrintErrPos(buffer, bufIdx, _fname, 0, 0);
                break;
        case INT_END:
        case HEX_END:
                matched = 1;
                buffer[bufIdx] = L'\0';
                swprintf(out, MAX_BUF_SIZE + 2, L"%ls%lc%lc", buffer, S_IMMD, L'\0');
                if (outOfBuf > 0)
                {
                        wprintf(WARN_OUT_OF_BUFFER(outOfBuf));
                        dgyPrintErrPos(buffer, bufIdx, _fname, 0, 0);
                }
                break;
        }
        return matched;
}

static int sym_Str(FILE *in, wint_t wc, wchar_t *out)
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
        int matched = 0;
        wchar_t buffer[MAX_BUF_SIZE];
        int bufIdx = 0;
        int status = START;
        int outOfBuf = 0;
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
                dgyPrintErrPos(buffer, bufIdx, _fname, 0, 0);
                break;
        case STR_2:
                wprintf(ERR_INVALID_SYMBOL("\\n"));
                dgyPrintErrPos(buffer, bufIdx, _fname, 0, 0);
                break;
        case STR_END:
                matched = 1;
                buffer[bufIdx] = L'\0';
                SymbolType type = (bufIdx == 1) ? S_CHAR : S_STR;
                swprintf(out, MAX_BUF_SIZE + 2, L"%ls%lc%lc", buffer, type, L'\0');
                if (outOfBuf > 0)
                {
                        wprintf(WARN_OUT_OF_BUFFER(outOfBuf));
                        dgyPrintErrPos(buffer, bufIdx, _fname, 0, 0);
                }
                break;
        }
        return matched;
}

static int sym_Comment(FILE *in, wint_t wc, wchar_t *out)
{
        enum
        {
                START,
                CMT_1,
                CMT_2,
                CMT_3,
                CMT_END
        };
        int matched = 0;
        int status = START;
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
                dgyPrintErrPos(NULL, 0, _fname, 0, 0);
                break;
        case CMT_3:
                wprintf(ERR_UNCLOSED_SYMBOL("/"));
                dgyPrintErrPos(NULL, 0, _fname, 0, 0);
                break;
        case CMT_END:
                matched = 1;
                SymbolType type = S_COMMENT;
                swprintf(out, 2, L"%lc%lc", type, L'\0');
                break;
        default:
                break;
        }
        return matched;
}

static int sym_Reserved(FILE *in, wint_t wc, wchar_t *out)
{
        enum
        {
                START = 1
        };
        static size_t reservedgyaxLength = 0;
        if (reservedgyaxLength == 0)
        {
                for (int i = START; i < RESERVED_SYM_END; ++i)
                {
                        size_t len = wcslen(_reservedSymTable[i].symbol);
                        if (len > reservedgyaxLength)
                                reservedgyaxLength = len;
                }
        }
        wchar_t *buffer = malloc(reservedgyaxLength * sizeof(wchar_t));
        int bufIdx = 0;

        int idx = 0;
        ReservedSymType type = S_RESERVED_UNDEFINED; // Init with an invalid type
        int matched = 0;
        for (int i = START; i < RESERVED_SYM_END; ++i)
        {
                idx = 0;
                if (isCharAt(wc, _reservedSymTable[i].symbol, idx))
                {
                        idx++;
                        int length = wcslen(_reservedSymTable[i].symbol);
                        while (idx < length)
                        {
                                buffer[bufIdx++] = wc;
                                if (getWideChar(&wc, in) != WEOF)
                                {
                                        if (isCharAt(wc, _reservedSymTable[i].symbol, idx))
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
        free(buffer);
        if (type != S_RESERVED_UNDEFINED)
        {
                matched = 1;
                swprintf(out, 3, L"%lc%lc%lc", type, S_RESERVED, L'\0');
        }
        return matched;
}

static int sym_Op(FILE *in, wint_t wc, wchar_t *out)
{
        enum
        {
                START = 1
        };
        static size_t opMaxLength = 0;
        if (opMaxLength == 0)
        {
                for (int i = START; i < OP_SYM_END; ++i)
                {
                        size_t len = wcslen(_opSymTable[i].symbol);
                        if (len > opMaxLength)
                                opMaxLength = len;
                }
        }
        wchar_t *buffer = malloc(opMaxLength * sizeof(wchar_t));
        int bufIdx = 0;

        int idx = 0;
        OpSymType type = S_OP_UNDEFINED; // Init with an invalid type
        int matched = 0;
        for (int i = START; i < OP_SYM_END; ++i)
        {
                idx = 0;
                if (isCharAt(wc, _opSymTable[i].symbol, idx))
                {
                        idx++;
                        int length = wcslen(_opSymTable[i].symbol);
                        while (idx < length)
                        {
                                buffer[bufIdx++] = wc;
                                if (getWideChar(&wc, in) != WEOF)
                                {
                                        if (isCharAt(wc, _opSymTable[i].symbol, idx))
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
        free(buffer);
        if (type != S_OP_UNDEFINED)
        {
                matched = 1;
                swprintf(out, 3, L"%lc%lc%lc", type, S_OP, L'\0');
        }
        return matched;
}

static int sym_Word(FILE *in, wint_t wc, wchar_t *out)
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
        int status = START;
        wchar_t buffer[MAX_BUF_SIZE];
        int bufIdx = 0;
        int matched = 0;
        int isExtern = 0; // Flag of Extern Word
        int outOfBuf = 0;
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
                matched = 1;
                buffer[bufIdx] = L'\0';
                SymbolType type = isExtern ? S_EXTERN_WORD : S_WORD;
                swprintf(out, MAX_BUF_SIZE + 2, L"%ls%lc%lc", buffer, type, L'\0');                
                if (outOfBuf > 0)
                {
                        wprintf(WARN_OUT_OF_BUFFER(outOfBuf));
                        dgyPrintErrPos(buffer, bufIdx, _fname, 0, 0);
                }
        }
        return matched;
}

static int sym_Cell(FILE *in, wint_t wc, wchar_t *out)
{
        enum
        {
                START,
                CELL_1,
                CELL_2,
                END,
        };
        int matched = 0;
        int isReg = 0; // Flag of Register
        int status = START;
        enum
        {
                MAX_BUF_SIZE = MAX_WORD_LEN + 2,
        };
        wchar_t buffer[MAX_BUF_SIZE];
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
                        if (sym_Immd(in, wc, buffer) || sym_Word(in, wc, buffer))
                        {
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
                dgyPrintErrPos(NULL, 0, _fname, 0, 0);
        }
        else if (status == END)
        {
                matched = 1;
                int len = wcslen(buffer);
                buffer[len - 1] = isReg ? S_REG : S_CELL; /* Just change type */
                swprintf(out, MAX_BUF_SIZE, L"%ls", buffer);
        }
        return matched;
}

ErrCode dgyDoLexer(FILE *in, wchar_t *out, const int maxMatchedCnt)
{
        ErrCode code = CODE_SUCCESS;
        if (!in || !out)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDoLexer");
                return CODE_FAILURE;
        }
        int matchedCnt = 0;
        if (maxMatchedCnt == -1) /* If maxMatchedCnt == -1, set matchedCnt to a number smaller than -1. */
        {
                matchedCnt = -2;
        }
        for (wint_t wc; (matchedCnt < maxMatchedCnt) && (getWideChar(&wc, in) != WEOF);)
        {
                int matched = 0;
                if (iswspace(wc))
                {
                        continue;
                }
                else if (sym_Str(in, wc, out))
                {
                        matched = 1;
                }
                else if (sym_Comment(in, wc, out))
                {
                        matched = 1;
                }
                else if (sym_Immd(in, wc, out))
                {
                        matched = 1;
                }
                else if (sym_Cell(in, wc, out))
                {
                        matched = 1;
                }
                else if (sym_Reserved(in, wc, out))
                {
                        matched = 1;
                }
                else if (sym_Op(in, wc, out))
                {
                        matched = 1;
                }
                else if (sym_Word(in, wc, out))
                {
                        matched = 1;
                }
                else
                {
                        wprintf(L"Error: Unrecognized character: '%lc'\n", wc);
                        dgyPrintErrPos(NULL, 0, _fname, 0, 0);
                }
                // check matched
                if (matched && maxMatchedCnt != -1) /* If maxMatchedCnt == -1, never increment matchedCnt. */
                {
                        matchedCnt++;
                }
        }
        if (ferror(in))
        {
                if (errno == EILSEQ)
                        wprintf(L"Character encoding error while reading.\n");
                else
                        wprintf(L"I/O error when reading\n");
                code = CODE_FAILURE;
        }
        else if (feof(in))
        {
                if (maxMatchedCnt != -1 && matchedCnt < maxMatchedCnt)
                {
                        wprintf(L"dgyDoLexer: Cannot get enough matched results\n");
                        code = CODE_FAILURE;
                }
        }
        return code;
}

ErrCode fdgyDoLexer(const char *fname, wchar_t *out)
{
        _fname = fname;
        wprintf(L"%s\n", _fname);
        ErrCode code = CODE_FAILURE;
        FILE *fp = fopen(fname, "r");
        if (!fp)
        {
                perror("fdgyDoLexer: fopen() failed");
                return CODE_FAILURE;
        }
        code = dgyDoLexer(fp, out, -1);
        fclose(fp);
        return code;
}

ErrCode dgyDoLexerOnce(FILE *in, wchar_t *out)
{
        ErrCode code = CODE_FAILURE;
        code = dgyDoLexer(in, out, 1);
        return code;
}
