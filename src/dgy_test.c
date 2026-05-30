#include "dgy_test.h"

// static void test_lexer_immd(void)
// {
//         fdgyDoParser("dgy/test_lexer_immd.dgy");
// }

ErrCode dgyUnitTest(void)
{
        dgyDoParser(stdin);
        // dgyDoLexerOnce(stdin, stdout);
        return CODE_SUCCESS;
}