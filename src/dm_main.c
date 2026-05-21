#include <stdio.h>

#include "dm_all.h"
#include "dm_core.h"

#define DM_UNIT_TEST

int main(int argc, char *argv[])
{
    ErrCode result;

    result = dmDo();

#ifdef DM_UNIT_TEST
    result = dmUnitTest();
#endif
    return result;
}
