#include "dgy_lexer.h"

static int isCharAt(wchar_t wc, wchar_t *wcs, int idx);
static int isValidNameChar(wchar_t wc);

static void sym_escape_seq(FILE *fp, int *len);

static int sym_Immd(FILE *fp, wint_t wc);
static int sym_Str(FILE *fp, wint_t wc);
static int sym_Comment(FILE *fp, wint_t wc);
static int sym_Reserved(FILE *fp, wint_t wc);
static int sym_Op(FILE *fp, wint_t wc);
static int sym_Name(FILE *fp, wint_t wc);

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

static void sym_escape_seq(FILE *fp, int *len)
{
        wint_t wc;
        if ((wc = fgetwc(fp)) != WEOF)
        {
                switch (wc)
                {
                case L'n':
                        (*len)++;
                        wprintf(L"\n");
                        break;
                case L'\\':
                case L'*':
                default:
                        (*len)++;
                        wprintf(L"%lc", wc);
                        break;
                }
        }
}

static int sym_Immd(FILE *fp, wint_t wc)
{
        int matched = 0;
        int len = 0;
        int status = 0;
        SymbleType type = IMMD_DEC;
        if (iswdigit(wc))
        {
                len++;
                status = 1;
                wprintf(L"Immd=%lc", wc);
        }
        else
        {
                goto end;
        }
        if (wc == L'0')
        {
                type = IMMD_HEX;
                status = 2;
        }
        while ((wc = fgetwc(fp)) != WEOF)
        {
                if (status == 1)
                {
                        if (iswdigit(wc) || wc == L'_')
                        {
                                len++;
                                wprintf(L"%lc", wc);
                        }
                        else
                        {
                                status = 4;
                                ungetwc(wc, fp);
                                goto end;
                        }
                }
                else if (status == 2)
                {
                        if (wc == L'x' || wc == L'X')
                        {
                                len++;
                                status = 3;
                                wprintf(L"%lc", wc);
                        }
                        else
                        {
                                status = 4;
                                ungetwc(wc, fp);
                                goto end;
                        }
                }
                else if (status == 3)
                {
                        if (iswxdigit(wc))
                        {
                                len++;
                                status = 5;
                                wprintf(L"%lc", wc);
                        }
                        else
                        {
                                ungetwc(wc, fp);
                                goto end; // error: '0x' is not a complete number
                        }
                }
                else if (status == 5)
                {
                        if (iswxdigit(wc) || wc == L'_')
                        {
                                len++;
                                wprintf(L"%lc", wc);
                        }
                        else
                        {
                                status = 6;
                                ungetwc(wc, fp);
                                goto end;
                        }
                }
        }
end:
        if (status == 3)
        {
                wprintf(ERR_EXPECT_SYMBLE("xdigits"));
        }
        else if (status == 4 || status == 6)
        {
                matched = 1;
                wprintf(L" [%d](%d)\n", type, len);
        }
        return matched;
}

static int sym_Str(FILE *fp, wint_t wc)
{
        int matched = 0;
        int len = 0;
        int status = 0;
        SymbleType type = STR;
        if (wc == L'*')
        {
                status = 1;
                len++;
                wprintf(L"Str=%lc", wc);
        }
        else
        {
                goto end;
        }
        while ((wc = fgetwc(fp)) != WEOF)
        {
                if (wc == L'\n')
                {
                        status = 3;
                        goto end;
                }
                switch (wc)
                {
                case L'*':
                        len++;
                        status = 2;
                        goto end;
                case L'\\':
                        sym_escape_seq(fp, &len);
                        break;
                default:
                        len++;
                        wprintf(L"%lc", wc);
                        break;
                }
        }
end:
        switch (status)
        {
        case 1:
                wprintf(ERR_UNCLOSED_SYMBLE("*"));
                break;
        case 2:
                if (len == 1)
                        type = CHAR;
                wprintf(L"%lc [%d](%d)\n", wc, type, len);
                matched = 1;
                break;
        case 3:
                wprintf(ERR_INVALID_SYMBLE("\\n"));
                break;
        default:
                break;
        }
        return matched;
}

static int sym_Comment(FILE *fp, wint_t wc)
{
        int matched = 0;
        int len = 0;
        int status = 0;
        SymbleType type = COMMENT;
        if (wc == L'/')
        {
                status = 1;
                len++;
                if ((wc = fgetwc(fp)) != WEOF)
                {
                        if (wc == L'*')
                        {
                                status = 2;
                                len++;
                                wprintf(L"Comment=/*");
                        }
                        else
                        {
                                ungetwc(wc, fp);
                                goto end;
                        }
                }
                else
                {
                        ungetwc(wc, fp);
                        goto end;
                }
        }
        else
        {
                goto end;
        }
        while ((wc = fgetwc(fp)) != WEOF)
        {
                if (status == 2)
                {
                        if (wc == L'*')
                        {
                                len++;
                                status = 3;
                        }
                        else
                        {
                                len++;
                        }
                        wprintf(L"%lc", wc);
                        continue;
                }
                if (status == 3)
                {
                        if (wc == L'/')
                        {
                                len++;
                                status = 4;
                                break;
                        }
                        else
                        {
                                len++;
                                status = 2;
                        }
                        wprintf(L"%lc", wc);
                }
        }
end:
        switch (status)
        {
        case 2:
                wprintf(ERR_UNCLOSED_SYMBLE("/*"));
                break;
        case 3:
                wprintf(ERR_UNCLOSED_SYMBLE("/"));
                break;
        case 4:
                matched = 1;
                wprintf(L"%lc [%d](%d)\n", wc, type, len);
                break;
        default:
                break;
        }
        return matched;
}

static int sym_Reserved(FILE *fp, wint_t wc)
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
                                if ((wc = fgetwc(fp)) != WEOF)
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
                                if ((wc = fgetwc(fp)) == WEOF || iswspace(wc)) // check end
                                {
                                        type = i;
                                        goto end;
                                }
                                else // check end failed
                                {
                                        while (bufIdx > 0)
                                        {
                                                ungetwc(wc, fp);
                                                wc = buffer[--bufIdx];
                                        }
                                        continue;
                                }
                        }
                        else // unmatched (idx < length)
                        {
                                while (bufIdx > 0)
                                {
                                        ungetwc(wc, fp);
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
                wprintf(L"Reserved='%ls' [%d](%d)\n", ReservedSymTable[type].symble, type, wcslen(ReservedSymTable[type].symble));
                matched = 1;
        }
        return matched;
}

static int sym_Op(FILE *fp, wint_t wc)
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
                                if ((wc = fgetwc(fp)) != WEOF)
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
                                        ungetwc(wc, fp);
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
                wprintf(L"Op='%ls' [%d](%d)\n", OpSymTable[type].symble, type, wcslen(OpSymTable[type].symble));
                matched = 1;
        }
        return matched;
}

static int sym_Name(FILE *fp, wint_t wc)
{
        int status = 0;
        int len = 0;
        int matched = 0;

        if (!iswdigit(wc) && isValidNameChar(wc))
        {
                len++;
                wprintf(L"Name='%lc", wc);
                status = 1;
        }
        else
        {
                goto end;
        }
        while ((wc = fgetwc(fp)) != EOF)
        {
                if (status == 1)
                {
                        if (isValidNameChar(wc))
                        {
                                len++;
                                wprintf(L"%lc", wc);
                        }
                        else
                        {
                                ungetwc(wc, fp);
                                status = 2;
                                goto end;
                        }
                }
        }
        if (wc == EOF)
        {
                status = 2;
        }
end:
        if (status == 2)
        {
                matched = 1;
                wprintf(L"' (%d)\n", len);
        }
        return matched;
}

ErrCode fdgyDoLexer(const char *fname)
{
        ErrCode code = CODE_FAILURE;
        FILE *fp = fopen(fname, "r");
        if (!fp)
        {
                perror("File opening failed");
                return CODE_FILE_OPEN_FAIL;
        }
        wint_t wc;
        while ((wc = fgetwc(fp)) != WEOF)
        {
                if (iswspace(wc))
                {
                        continue;
                }
                else if (sym_Str(fp, wc))
                {
                        continue;
                }
                else if (sym_Comment(fp, wc))
                {
                        continue;
                }
                else if (sym_Immd(fp, wc))
                {
                        continue;
                }
                else if (sym_Reserved(fp, wc))
                {
                        continue;
                }
                else if (sym_Op(fp, wc))
                {
                        continue;
                }
                else if (sym_Name(fp, wc))
                {
                        continue;
                }
                else
                {
                        wprintf(L"Error: Unrecognized character: '%lc'\n", wc);
                }
        }
        if (ferror(fp))
        {
                if (errno == EILSEQ)
                        wprintf(L"Character encoding error while reading.\n");
                else
                        wprintf(L"I/O error when reading\n");
        }
        else if (feof(fp))
        {
                wprintf(L"End of file is reached successfully\n");
                code = CODE_SUCCESS;
        }
        fclose(fp);
        return code;
}
