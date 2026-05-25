#include "dm_parser.h"

static void sym_escape_seq(FILE *fp, wint_t *wc, int *len);
static void sym_slash(FILE *fp, wint_t *wc);
static void sym_hash(FILE *fp, wint_t *wc);
static int isReservedCharAt(wint_t wc, int *idx, int *type, int *length);
        
static void sym_Immd(FILE *fp, wint_t *wc);
static void sym_Str(FILE *fp, wint_t *wc);
static void sym_Obj(FILE *fp, wint_t *wc);
static void sym_Val(FILE *fp, wint_t *wc);
static void sym_Cell(FILE *fp, wint_t *wc);
static void sym_Reserved(FILE *fp, wint_t *wc, int *idx, int type, int length);
static void sym_Op(FILE *fp, wint_t *wc, int type, int length);
static void sym_Comment(FILE *fp, wint_t *wc);

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

static void sym_not_equal()
{
        wprintf(L"Op='/=' [12](2)\n");
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

static void sym_hash(FILE *fp, wint_t *wc)
{
}

static int isReservedCharAt(wint_t wc, int *idx, int *type, int *length)
{
        const wchar_t **symtable = ReservedSymTable;
        int matched = 0;
        if (*idx == 0)
        {
                for (int i = 0; symtable[i] != NULL; ++i)
                {
                        if (wc == symtable[i][*idx])
                        {
                                *type = i;
                                *length = wcslen(symtable[i]);
                                *idx += 1;
                                matched = 1;
                                break;
                        }
                }
        }
        else if (*idx > 0 && *idx < *length)
        {
                if (wc == symtable[*type][*idx])
                {
                        *idx += 1;
                        matched = 1;
                }
        }
        
        return matched;
}

static int isOp(wint_t wc, FILE *fp, int *type, int *length)
{
        const wchar_t **symtable = OpSymTable;
        int matched = 0;

        for (int i = 0; symtable[i] != NULL; ++i)
        {
                if (wc == symtable[i][0])
                {
                        matched = 1;
                        *type = i;
                        break;
                }
        }

        wint_t wc2;
        if (wc == L'<' || wc == L'>' || wc == L'/')
        {
                if ((wc2 = fgetwc(fp)) != WEOF)
                {
                        if (wc == L'<' && wc2 == L'=')
                        {
                                *type = BEQ; /* '<=' */
                        }
                        else if (wc == L'>' && wc2 == L'=')
                        {
                                *type = AEQ; /* '>=' */
                        }
                        else if (wc == L'/' && wc2 == L'=')
                        {
                                *type = NEQ; /* '/=' */
                        }
                        ungetwc(wc2, fp);
                }
        }
        *length = wcslen(symtable[*type]);
        return matched;
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

static void sym_Reserved(FILE *fp, wint_t *wc, int *idx, int type, int length)
{
        wprintf(L"Reserved=%lc", *wc);
        while ((*wc = fgetwc(fp)) != WEOF)
        {
                if (0 == isReservedCharAt(*wc, idx, &type, &length))
                {
                        ungetwc(*wc, fp);
                        break;
                }
                else
                {
                        wprintf(L"%lc", *wc);
                }
        }
        if (*idx == length)
        {
                wprintf(L" [%d](%d)\n", type, length);
        }
}

static void sym_Op(FILE *fp, wint_t *wc, int type, int length)
{
        wprintf(L"Op='%lc", *wc);
        for (int i = 1; i < length; ++i)
        {
                if ((*wc = fgetwc(fp)) != WEOF)
                {
                        wprintf(L"%lc", *wc);
                }
        }
        wprintf(L"' [%d](%d)\n", type, length);
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
                else if (wc == L'#')
                {
                        sym_hash(fp, &wc);
                }
                else
                {
                        int idx = 0, type, length;
                        if (isReservedCharAt(wc, &idx, &type, &length))
                        {
                                sym_Reserved(fp, &wc, &idx, type, length);
                        }
                        else if (isOp(wc, fp, &type, &length))
                        {
                                sym_Op(fp, &wc, type, length);
                        }
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
