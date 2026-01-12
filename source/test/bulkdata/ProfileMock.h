#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "telemetry2_0.h"
#include "test/bulkdata/ProfileMock.h"

ProfileMock* g_profileMock = nullptr;

// ... existing bridge(s) ...

extern "C" T2ERROR prepareJSONReport(cJSON *jsonObj, char **reportBuff)
{
    if (g_profileMock)
        return g_profileMock->prepareJSONReport(jsonObj, reportBuff);
    return T2ERROR_FAILURE; // Or call real prepareJSONReport if you like
}
