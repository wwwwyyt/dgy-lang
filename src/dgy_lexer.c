#include "dgy_lexer.h"

static int isCharAt(wchar_t wc, wchar_t *wcs, int idx);
static int isValidNameChar(wchar_t wc);

static void sym_escape_seq(FILE *in, int *len, wchar_t *buffer);

static int sym_Immd(FILE *in, wint_t wc, wchar_t *out);
static int sym_Str(FILE *in, wint_t wc, wchar_t *out);
static int sym_Comment(FILE *in, wint_t wc, wchar_t *out);
static int sym_Reserved(FILE *in, wint_t wc, wchar_t *out);
static int sym_Op(FILE *in, wint_t wc, wchar_t *out);
static int sym_Name(FILE *in, wint_t wc, wchar_t *out);

static int isCharAt(wchar_t wc, wchar_t *wcs, int idx)
{
        if (idx >= wcslen(wcs))
                return 0;
        else
                return wc == wcs[idx];
}

static int isValidNameChar(wchar_t wc)
{
        if ((wc >= 0x4E00 && wc <= 0x62FF) ||
            (wc >= 0x7700 && wc <= 0x9FFF)) // Chinese character
                return 1;
        else if (iswalpha(wc)) // English character
                return 1;
        else if (iswdigit(wc)) // 0-9
                return 1;
        else
                return 0;
}

static void sym_escape_seq(FILE *in, int *len, wchar_t *buffer)
{
        wint_t wc;
        if ((wc = fgetwc(in)) != WEOF)
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
                POS_1,
                POS_END,
                NEG_1,
                NEG_2,
                NEG_END,
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
        if (wc != L'0' && iswdigit(wc))
        {
                status = POS_1; // positive integer
                buffer[bufIdx++] = wc;
        }
        else if (wc == L'-')
        {
                status = NEG_1; // negative integer
                buffer[bufIdx++] = wc;
        }
        else if (wc == L'0')
        {
                status = HEX_1; // hexadecimal integer
                buffer[bufIdx++] = wc;
        }
        else
        {
                goto end;
        }
        while ((wc = fgetwc(in)) != WEOF)
        {
                switch (status)
                {
                case POS_1:
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
                                status = POS_END;
                                ungetwc(wc, in);
                                goto end;
                        }
                        break;
                case NEG_1:
                        if (wc != L'0' && iswdigit(wc))
                        {
                                status = NEG_2;
                                buffer[bufIdx++] = wc;
                        }
                        else
                        {
                                ungetwc(wc, in);
                                goto end; // expect digits
                        }
                        break;
                case NEG_2:
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
                                status = NEG_END;
                                ungetwc(wc, in);
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
                                status = POS_END;
                                ungetwc(wc, in);
                                goto end;
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
                                ungetwc(wc, in);
                                goto end; // expect xdigit
                        }
                        break;
                case HEX_3:
                        if (iswxdigit(wc) || wc == L'_')
                        {
                                if (bufIdx < MAX_BUF_SIZE && wc != L'_')
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
                                ungetwc(wc, in);
                                goto end;
                        }
                        break;
                }
        }

end:
        switch (status)
        {
        case NEG_1:
                wprintf(ERR_EXPECT_SYMBLE("digits"));
                break;
        case HEX_2:
                wprintf(ERR_EXPECT_SYMBLE("xdigits"));
                break;
        case POS_END:
        case NEG_END:
        case HEX_END:
                matched = 1;
                buffer[bufIdx] = L'\0';
                swprintf(out, MAX_BUF_SIZE, L"%ls%lc%lc", buffer, S_IMMD, L'\0');
                if (outOfBuf > 0)
                {
                        wprintf(WARN_OUT_OF_BUFFER(outOfBuf));
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
        while ((wc = fgetwc(in)) != WEOF)
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
                wprintf(ERR_UNCLOSED_SYMBLE("*"));
                break;
        case STR_2:
                wprintf(ERR_INVALID_SYMBLE("\\n"));
                break;
        case STR_END:
                matched = 1;
                buffer[bufIdx] = L'\0';
                swprintf(out, MAX_BUF_SIZE, L"%ls%lc%lc", buffer, S_STR, L'\0');
                if (outOfBuf > 0)
                {
                        wprintf(WARN_OUT_OF_BUFFER(outOfBuf));
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
                if ((wc = fgetwc(in)) != WEOF)
                {
                        if (wc == L'*')
                        {
                                status = CMT_2;
                        }
                        else
                        {
                                ungetwc(wc, in);
                                goto end;
                        }
                }
                else
                {
                        ungetwc(wc, in);
                        goto end;
                }
        }
        else
        {
                goto end;
        }
        while ((wc = fgetwc(in)) != WEOF)
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
                wprintf(ERR_UNCLOSED_SYMBLE("/*"));
                break;
        case CMT_3:
                wprintf(ERR_UNCLOSED_SYMBLE("/"));
                break;
        case CMT_END:
                matched = 1;
                break;
        default:
                break;
        }
        return matched;
}

static int sym_Reserved(FILE *in, wint_t wc, wchar_t *out)
{
        static size_t reservedgyaxLength = 0;
        if (reservedgyaxLength == 0)
        {
                for (int i = 0; i < OP_SYM_CNT; ++i)
                {
                        size_t len = wcslen(ReservedSymTable[i].symble);
                        if (len > reservedgyaxLength)
                                reservedgyaxLength = len;
                }
        }
        wchar_t *buffer = malloc(reservedgyaxLength * sizeof(wchar_t));
        int bufIdx = 0;

        int idx = 0;
        ReservedSymType type = RESERVED_SYM_CNT; // Init with an invalid type
        int matched = 0;
        for (int i = 0; i < RESERVED_SYM_CNT; ++i)
        {
                idx = 0;
                if (isCharAt(wc, ReservedSymTable[i].symble, idx))
                {
                        idx++;
                        int length = wcslen(ReservedSymTable[i].symble);
                        while (idx < length)
                        {
                                buffer[bufIdx++] = wc;
                                if ((wc = fgetwc(in)) != WEOF)
                                {
                                        if (isCharAt(wc, ReservedSymTable[i].symble, idx))
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
                                if ((wc = fgetwc(in)) == WEOF || iswspace(wc)) // check end
                                {
                                        type = i;
                                        goto end;
                                }
                                else // check end failed
                                {
                                        while (bufIdx > 0)
                                        {
                                                ungetwc(wc, in);
                                                wc = buffer[--bufIdx];
                                        }
                                        continue;
                                }
                        }
                        else // unmatched (idx < length)
                        {
                                while (bufIdx > 0)
                                {
                                        ungetwc(wc, in);
                                        wc = buffer[--bufIdx];
                                }
                                continue;
                        }
                }
        }
end:
        free(buffer);
        if (type != RESERVED_SYM_CNT)
        {
                matched = 1;
                swprintf(out, 3, L"%lc%lc%lc", type, S_RESERVED, L'\0');
        }
        return matched;
}

static int sym_Op(FILE *in, wint_t wc, wchar_t *out)
{
        static size_t opMaxLength = 0;
        if (opMaxLength == 0)
        {
                for (int i = 0; i < OP_SYM_CNT; ++i)
                {
                        size_t len = wcslen(OpSymTable[i].symble);
                        if (len > opMaxLength)
                                opMaxLength = len;
                }
        }
        wchar_t *buffer = malloc(opMaxLength * sizeof(wchar_t));
        int bufIdx = 0;

        int idx = 0;
        OpSymType type = OP_SYM_CNT; // Init with an invalid type
        int matched = 0;
        for (int i = 0; i < OP_SYM_CNT; ++i)
        {
                idx = 0;
                if (isCharAt(wc, OpSymTable[i].symble, idx))
                {
                        idx++;
                        int length = wcslen(OpSymTable[i].symble);
                        while (idx < length)
                        {
                                buffer[bufIdx++] = wc;
                                if ((wc = fgetwc(in)) != WEOF)
                                {
                                        if (isCharAt(wc, OpSymTable[i].symble, idx))
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
                                        ungetwc(wc, in);
                                        wc = buffer[--bufIdx];
                                }
                                continue;
                        }
                }
        }
end:
        free(buffer);
        if (type != OP_SYM_CNT)
        {
                matched = 1;
                swprintf(out, 3, L"%lc%lc%lc", type, S_OP, L'\0');
        }
        return matched;
}

static int sym_Name(FILE *in, wint_t wc, wchar_t *out)
{
        enum
        {
                START,
                NAME_1,
                NAME_END,
        };
        enum
        {
                MAX_BUF_SIZE = MAX_NAME_LEN,
        };
        int status = START;
        wchar_t buffer[MAX_BUF_SIZE];
        int bufIdx = 0;
        int matched = 0;
        int outOfBuf = 0;
        if (!iswdigit(wc) && isValidNameChar(wc))
        {

                buffer[bufIdx++] = wc;
                status = NAME_1;
        }
        else
        {
                goto end;
        }
        while ((wc = fgetwc(in)) != EOF)
        {
                if (status == NAME_1)
                {
                        if (isValidNameChar(wc))
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
                                ungetwc(wc, in);
                                status = NAME_END;
                                goto end;
                        }
                }
        }
        if (wc == EOF)
        {
                status = NAME_END;
        }
end:
        if (status == NAME_END)
        {
                matched = 1;
                buffer[bufIdx] = L'\0';
                swprintf(out, MAX_BUF_SIZE, L"%ls%lc%lc", buffer, S_NAME, L'\0');
                if (outOfBuf > 0)
                {
                        wprintf(WARN_OUT_OF_BUFFER(outOfBuf));
                }
        }
        return matched;
}

ErrCode dgyDoLexer(FILE *in, wchar_t *out, const int maxMatchedCnt)
{
        ErrCode code = CODE_FAILURE;
        if (!in)
        {
                perror("dgyDoLexer");
                return CODE_NULLPTR;
        }
        int matchedCnt = 0;
        if (maxMatchedCnt == -1) /* If maxMatchedCnt == -1, set matchedCnt to a number smaller than -1. */
        {
                matchedCnt = -2;
        }
        for (wint_t wc; (matchedCnt < maxMatchedCnt) && ((wc = fgetwc(in)) != WEOF);)
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
                else if (sym_Reserved(in, wc, out))
                {
                        matched = 1;
                }
                else if (sym_Op(in, wc, out))
                {
                        matched = 1;
                }
                else if (sym_Name(in, wc, out))
                {
                        matched = 1;
                }
                else
                {
                        wprintf(L"\nError: Unrecognized character: '%lc'\n", wc);
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
                        wprintf(L"\nCharacter encoding error while reading.\n");
                else
                        wprintf(L"\nI/O error when reading\n");
        }
        else if (feof(in))
        {
                wprintf(L"\nEnd of stream is reached successfully\n");
                code = CODE_SUCCESS;
        }
        return code;
}

ErrCode fdgyDoLexer(const char *fname, wchar_t *out)
{
        ErrCode code = CODE_FAILURE;
        FILE *fp = fopen(fname, "r");
        if (!fp)
        {
                perror("fdgyDoLexer");
                return CODE_FILE_OPEN_FAIL;
        }
        code = dgyDoLexer(fp, out, -1);
        fclose(fp);
        return code;
}

ErrCode dgyDoLexerOnce(FILE *in, wchar_t *out)
{
        ErrCode code = CODE_FAILURE;
        if (!in)
        {
                perror("dgyDoLexerOnce");
                return CODE_NULLPTR;
        }
        code = dgyDoLexer(in, out, 1);
        return code;
}