#include <cjson/cJSON.h>
#include "telemetry2_0.h"
#include "test/bulkdata/ProfileMock.h"
class ProfileInterface
{
public:
    virtual ~ProfileInterface() = default;
    virtual char* cJSON_PrintUnformatted(const cJSON* item) = 0;
    virtual T2ERROR prepareJSONReport(cJSON *jsonObj, char **reportBuff) = 0; // <-- Add this line!
};

class ProfileMock : public ProfileInterface
{
public:
    virtual ~ProfileMock() {}
    MOCK_METHOD(char*, cJSON_PrintUnformatted, (const cJSON* item), (override));
    MOCK_METHOD(T2ERROR, prepareJSONReport, (cJSON *jsonObj, char **reportBuff), (override)); // <-- Add this line!
};

// Global pointer (already present)
extern ProfileMock* g_profileMock;
