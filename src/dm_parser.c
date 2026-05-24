#include "dm_parser.h"

static void sym_escape_seq(FILE *fp, wint_t *wc, int *len);
static void sym_slash(FILE *fp, wint_t *wc);
static void sym_hash(FILE *fp, wint_t *wc);

static void sym_Immd(FILE *fp, wint_t *wc);
static void sym_Str(FILE *fp, wint_t *wc);
static void sym_Obj(FILE *fp, wint_t *wc);
static void sym_Val(FILE *fp, wint_t *wc);
static void sym_Cell(FILE *fp, wint_t *wc);
static void sym_Reserve(FILE *fp, wint_t *wc);
static void sym_Op(FILE *fp, wint_t *wc);
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
                        break;
                default:
                        break;
                }
        }
}

static void sym_hash(FILE *fp, wint_t *wc)
{
        
}

static void sym_Immd(FILE *fp, wint_t *wc)
{
        int len = 1;
        int status = 1;
        wprintf(L"Immd=%lc", *wc);
        if (*wc == L'0')
        {
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
        switch (status)
        {
        case 3:
                wprintf(ERR_EXPECT_SYMBLE("xdigits"));
                break;
        case 1:
        case 2:
        case 4:
        case 5:
        case 6:
                wprintf(L" (%d)\n", len);
                break;
        }
}

static void sym_Str(FILE *fp, wint_t *wc)
{
        int len = 1;
        int status = 1;
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
                wprintf(L"%lc (%d)\n", *wc, len);
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
                wprintf(L"%lc (%d)\n", *wc, len);
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
                }
        }

        if (ferror(fp))
        {
                if (errno == EILSEQ)
                        puts("Character encoding error while reading.");
                else
                        puts("I/O error when reading");
        }
        else if (feof(fp))
        {
                puts("End of file is reached successfully");
                code = CODE_SUCCESS;
        }
        fclose(fp);
        return code;
}
