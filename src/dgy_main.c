#include <stdio.h>

#include "dgy_all.h"
#include "dgy_core.h"
#include "dgy_test.h"

#define dgy_UNIT_TEST

int main(int argc, char *argv[])
{
        ErrCode result;

        result = dgyDo();

#ifdef dgy_UNIT_TEST
        result = dgyUnitTest();
#endif
        dgyQuit();
        return result;
}
