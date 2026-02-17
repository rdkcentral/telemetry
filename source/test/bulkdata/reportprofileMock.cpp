#include <stdbool.h>
#include <cjson/cJSON.h>
#include "test/bulkdata/reportprofileMock.h"



// Mock Method

extern "C" bool __wrap_isRbusEnabled()
{
    if (!g_reportprofileMock)
    {
        return false;
    }
    return g_reportprofileMock->isRbusEnabled();
}

