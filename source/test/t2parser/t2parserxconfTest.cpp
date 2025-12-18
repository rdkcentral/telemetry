#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include "t2parserxconf.h"
#include "profilexconf.h"
#include "telemetry2_0.h"
}

/*
 * When required fields are missing, processConfigurationXConf should fail.
 */
TEST(T2ParserXConf, MissingRequiredFields)
{
    ProfileXConf *profile = NULL;
    const char *json_missing = "{\"urn:settings:TelemetryProfile\": { \"telemetryProfile:name\": \"TestProfile\", \"telemetryProfile\": [] } }";
    // Missing uploadRepository:URL and schedule and no telemetryProfile entries -> failure
    EXPECT_EQ(T2ERROR_FAILURE, processConfigurationXConf((char*)json_missing, &profile));
    EXPECT_EQ((ProfileXConf*)NULL, profile);
}

/*
 * When schedule doesn't contain a '/' pattern, default schedule is used (15 minutes -> 900 sec).
 */
TEST(T2ParserXConf, DefaultScheduleUsed)
{
    ProfileXConf *profile = NULL;
    const char *json_default_schedule =
        "{"
        "  \"urn:settings:TelemetryProfile\": {"
        "    \"telemetryProfile:name\": \"DefaultSched\","
        "    \"uploadRepository:URL\": \"http://example.com/upload\","
        "    \"schedule\": \"11 4 * * *\","
        "    \"telemetryProfile\": [ {"
        "        \"header\": \"param1\","
        "        \"content\": \"alias1\","
        "        \"type\": \"/var/log/some.log\","
        "        \"pollingFrequency\": \"0\""
        "    } ]"
        "  }"
        "}";
    EXPECT_EQ(T2ERROR_SUCCESS, processConfigurationXConf((char*)json_default_schedule, &profile));
    ASSERT_NE(profile, nullptr);
    // default should be 15 minutes -> 900 seconds
    EXPECT_EQ(900, profile->reportingInterval);
    ASSERT_NE(profile->t2HTTPDest, nullptr);
    EXPECT_STREQ("http://example.com/upload", profile->t2HTTPDest->URL ? profile->t2HTTPDest->URL : "");
    // minimal cleanup (full deep-free is not attempted here)
    // tests rely on process teardown to reclaim memory in CI runs
}

/*
 * When schedule contains a slash pattern like "0/5 * * * *", getScheduleInSeconds should
 * extract the number after the slash and use it (5 minutes -> 300 seconds).
 * This test also includes a top_log.txt entry to exercise topMarker handling.
 */
TEST(T2ParserXConf, SlashScheduleAndTopLogHandling)
{
    ProfileXConf *profile = NULL;
    const char *json_slash_schedule =
        "{"
        "  \"urn:settings:TelemetryProfile\": {"
        "    \"telemetryProfile:name\": \"SlashSched\","
        "    \"uploadRepository:URL\": \"http://example.com/upload2\","
        "    \"schedule\": \"0/5 * * * *\","
        "    \"telemetryProfile\": ["
        "      {"
        "        \"header\": \"tm_top\","
        "        \"content\": \"search-term\","
        "        \"type\": \"top_log.txt\","
        "        \"pollingFrequency\": \"0\""
        "      },"
        "      {"
        "        \"header\": \"param_tr181\","
        "        \"content\": \"Device.DeviceInfo.Manufacturer\","
        "        \"type\": \"<message_bus>\","
        "        \"pollingFrequency\": \"0\""
        "      }"
        "    ]"
        "  }"
        "}";
    EXPECT_EQ(T2ERROR_SUCCESS, processConfigurationXConf((char*)json_slash_schedule, &profile));
    ASSERT_NE(profile, nullptr);
    // schedule "0/5 * * * *" should parse to 5 minutes => 300 sec
    EXPECT_EQ(300, profile->reportingInterval);
    ASSERT_NE(profile->t2HTTPDest, nullptr);
    EXPECT_STREQ("http://example.com/upload2", profile->t2HTTPDest->URL ? profile->t2HTTPDest->URL : "");
    // Ensure the profile name got set
    EXPECT_STREQ("SlashSched", profile->name ? profile->name : "");
}
TEST(T2ParserXConf, getParameter_alltestcases)
{
    const char *json =
        "{"
        "  \"urn:settings:TelemetryProfile\": {"
        "    \"telemetryProfile:name\": \"testprofile\","
        "    \"uploadRepository:URL\": \"http://localhost\","
        "    \"schedule\": \"0/15 * * * *\","
        "    \"telemetryProfile\": ["
        "      { \"header\": \"ParamName\", \"content\": \"Profile.Name\", \"type\": \"<event>\", \"pollingFrequency\": \"1\" },"
        "      { \"header\": \"ParamVersion\", \"content\": \"Profile.Version\", \"type\": \"<event>\", \"pollingFrequency\": \"1\" },"
        "      { \"header\": \"ParamDescription\", \"content\": \"Profile.Description\", \"type\": \"<event>\", \"pollingFrequency\": \"1\" },"
        "      { \"header\": \"ParamProtocol\", \"content\": \"Profile.Protocol\", \"type\": \"<event>\", \"pollingFrequency\": \"1\" },"
        "      { \"header\": \"ParamRootName\", \"content\": \"Profile.RootName\", \"type\": \"<event>\", \"pollingFrequency\": \"1\" },"
        "      { \"header\": \"ParamEncodingType\", \"content\": \"Profile.EncodingType\", \"type\": \"<event>\", \"pollingFrequency\": \"1\" },"
        "      { \"header\": \"ParamReportingInterval\", \"content\": \"Profile.ReportingInterval\", \"type\": \"<event>\", \"pollingFrequency\": \"1\" },"
        "      { \"header\": \"ParamTimeReference\", \"content\": \"Profile.TimeReference\", \"type\": \"<event>\", \"pollingFrequency\": \"1\" },"
        "      { \"header\": \"ParamActivationTimeOut\", \"content\": \"Profile.ActivationTimeOut\", \"type\": \"<event>\", \"pollingFrequency\": \"1\" },"
        "      { \"header\": \"ParamUnknown\", \"content\": \"Profile.Unknown\", \"type\": \"<event>\", \"pollingFrequency\": \"1\" }"
        "    ]"
        "  }"
        "}";

    ProfileXConf *profile = NULL;
    int ret = processConfigurationXConf((char*)json, &profile);
    assert(ret == 0);
    assert(profile != NULL);
}
