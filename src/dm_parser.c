#include "dm_parser.h"

static int isCharAt(wchar_t wc, wchar_t *wcs, int idx);

static void sym_escape_seq(FILE *fp, wint_t *wc, int *len);
static void sym_slash(FILE *fp, wint_t *wc);
static void sym_not_equal(void);
// static void sym_hash(FILE *fp, wint_t *wc);
// static int isReservedCharAt(wint_t wc, int *idx, int *type, int *length);

static void sym_Immd(FILE *fp, wint_t *wc);
static void sym_Str(FILE *fp, wint_t *wc);
static void sym_Comment(FILE *fp, wint_t *wc);
static int sym_Reserved(FILE *fp, wint_t *wc);
static int sym_Op(FILE *fp, wint_t *wc);

// static void sym_Obj(FILE *fp, wint_t *wc);
// static void sym_Val(FILE *fp, wint_t *wc);
// static void sym_Cell(FILE *fp, wint_t *wc);

static int isCharAt(wchar_t wc, wchar_t *wcs, int idx)
{
        if (idx >= wcslen(wcs))
                return 0;
        else
                return wc == wcs[idx];
}

static void sym_escape_seq(FILE *fp, wint_t *wc, int *len)
{
        if ((*wc = fgetwc(fp)) != WEOF)
        {
                switch (*wc)
                {
                case L'n':
                        (*len)++;
                        wprintf(L"\n");
                        break;
                case L'\\':
                case L'*':
                default:
                        (*len)++;
                        wprintf(L"%lc", *wc);
                        break;
                }
        }
}

static void sym_slash(FILE *fp, wint_t *wc)
{
        if ((*wc = fgetwc(fp)) != WEOF)
        {
                switch (*wc)
                {
                case L'*':
                        sym_Comment(fp, wc);
                        break;
                case L'=':
                        sym_not_equal();
                        break;
                default:
                        break;
                }
        }
}

static void sym_not_equal(void)
{
        wprintf(L"Op='/=' [%d](%d)\n", NEQ, wcslen(OpSymTable[NEQ].symble));
}

static void sym_Immd(FILE *fp, wint_t *wc)
{
        int len = 1;
        int status = 1;
        SymbleType type = IMMD_DEC;
        wprintf(L"Immd=%lc", *wc);
        if (*wc == L'0')
        {
                type = IMMD_HEX;
                status = 2;
        }
        while ((*wc = fgetwc(fp)) != WEOF)
        {
                if (status == 1)
                {
                        if (iswdigit(*wc) || *wc == L'_')
                        {
                                len++;
                                wprintf(L"%lc", *wc);
                        }
                        else
                        {
                                status = 4;
                                ungetwc(*wc, fp);
                                goto end;
                        }
                }
                else if (status == 2)
                {
                        if (*wc == L'x' || *wc == L'X')
                        {
                                len++;
                                status = 3;
                                wprintf(L"%lc", *wc);
                        }
                        else
                        {
                                status = 4;
                                ungetwc(*wc, fp);
                                goto end;
                        }
                }
                else if (status == 3)
                {
                        if (iswxdigit(*wc))
                        {
                                len++;
                                status = 5;
                                wprintf(L"%lc", *wc);
                        }
                        else
                        {
                                ungetwc(*wc, fp);
                                goto end;
                        }
                }
                else if (status == 5)
                {
                        if (iswxdigit(*wc) || *wc == L'_')
                        {
                                len++;
                                wprintf(L"%lc", *wc);
                        }
                        else
                        {
                                status = 6;
                                ungetwc(*wc, fp);
                                goto end;
                        }
                }
        }
end:
        if (status == 3)
        {
                wprintf(ERR_EXPECT_SYMBLE("xdigits"));
        }
        else
        {
                wprintf(L" [%d](%d)\n", type, len);
        }
}

static void sym_Str(FILE *fp, wint_t *wc)
{
        int len = 1;
        int status = 1;
        SymbleType type = STR;
        wprintf(L"Str=%lc", *wc);
        while ((*wc = fgetwc(fp)) != WEOF)
        {
                if (*wc == L'\n')
                {
                        status = 3;
                        goto end;
                }
                switch (*wc)
                {
                case L'*':
                        len++;
                        status = 2;
                        goto end;
                case L'\\':
                        sym_escape_seq(fp, wc, &len);
                        break;
                default:
                        len++;
                        wprintf(L"%lc", *wc);
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
                wprintf(L"%lc [%d](%d)\n", *wc, type, len);
                break;
        case 3:
                wprintf(ERR_INVALID_SYMBLE("\\n"));
                break;
        }
}

static void sym_Comment(FILE *fp, wint_t *wc)
{
        int len = 2;
        int status = 2;
        SymbleType type = COMMENT;
        wprintf(L"Comment=/*");
        while ((*wc = fgetwc(fp)) != WEOF)
        {
                if (status == 2)
                {
                        if (*wc == L'*')
                        {
                                len++;
                                status = 3;
                        }
                        else
                        {
                                len++;
                        }
                        wprintf(L"%lc", *wc);
                        continue;
                }
                if (status == 3)
                {
                        if (*wc == L'/')
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
                        wprintf(L"%lc", *wc);
                }
        }

        switch (status)
        {
        case 2:
                wprintf(ERR_UNCLOSED_SYMBLE("/*"));
                break;
        case 3:
                wprintf(ERR_UNCLOSED_SYMBLE("/"));
                break;
        case 4:
                wprintf(L"%lc [%d](%d)\n", *wc, type, len);
                break;
        }
}

static int sym_Reserved(FILE *fp, wint_t *wc)
{
        int idx = 0;
        ReservedSymType type = RESERVED_SYM_CNT; // Init with an invalid type
        int matched = 0;

        for (int i = 0; i < RESERVED_SYM_CNT; ++i)
        {
                idx = 0;
                if (isCharAt(*wc, ReservedSymTable[i].symble, idx))
                {
                        idx++;

                        int length = wcslen(ReservedSymTable[i].symble);
                        while (idx < length)
                        {
                                if ((*wc = fgetwc(fp)) != WEOF)
                                {
                                        if (isCharAt(*wc, ReservedSymTable[i].symble, idx))
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
                                if ((*wc = fgetwc(fp)) == WEOF || iswspace(*wc)) // check end
                                {
                                        type = i;
                                        goto end;
                                }
                                else // check end failed
                                {
                                        ungetwc(*wc, fp);
                                }
                        }
                        else // unmatched (idx < length)
                        {

                                for (int i = 0; i < idx + 1; ++i)
                                {
                                        ungetwc(*wc, fp);
                                }
                        }
                }
        }
end:
        if (type != RESERVED_SYM_CNT)
        {
                wprintf(L"Reserved=%ls [%d](%d)\n", ReservedSymTable[type].symble, type, wcslen(ReservedSymTable[type].symble));
                matched = 1;
        }
        return matched;
}

static int sym_Op(FILE *fp, wint_t *wc)
{
        int idx = 0;
        OpSymType type = OP_SYM_CNT; // Init with an invalid type
        int matched = 0;

        for (int i = 0; i < OP_SYM_CNT; ++i)
        {
                idx = 0;
                if (isCharAt(*wc, OpSymTable[i].symble, idx))
                {
                        idx++;

                        int length = wcslen(OpSymTable[i].symble);
                        while (idx < length)
                        {
                                if ((*wc = fgetwc(fp)) != WEOF)
                                {
                                        if (isCharAt(*wc, OpSymTable[i].symble, idx))
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
                                if ((*wc = fgetwc(fp)) == WEOF || iswspace(*wc)) // check end
                                {
                                        type = i;
                                        goto end;
                                }
                                else // check end failed
                                {
                                        ungetwc(*wc, fp);
                                        continue;
                                }
                        }
                        else // unmatched (idx < length)
                        {

                                for (int i = 0; i < idx + 1; ++i)
                                {
                                        ungetwc(*wc, fp);
                                }
                        }
                }
        }
end:
        if (type != OP_SYM_CNT)
        {
                wprintf(L"Op='%ls' [%d](%d)\n", OpSymTable[type].symble, type, wcslen(OpSymTable[type].symble));
                matched = 1;
        }
        return matched;
}

ErrCode fdmDoLexer(const char *fname)
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
                if (wc == L'*')
                {
                        sym_Str(fp, &wc);
                }
                else if (wc == L'/')
                {
                        sym_slash(fp, &wc);
                }
                else if (iswdigit(wc))
                {
                        sym_Immd(fp, &wc);
                }
                else if (sym_Reserved(fp, &wc))
                {
                        continue;
                }
                else if (sym_Op(fp, &wc))
                {
                        continue;
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
