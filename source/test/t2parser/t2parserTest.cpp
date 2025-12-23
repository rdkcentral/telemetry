/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "gtest/gtest.h"
#include <iostream>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

using namespace std;

extern "C" {
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>

#include <stdint.h>
#include <limits.h>
#include <msgpack.h>
#include <cjson/cJSON.h>

#include <bulkdata/reportprofiles.h>
#include <bulkdata/profilexconf.h>
#include <bulkdata/profile.h>
#include <utils/t2common.h>
#include <xconf-client/xconfclient.h>
#include <t2parser/t2parser.h>
#include <t2parser/t2parserxconf.h>
#include <telemetry2_0.h>
#include <ccspinterface/busInterface.h>
#include <glib.h>
#include <glib/gi18n.h>

sigset_t blocking_signal;

}

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "t2parserMock.h"
#include "test/mocks/rdklogMock.h"
#include "test/mocks/rbusMock.h"

extern "C" {
    // tell C++ about the C function defined in t2parser.c
    T2ERROR verifyTriggerCondition(cJSON *jprofileTriggerCondition);
    T2ERROR addTriggerCondition(Profile *profile, cJSON *jprofileTriggerCondition);
    T2ERROR encodingSet(Profile* profile, cJSON *jprofileEncodingType, cJSON *jprofileJSONReportFormat, cJSON *jprofileJSONReportTimestamp);
    T2ERROR protocolSet (Profile *profile, cJSON *jprofileProtocol, cJSON *jprofileHTTPURL, cJSON *jprofileHTTPRequestURIParameter, int ThisprofileHTTPRequestURIParameter_count, cJSON *jprofileRBUSMethodName, cJSON *jprofileRBUSMethodParamArr, int rbusMethodParamArrCount);
typedef T2ERROR (*AddParameterFunc)(
    Profile *profile,
    const char *name,
    const char *ref,
    const char *fileName,
    int skipFreq,
    int firstSeekFromEOF,
    const char *ptype,
    const char *use,
    bool ReportEmpty,
    reportTimestampFormat reportTimestamp,
    bool trim,
    const char *regex
);

AddParameterFunc getAddParameterCallback(void);

}
T2parserMock *m_t2parserMock = NULL;
rdklogMock *m_rdklogMock = NULL;
rbusMock *g_rbusMock = NULL;

TEST(PROCESSXCONFCONFIGURATION, TEST_NULL_INVALID)
{
     ProfileXConf *profile = 0;
     fstream new_file;
     char* data = NULL;
     new_file.open("xconfInputfile.txt", ios::in);

     if (new_file.is_open()) {
        string sa;
// 1st case schedule NULL
	getline(new_file, sa);
	cout << sa << endl;
	int len = sa.length();
        data = new char[len + 1];
	strcpy(data, sa.c_str());
	cout << data << endl;
        EXPECT_EQ(T2ERROR_FAILURE, processConfigurationXConf(data, &profile));
        delete[] data;
// 2nd case ProfileName NULL	
        getline(new_file, sa);

	cout << sa << endl;
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        cout << data << endl;
	EXPECT_EQ(T2ERROR_FAILURE, processConfigurationXConf(data, &profile));
	delete[] data;
// 3rd case URL NULL
	getline(new_file, sa);

        cout << sa << endl;
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        cout << data << endl;
        EXPECT_EQ(T2ERROR_FAILURE, processConfigurationXConf(data, &profile));
	delete[] data;
// 4th case Profiledata NULL
	getline(new_file, sa);

        cout << sa << endl;
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        cout << data << endl;
        EXPECT_EQ(T2ERROR_FAILURE, processConfigurationXConf(data, &profile));
	delete[] data;

// 5th case Working case
        getline(new_file, sa);

        cout << sa << endl;
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        cout << data << endl;
        EXPECT_EQ(T2ERROR_SUCCESS, processConfigurationXConf(data, &profile));
        delete[] data;
     }
     new_file.close();
}

TEST(PROCESSCONFIGURATION_CJSON, TEST_NULL_INVALID_PARAM)
{
    Profile *profile = 0;
    fstream new_file;
    string sa;
    int len = 0;
    new_file.open("rpInputfile.txt", ios::in);
    char* data = NULL;
    if (new_file.is_open()) {
	getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //Profilename NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, NULL, "hash1", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //Protocol NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile2", "hash2", &profile));
	delete[] data;
        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //Encoding NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile3", "hash3", &profile));
        delete[] data;

        getline(new_file, sa); 
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //Parameter NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile4", "hash4", &profile));
        delete[] data;

       	getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //HASH NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile5", NULL, &profile));
	delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //RI<ML NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile6", "hash6", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
	//!RI!TC
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile7", "hash7", &profile));
        delete[] data;
       //HTTP PROTOCOL
	getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile8", "hash8", &profile));
	delete[] data;

       //Protocol HTTP
       getline(new_file, sa);
       len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile9", "hash9", &profile));
	delete[] data;

	getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile10", "hash10", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //RBUS_METHOD_PARAM_NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile11", "hash11", &profile));
        delete[] data;

       getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //Method not present
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile12", "hash12", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
	//Unsupported protocol
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile13", "hash13", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
	 //ReportTimeformat NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile14", "hash14", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //RI_GT_AT_NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile15", "hash15", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
	//TC type
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile16", "hash16", &profile));
	delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
	//Type!=datamodel
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile17", "hash17", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
	//operator NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile18", "hash18", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
	//operator invalid
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile19", "hash19", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //threshold
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile20", "hash20", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        //Ref NULL
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile21", "hash21", &profile));
        delete[] data;

        getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
	//Ref Invalid
        EXPECT_EQ(T2ERROR_FAILURE,  processConfiguration(&data, "RDKB_Profile22", "hash22", &profile));
	delete[] data;
        sleep(2);
	getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
	//datamodel parameter reference is mandatory
	EXPECT_EQ(T2ERROR_SUCCESS,  processConfiguration(&data, "RDKB_Profile23", "hash23", &profile));
	delete[] data;

	//type is invalid for parameters
	getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        EXPECT_EQ(T2ERROR_SUCCESS,  processConfiguration(&data, "RDKB_Profile24", "hash24", &profile));
        delete[] data;

	//working case
	getline(new_file, sa);
        len = sa.length();
        data = new char[len + 1];
        strcpy(data, sa.c_str());
        EXPECT_EQ(T2ERROR_SUCCESS,  processConfiguration(&data, "RDKB_Profile25", "hash25", &profile));
        delete[] data;
    }
    new_file.close();
}

//MessagePack Testing

//When Protocol is not given in the profile
TEST(PROCESSCONFIGURATION_MSGPACK, PROTOCOL_NULL)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDGldmFsdWXfAAAADaROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGsRW5jb2RpbmdUeXBlpEpTT06xUmVwb3J0aW5nSW50ZXJ2YWw8sUFjdGl2YXRpb25UaW1lb3V0zQ4Qq0dlbmVyYXRlTm93wqhSb290TmFtZapGUjJfVVNfVEMyrVRpbWVSZWZlcmVuY2W0MjAyMi0xMi0xOVQwOTozMzo1NlqpUGFyYW1ldGVy3QAAAAPfAAAABKR0eXBlqWRhdGFNb2RlbKRuYW1lplVQVElNRalyZWZlcmVuY2W4RGV2aWNlLkRldmljZUluZm8uVXBUaW1lo3VzZahhYnNvbHV0Zd8AAAAEpHR5cGWlZXZlbnSpZXZlbnROYW1lr1VTRURfTUVNMV9zcGxpdKljb21wb25lbnSmc3lzaW50o3VzZahhYnNvbHV0Zd8AAAAFpHR5cGWkZ3JlcKZtYXJrZXLZIlNZU19JTkZPX0NyYXNoUG9ydGFsVXBsb2FkX3N1Y2Nlc3Omc2VhcmNosVN1Y2Nlc3MgdXBsb2FkaW5np2xvZ0ZpbGWsY29yZV9sb2cudHh0o3VzZaVjb3VudLRSZXBvcnRpbmdBZGp1c3RtZW50c98AAAADrlJlcG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zScQpEhUVFDfAAAABKNVUky7aHR0cHM6Ly9zdGJydGwucjUzLnhjYWwudHYvq0NvbXByZXNzaW9upE5vbmWmTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Encoding is not given in the profile
TEST(PROCESSCONFIGURATION_MSGPACK, ENCODING_NULL)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDKldmFsdWXfAAAADaROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGoUHJvdG9jb2ykSFRUULFSZXBvcnRpbmdJbnRlcnZhbDyxQWN0aXZhdGlvblRpbWVvdXTNDhCrR2VuZXJhdGVOb3fCqFJvb3ROYW1lqkZSMl9VU19UQzKtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTE5VDA5OjMzOjU2WqlQYXJhbWV0ZXLdAAAAA98AAAAEpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRl3wAAAASkdHlwZaVldmVudKlldmVudE5hbWWvVVNFRF9NRU0xX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRl3wAAAAWkdHlwZaRncmVwpm1hcmtlctkiU1lTX0lORk9fQ3Jhc2hQb3J0YWxVcGxvYWRfc3VjY2Vzc6ZzZWFyY2ixU3VjY2VzcyB1cGxvYWRpbmenbG9nRmlsZaxjb3JlX2xvZy50eHSjdXNlpWNvdW50tFJlcG9ydGluZ0FkanVzdG1lbnRz3wAAAAOuUmVwb3J0T25VcGRhdGXCtkZpcnN0UmVwb3J0aW5nSW50ZXJ2YWwPsE1heFVwbG9hZExhdGVuY3nNw1CkSFRUUN8AAAAEo1VSTLtodHRwczovL3N0YnJ0bC5yNTMueGNhbC50di+rQ29tcHJlc3Npb26kTm9uZaZNZXRob2SkUE9TVLNSZXF1ZXN0VVJJUGFyYW1ldGVy3QAAAAHfAAAAAqROYW1lqnJlcG9ydE5hbWWpUmVmZXJlbmNlrFByb2ZpbGUuTmFtZaxKU09ORW5jb2RpbmffAAAAAqxSZXBvcnRGb3JtYXStTmFtZVZhbHVlUGFpcq9SZXBvcnRUaW1lc3RhbXCkTm9uZQ==";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Parameter is not given in the profile
TEST(PROCESSCONFIGURATION_MSGPACK, PARAMETER_NULL)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWSUKRLQ19Qcm9maWxlpGhhc2iSGFzaDOldmFsdWXfAAAADAROYW1lrFJESOJfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGOUHJvdG9jb2yKSFRUUKXFbmNvZGluZ1R5cGWKSINPTrFSZXBvcnRpbmdJbnRlcnZhbB6xQWN0aXZhdGlvblRpbWVvdXTNDhCrR2VuZXJhdGVOb3fCqFJvb3ROYW1lqkZSMI9VU19UQZKtVGItZVJIZmVyZW5jZbQyMDIyLTEYLTESVDA5OjMzOjU2WRSZXBvcnRpbmdBZGp1c3RtZW50c98AAAADriJicG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zTqYpEhUVFDfAAAABKNYUKу7aHR0cHM6Ly9zdGJydGwucjUzLnhjYWwudHYvgONvbXByZXNzaW9upE5vbmwmTWV0aG9kpFBPU1SZUmVxdWVzdFVSSVBhcmFtZXRIct0AAAAB3wAAAAKKTmFtZapyZXBvcnROYW1lqVJIZmVyZW5jZaxQcm9maWxILk5hbWWsSINPTKVUY29kaW5n3wAAAAKsUmVwb3JORm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3JOVGIZXNOYW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Max Latency is greater than Reporting interval 
TEST(PROCESSCONFIGURATION_MSGPACK, RI_ML_NULL)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAACpG5hbWWsUkRLQl9Qcm9maWxlpXZhbHVl3wAAAA6kTmFtZaxSREtCX1Byb2ZpbGWrRGVzY3JpcHRpb26sUkRLQl9Qcm9maWxlp1ZlcnNpb26jMC4xqFByb3RvY29spEhUVFCsRW5jb2RpbmdUeXBlpEpTT06xUmVwb3J0aW5nSW50ZXJ2YWwesUFjdGl2YXRpb25UaW1lb3V0zQ4Qq0dlbmVyYXRlTm93wqhSb290TmFtZapGUjJfVVNfVEMyrVRpbWVSZWZlcmVuY2W0MjAyMi0xMi0xOVQwOTozMzo1NlqpUGFyYW1ldGVy3QAAAAPfAAAABKR0eXBlqWRhdGFNb2RlbKRuYW1lplVQVElNRalyZWZlcmVuY2W4RGV2aWNlLkRldmljZUluZm8uVXBUaW1lo3VzZahhYnNvbHV0Zd8AAAAEpHR5cGWlZXZlbnSpZXZlbnROYW1lr1VTRURfTUVNMV9zcGxpdKljb21wb25lbnSmc3lzaW50o3VzZahhYnNvbHV0Zd8AAAAFpHR5cGWkZ3JlcKZtYXJrZXLZIlNZU19JTkZPX0NyYXNoUG9ydGFsVXBsb2FkX3N1Y2Nlc3Omc2VhcmNosVN1Y2Nlc3MgdXBsb2FkaW5np2xvZ0ZpbGWsY29yZV9sb2cudHh0o3VzZaVjb3VudLRSZXBvcnRpbmdBZGp1c3RtZW50c98AAAADrlJlcG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zcNQpEhUVFDfAAAABKNVUky7aHR0cHM6Ly9zdGJydGwucjUzLnhjYWwudHYvq0NvbXByZXNzaW9upE5vbmWmTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When TriggerCondition is not given Reporting interval is mandatory
TEST(PROCESSCONFIGURATION_MSGPACK, TC_RI_MANDATORY)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char* data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDaldmFsdWXfAAAADaROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGoUHJvdG9jb2ykSFRUUKxFbmNvZGluZ1R5cGWkSlNPTrFBY3RpdmF0aW9uVGltZW91dM0OEKtHZW5lcmF0ZU5vd8KoUm9vdE5hbWWqRlIyX1VTX1RDMq1UaW1lUmVmZXJlbmNltDIwMjItMTItMTlUMDk6MzM6NTZaqVBhcmFtZXRlct0AAAAD3wAAAASkdHlwZalkYXRhTW9kZWykbmFtZaZVUFRJTUWpcmVmZXJlbmNluERldmljZS5EZXZpY2VJbmZvLlVwVGltZaN1c2WoYWJzb2x1dGXfAAAABKR0eXBlpWV2ZW50qWV2ZW50TmFtZa9VU0VEX01FTTFfc3BsaXSpY29tcG9uZW50pnN5c2ludKN1c2WoYWJzb2x1dGXfAAAABaR0eXBlpGdyZXCmbWFya2Vy2SJTWVNfSU5GT19DcmFzaFBvcnRhbFVwbG9hZF9zdWNjZXNzpnNlYXJjaLFTdWNjZXNzIHVwbG9hZGluZ6dsb2dGaWxlrGNvcmVfbG9nLnR4dKN1c2WlY291bnS0UmVwb3J0aW5nQWRqdXN0bWVudHPfAAAAA65SZXBvcnRPblVwZGF0ZcK2Rmlyc3RSZXBvcnRpbmdJbnRlcnZhbA+wTWF4VXBsb2FkTGF0ZW5jec06mKRIVFRQ3wAAAASjVVJMu2h0dHBzOi8vc3RicnRsLnI1My54Y2FsLnR2L6tDb21wcmVzc2lvbqROb25lpk1ldGhvZKRQT1NUs1JlcXVlc3RVUklQYXJhbWV0ZXLdAAAAAd8AAAACpE5hbWWqcmVwb3J0TmFtZalSZWZlcmVuY2WsUHJvZmlsZS5OYW1lrEpTT05FbmNvZGluZ98AAAACrFJlcG9ydEZvcm1hdK1OYW1lVmFsdWVQYWlyr1JlcG9ydFRpbWVzdGFtcKROb25l";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When HTTP Protocol URL is not given
TEST(PROCESSCONFIGURATION_MSGPACK, HTTP_URL_NULL)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char* data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDaldmFsdWXfAAAADqROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGoUHJvdG9jb2ykSFRUUKxFbmNvZGluZ1R5cGWkSlNPTrFSZXBvcnRpbmdJbnRlcnZhbDyxQWN0aXZhdGlvblRpbWVvdXTNDhCrR2VuZXJhdGVOb3fCqFJvb3ROYW1lqkZSMl9VU19UQzKtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTE5VDA5OjMzOjU2WqlQYXJhbWV0ZXLdAAAAA98AAAAEpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRl3wAAAASkdHlwZaVldmVudKlldmVudE5hbWWvVVNFRF9NRU0xX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRl3wAAAAWkdHlwZaRncmVwpm1hcmtlctkiU1lTX0lORk9fQ3Jhc2hQb3J0YWxVcGxvYWRfc3VjY2Vzc6ZzZWFyY2ixU3VjY2VzcyB1cGxvYWRpbmenbG9nRmlsZaxjb3JlX2xvZy50eHSjdXNlpWNvdW50tFJlcG9ydGluZ0FkanVzdG1lbnRz3wAAAAOuUmVwb3J0T25VcGRhdGXCtkZpcnN0UmVwb3J0aW5nSW50ZXJ2YWwPsE1heFVwbG9hZExhdGVuY3nNw1CkSFRUUN8AAAADq0NvbXByZXNzaW9upE5vbmWmTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

TEST(PROCESSCONFIGURATION_MSGPACK, COMPRESSION_NULL)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char* data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDeldmFsdWXfAAAADqROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGoUHJvdG9jb2ykSFRUUKxFbmNvZGluZ1R5cGWkSlNPTrFSZXBvcnRpbmdJbnRlcnZhbDyxQWN0aXZhdGlvblRpbWVvdXTNDhCrR2VuZXJhdGVOb3fCqFJvb3ROYW1lqkZSMl9VU19UQzKtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTE5VDA5OjMzOjU2WqlQYXJhbWV0ZXLdAAAAA98AAAAEpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRl3wAAAASkdHlwZaVldmVudKlldmVudE5hbWWvVVNFRF9NRU0xX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRl3wAAAAWkdHlwZaRncmVwpm1hcmtlctkiU1lTX0lORk9fQ3Jhc2hQb3J0YWxVcGxvYWRfc3VjY2Vzc6ZzZWFyY2ixU3VjY2VzcyB1cGxvYWRpbmenbG9nRmlsZaxjb3JlX2xvZy50eHSjdXNlpWNvdW50tFJlcG9ydGluZ0FkanVzdG1lbnRz3wAAAAOuUmVwb3J0T25VcGRhdGXCtkZpcnN0UmVwb3J0aW5nSW50ZXJ2YWwPsE1heFVwbG9hZExhdGVuY3nNw1CkSFRUUN8AAAADo1VSTLtodHRwczovL3N0YnJ0bC5yNTMueGNhbC50di+mTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0;
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));
}

//When HTTP method is NULL
TEST(PROCESSCONFIGURATION_MSGPACK, METHOD_NULL)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char* data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDildmFsdWXfAAAADqROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGoUHJvdG9jb2ykSFRUUKxFbmNvZGluZ1R5cGWkSlNPTrFSZXBvcnRpbmdJbnRlcnZhbDyxQWN0aXZhdGlvblRpbWVvdXTNDhCrR2VuZXJhdGVOb3fCqFJvb3ROYW1lqkZSMl9VU19UQzKtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTE5VDA5OjMzOjU2WqlQYXJhbWV0ZXLdAAAAA98AAAAEpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRl3wAAAASkdHlwZaVldmVudKlldmVudE5hbWWvVVNFRF9NRU0xX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRl3wAAAAWkdHlwZaRncmVwpm1hcmtlctkiU1lTX0lORk9fQ3Jhc2hQb3J0YWxVcGxvYWRfc3VjY2Vzc6ZzZWFyY2ixU3VjY2VzcyB1cGxvYWRpbmenbG9nRmlsZaxjb3JlX2xvZy50eHSjdXNlpWNvdW50tFJlcG9ydGluZ0FkanVzdG1lbnRz3wAAAAOuUmVwb3J0T25VcGRhdGXCtkZpcnN0UmVwb3J0aW5nSW50ZXJ2YWwPsE1heFVwbG9hZExhdGVuY3nNw1CkSFRUUN8AAAADo1VSTLtodHRwczovL3N0YnJ0bC5yNTMueGNhbC50di+rQ29tcHJlc3Npb26kTm9uZbNSZXF1ZXN0VVJJUGFyYW1ldGVy3QAAAAHfAAAAAqROYW1lqnJlcG9ydE5hbWWpUmVmZXJlbmNlrFByb2ZpbGUuTmFtZaxKU09ORW5jb2RpbmffAAAAAqxSZXBvcnRGb3JtYXStTmFtZVZhbHVlUGFpcq9SZXBvcnRUaW1lc3RhbXCkTm9uZQ==";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0;
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));
}


//When RBUS_METHOD parameter is not given in the profile
TEST(PROCESSCONFIGURATION_MSGPACK, RBUS_METHOD_PARAM_NULL)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2imSGFzaDIzpXZhbHVl3wAAAAykTmFtZaxSREtCX1Byb2ZpbGWrRGVzY3JpcHRpb263Q2hlY2sgTXVsdGkgcHJvZmxpZSBvbmWnVmVyc2lvbqExqFByb3RvY29sq1JCVVNfTUVUSE9ErEVuY29kaW5nVHlwZaRKU09Oq0dlbmVyYXRlTm93wrFSZXBvcnRpbmdJbnRlcnZhbAqxQWN0aXZhdGlvblRpbWVPdXTM8K1UaW1lUmVmZXJlbmNltDAwMDEtMDEtMDFUMDA6MDA6MDBaqVBhcmFtZXRlct0AAAAF3wAAAAWkdHlwZaVldmVudKlldmVudE5hbWW0U1lTX1NIX2xpZ2h0dHBkQ3Jhc2ipY29tcG9uZW50s3Rlc3QtYW5kLWRpYWdub3N0aWOjdXNlpWNvdW50q3JlcG9ydEVtcHR5wt8AAAAFpHR5cGWkZ3JlcKZtYXJrZXK2U1lTX0lORk9fTE9HU19VUExPQURFRKZzZWFyY2jZLExPR1MgVVBMT0FERUQgU1VDQ0VTU0ZVTExZLCBSRVRVUk4gQ09ERTogMjAwp2xvZ0ZpbGWwQ29uc29sZWxvZy50eHQuMKN1c2WlY291bnTfAAAAA6R0eXBlqWRhdGFNb2RlbKRuYW1lo01BQ6lyZWZlcmVuY2XZJkRldmljZS5EZXZpY2VJbmZvLlhfQ09NQ0FTVC1DT01fQ01fTUFD3wAAAAKkdHlwZalkYXRhTW9kZWypcmVmZXJlbmNlrFByb2ZpbGUuTmFtZd8AAAACpHR5cGWpZGF0YU1vZGVsqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWrUkJVU19NRVRIT0TfAAAAAqZNZXRob2SgqlBhcmFtZXRlcnPdAAAAAt8AAAACpG5hbWWrY29udGVudFR5cGWldmFsdWW0UHJvZmlsZS5FbmNvZGluZ1R5cGXfAAAAAqRuYW1lpXRvcGljpXZhbHVlrFByb2ZpbGUuTmFtZaxKU09ORW5jb2RpbmffAAAAAqxSZXBvcnRGb3JtYXStTmFtZVZhbHVlUGFpcq9SZXBvcnRUaW1lc3RhbXCkTm9uZQ==";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Method param is not given in the profile
TEST(PROCESSCONFIGURATION_MSGPACK, METHOD_PARAM_NOT_PRESENT)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2imSGFzaDI0pXZhbHVl3wAAAAykTmFtZaxSREtCX1Byb2ZpbGWrRGVzY3JpcHRpb263Q2hlY2sgTXVsdGkgcHJvZmxpZSBvbmWnVmVyc2lvbqExqFByb3RvY29sq1JCVVNfTUVUSE9ErEVuY29kaW5nVHlwZaRKU09Oq0dlbmVyYXRlTm93wrFSZXBvcnRpbmdJbnRlcnZhbAqxQWN0aXZhdGlvblRpbWVPdXTM8K1UaW1lUmVmZXJlbmNltDAwMDEtMDEtMDFUMDA6MDA6MDBaqVBhcmFtZXRlct0AAAAF3wAAAAWkdHlwZaVldmVudKlldmVudE5hbWW0U1lTX1NIX2xpZ2h0dHBkQ3Jhc2ipY29tcG9uZW50s3Rlc3QtYW5kLWRpYWdub3N0aWOjdXNlpWNvdW50q3JlcG9ydEVtcHR5wt8AAAAFpHR5cGWkZ3JlcKZtYXJrZXK2U1lTX0lORk9fTE9HU19VUExPQURFRKZzZWFyY2jZLExPR1MgVVBMT0FERUQgU1VDQ0VTU0ZVTExZLCBSRVRVUk4gQ09ERTogMjAwp2xvZ0ZpbGWwQ29uc29sZWxvZy50eHQuMKN1c2WlY291bnTfAAAAA6R0eXBlqWRhdGFNb2RlbKRuYW1lo01BQ6lyZWZlcmVuY2XZJkRldmljZS5EZXZpY2VJbmZvLlhfQ09NQ0FTVC1DT01fQ01fTUFD3wAAAAKkdHlwZalkYXRhTW9kZWypcmVmZXJlbmNlrFByb2ZpbGUuTmFtZd8AAAACpHR5cGWpZGF0YU1vZGVsqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWrUkJVU19NRVRIT0TfAAAAAapQYXJhbWV0ZXJz3QAAAALfAAAAAqRuYW1lq2NvbnRlbnRUeXBlpXZhbHVltFByb2ZpbGUuRW5jb2RpbmdUeXBl3wAAAAKkbmFtZaV0b3BpY6V2YWx1ZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Unsupported protocol is given in the profile
TEST(PROCESSCONFIGURATION_MSGPACK, PROTOCOL_INVALID)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2imSGFzaDI2pXZhbHVl3wAAAA6kTmFtZaxSREtCX1Byb2ZpbGWrRGVzY3JpcHRpb26sUkRLQl9Qcm9maWxlp1ZlcnNpb26jMC4xqFByb3RvY29srFJCVVNfSU5WQUxJRKxFbmNvZGluZ1R5cGWkSlNPTrFSZXBvcnRpbmdJbnRlcnZhbDyxQWN0aXZhdGlvblRpbWVvdXTNDhCrR2VuZXJhdGVOb3fCqFJvb3ROYW1lqkZSMl9VU19UQzKtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTE5VDA5OjMzOjU2WqlQYXJhbWV0ZXLdAAAAA98AAAAEpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRl3wAAAASkdHlwZaVldmVudKlldmVudE5hbWWvVVNFRF9NRU0xX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRl3wAAAAWkdHlwZaRncmVwpm1hcmtlctkiU1lTX0lORk9fQ3Jhc2hQb3J0YWxVcGxvYWRfc3VjY2Vzc6ZzZWFyY2ixU3VjY2VzcyB1cGxvYWRpbmenbG9nRmlsZaxjb3JlX2xvZy50eHSjdXNlpWNvdW50tFJlcG9ydGluZ0FkanVzdG1lbnRz3wAAAAOuUmVwb3J0T25VcGRhdGXCtkZpcnN0UmVwb3J0aW5nSW50ZXJ2YWwPsE1heFVwbG9hZExhdGVuY3nNw1CkSFRUUN8AAAAEo1VSTLtodHRwczovL3N0YnJ0bC5yNTMueGNhbC50di+rQ29tcHJlc3Npb26kTm9uZaZNZXRob2SkUE9TVLNSZXF1ZXN0VVJJUGFyYW1ldGVy3QAAAAHfAAAAAqROYW1lqnJlcG9ydE5hbWWpUmVmZXJlbmNlrFByb2ZpbGUuTmFtZaxKU09ORW5jb2RpbmffAAAAAqxSZXBvcnRGb3JtYXStTmFtZVZhbHVlUGFpcq9SZXBvcnRUaW1lc3RhbXCkTm9uZQ==";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Reporting interval is greater than activation timeout
TEST(PROCESSCONFIGURATION_MSGPACK, RI_GT_AT)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2imSGFzaDI4pXZhbHVl3wAAAA6kTmFtZaxSREtCX1Byb2ZpbGWrRGVzY3JpcHRpb26sUkRLQl9Qcm9maWxlp1ZlcnNpb26jMC4xqFByb3RvY29spEhUVFCsRW5jb2RpbmdUeXBlpEpTT06xUmVwb3J0aW5nSW50ZXJ2YWzNF3CxQWN0aXZhdGlvblRpbWVPdXTNDhCrR2VuZXJhdGVOb3fCqFJvb3ROYW1lqkZSMl9VU19UQzKtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTE5VDA5OjMzOjU2WqlQYXJhbWV0ZXLdAAAAA98AAAAEpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRl3wAAAASkdHlwZaVldmVudKlldmVudE5hbWWvVVNFRF9NRU0xX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRl3wAAAAWkdHlwZaRncmVwpm1hcmtlctkiU1lTX0lORk9fQ3Jhc2hQb3J0YWxVcGxvYWRfc3VjY2Vzc6ZzZWFyY2ixU3VjY2VzcyB1cGxvYWRpbmenbG9nRmlsZaxjb3JlX2xvZy50eHSjdXNlpWNvdW50tFJlcG9ydGluZ0FkanVzdG1lbnRz3wAAAAOuUmVwb3J0T25VcGRhdGXCtkZpcnN0UmVwb3J0aW5nSW50ZXJ2YWwPsE1heFVwbG9hZExhdGVuY3nNw1CkSFRUUN8AAAAEo1VSTLtodHRwczovL3N0YnJ0bC5yNTMueGNhbC50di+rQ29tcHJlc3Npb26kTm9uZaZNZXRob2SkUE9TVLNSZXF1ZXN0VVJJUGFyYW1ldGVy3QAAAAHfAAAAAqROYW1lqnJlcG9ydE5hbWWpUmVmZXJlbmNlrFByb2ZpbGUuTmFtZaxKU09ORW5jb2RpbmffAAAAAqxSZXBvcnRGb3JtYXStTmFtZVZhbHVlUGFpcq9SZXBvcnRUaW1lc3RhbXCkTm9uZQ==";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Trigger condition type is not given in the profile
TEST(PROCESSCONFIGURATION_MSGPACK, TC_NO_TYPE)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2imSGFzaDI5pXZhbHVl3wAAAA6kTmFtZaxSREtCX1Byb2ZpbGWrRGVzY3JpcHRpb26sUkRLQl9Qcm9maWxlp1ZlcnNpb26jMC4xqFByb3RvY29spEhUVFCsRW5jb2RpbmdUeXBlpEpTT06xQWN0aXZhdGlvblRpbWVvdXTNDhCtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTIwVDA5OjMzOjU2WqtHZW5lcmF0ZU5vd8KoUm9vdE5hbWWqRlIyX1VTX1RDNalQYXJhbWV0ZXLdAAAAAt8AAAAEpHR5cGWlZXZlbnSpZXZlbnROYW1lr1VTRURfTUVNMV9zcGxpdKljb21wb25lbnSmc3lzaW50o3VzZahhYnNvbHV0Zd8AAAAFpHR5cGWkZ3JlcKZtYXJrZXLZIlNZU19JTkZPX0NyYXNoUG9ydGFsVXBsb2FkX3N1Y2Nlc3Omc2VhcmNosVN1Y2Nlc3MgdXBsb2FkaW5np2xvZ0ZpbGWsY29yZV9sb2cudHh0o3VzZaVjb3VudLBUcmlnZ2VyQ29uZGl0aW9u3QAAAAHfAAAAAqlyZWZlcmVuY2XZOURldmljZS5EZXZpY2VJbmZvLlhfUkRLQ0VOVFJBTC1DT01fUkZDLkZlYXR1cmUuT1ZTLkVuYWJsZahvcGVyYXRvcqNhbnm0UmVwb3J0aW5nQWRqdXN0bWVudHPfAAAAA65SZXBvcnRPblVwZGF0ZcK2Rmlyc3RSZXBvcnRpbmdJbnRlcnZhbA+wTWF4VXBsb2FkTGF0ZW5jec1OIKRIVFRQ3wAAAASjVVJMu2h0dHBzOi8vc3RicnRsLnI1My54Y2FsLnR2L6tDb21wcmVzc2lvbqROb25lpk1ldGhvZKRQT1NUs1JlcXVlc3RVUklQYXJhbWV0ZXLdAAAAAd8AAAACpE5hbWWqcmVwb3J0TmFtZalSZWZlcmVuY2WsUHJvZmlsZS5OYW1lrEpTT05FbmNvZGluZ98AAAACrFJlcG9ydEZvcm1hdK1OYW1lVmFsdWVQYWlyr1JlcG9ydFRpbWVzdGFtcKROb25l";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Trigger condition type is given invalid
TEST(PROCESSCONFIGURATION_MSGPACK, TC_OPERATOR_NULL)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char* data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2imSGFzaDMxpXZhbHVl3wAAAA6kTmFtZaxSREtCX1Byb2ZpbGWrRGVzY3JpcHRpb26sUkRLQl9Qcm9maWxlp1ZlcnNpb26jMC4xqFByb3RvY29spEhUVFCsRW5jb2RpbmdUeXBlpEpTT06xQWN0aXZhdGlvblRpbWVPdXTNDhCtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTIwVDA5OjMzOjU2WqtHZW5lcmF0ZU5vd8KoUm9vdE5hbWWqRlIyX1VTX1RDNalQYXJhbWV0ZXLdAAAAAt8AAAAEpHR5cGWlZXZlbnSpZXZlbnROYW1lr1VTRURfTUVNMV9zcGxpdKljb21wb25lbnSmc3lzaW50o3VzZahhYnNvbHV0Zd8AAAAFpHR5cGWkZ3JlcKZtYXJrZXLZIlNZU19JTkZPX0NyYXNoUG9ydGFsVXBsb2FkX3N1Y2Nlc3Omc2VhcmNosVN1Y2Nlc3MgdXBsb2FkaW5np2xvZ0ZpbGWsY29yZV9sb2cudHh0o3VzZaVjb3VudLBUcmlnZ2VyQ29uZGl0aW9u3QAAAAHfAAAAAqR0eXBlqWRhdGFNb2RlbKlyZWZlcmVuY2XZOURldmljZS5EZXZpY2VJbmZvLlhfUkRLQ0VOVFJBTC1DT01fUkZDLkZlYXR1cmUuT1ZTLkVuYWJsZbRSZXBvcnRpbmdBZGp1c3RtZW50c98AAAADrlJlcG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zU4gpEhUVFDfAAAABKNVUky7aHR0cHM6Ly9zdGJydGwucjUzLnhjYWwudHYvq0NvbXByZXNzaW9upE5vbmWmTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Triggercondition operator is not given in the profile
TEST(PROCESSCONFIGURATION_MSGPACK, TC_INVALID_TYPE)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2imSGFzaDMwpXZhbHVl3wAAAA6kTmFtZaxSREtCX1Byb2ZpbGWrRGVzY3JpcHRpb26sUkRLQl9Qcm9maWxlp1ZlcnNpb26jMC4xqFByb3RvY29spEhUVFCsRW5jb2RpbmdUeXBlpEpTT06xQWN0aXZhdGlvblRpbWVPdXTNDhCtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTIwVDA5OjMzOjU2WqtHZW5lcmF0ZU5vd8KoUm9vdE5hbWWqRlIyX1VTX1RDNalQYXJhbWV0ZXLdAAAAAt8AAAAEpHR5cGWlZXZlbnSpZXZlbnROYW1lr1VTRURfTUVNMV9zcGxpdKljb21wb25lbnSmc3lzaW50o3VzZahhYnNvbHV0Zd8AAAAFpHR5cGWkZ3JlcKZtYXJrZXLZIlNZU19JTkZPX0NyYXNoUG9ydGFsVXBsb2FkX3N1Y2Nlc3Omc2VhcmNosVN1Y2Nlc3MgdXBsb2FkaW5np2xvZ0ZpbGWsY29yZV9sb2cudHh0o3VzZaVjb3VudLBUcmlnZ2VyQ29uZGl0aW9u3QAAAAHfAAAAA6R0eXBlpUV2ZW50qXJlZmVyZW5jZdk5RGV2aWNlLkRldmljZUluZm8uWF9SREtDRU5UUkFMLUNPTV9SRkMuRmVhdHVyZS5PVlMuRW5hYmxlqG9wZXJhdG9yo2FuebRSZXBvcnRpbmdBZGp1c3RtZW50c98AAAADrlJlcG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zU4gpEhUVFDfAAAABKNVUky7aHR0cHM6Ly9zdGJydGwucjUzLnhjYWwudHYvq0NvbXByZXNzaW9upE5vbmWmTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}


//When Triggercondition operator is invalid in the given profile
TEST(PROCESSCONFIGURATION_MSGPACK, TC_OPERATOR_INVALID)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2imSGFzaDMypXZhbHVl3wAAAA6kTmFtZaxSREtCX1Byb2ZpbGWrRGVzY3JpcHRpb26sUkRLQl9Qcm9maWxlp1ZlcnNpb26jMC4xqFByb3RvY29spEhUVFCsRW5jb2RpbmdUeXBlpEpTT06xQWN0aXZhdGlvblRpbWVPdXTNDhCtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTIwVDA5OjMzOjU2WqtHZW5lcmF0ZU5vd8KoUm9vdE5hbWWqRlIyX1VTX1RDNalQYXJhbWV0ZXLdAAAAAt8AAAAEpHR5cGWlZXZlbnSpZXZlbnROYW1lr1VTRURfTUVNMV9zcGxpdKljb21wb25lbnSmc3lzaW50o3VzZahhYnNvbHV0Zd8AAAAFpHR5cGWkZ3JlcKZtYXJrZXLZIlNZU19JTkZPX0NyYXNoUG9ydGFsVXBsb2FkX3N1Y2Nlc3Omc2VhcmNosVN1Y2Nlc3MgdXBsb2FkaW5np2xvZ0ZpbGWsY29yZV9sb2cudHh0o3VzZaVjb3VudLBUcmlnZ2VyQ29uZGl0aW9u3QAAAAHfAAAAA6R0eXBlqWRhdGFNb2RlbKlyZWZlcmVuY2XZOURldmljZS5EZXZpY2VJbmZvLlhfUkRLQ0VOVFJBTC1DT01fUkZDLkZlYXR1cmUuT1ZTLkVuYWJsZahvcGVyYXRvcqJsZLRSZXBvcnRpbmdBZGp1c3RtZW50c98AAAADrlJlcG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zU4gpEhUVFDfAAAABKNVUky7aHR0cHM6Ly9zdGJydGwucjUzLnhjYWwudHYvq0NvbXByZXNzaW9upE5vbmWmTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Triggercondition threshold is not properly given for operator lt gt
TEST(PROCESSCONFIGURATION_MSGPACK, TC_NO_THRESHOLD_FOR_OPERATOR)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDOldmFsdWXfAAAADqROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGoUHJvdG9jb2ykSFRUUKxFbmNvZGluZ1R5cGWkSlNPTrFBY3RpdmF0aW9uVGltZU91dM0OEK1UaW1lUmVmZXJlbmNltDIwMjItMTItMjBUMDk6MzM6NTZaq0dlbmVyYXRlTm93wqhSb290TmFtZapGUjJfVVNfVEM1qVBhcmFtZXRlct0AAAAC3wAAAASkdHlwZaVldmVudKlldmVudE5hbWWvVVNFRF9NRU0xX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRl3wAAAAWkdHlwZaRncmVwpm1hcmtlctkiU1lTX0lORk9fQ3Jhc2hQb3J0YWxVcGxvYWRfc3VjY2Vzc6ZzZWFyY2ixU3VjY2VzcyB1cGxvYWRpbmenbG9nRmlsZaxjb3JlX2xvZy50eHSjdXNlpWNvdW50sFRyaWdnZXJDb25kaXRpb27dAAAAAd8AAAADpHR5cGWpZGF0YU1vZGVsqXJlZmVyZW5jZb1EZXZpY2UuRGV2aWNlSW5mby5EdW1teVBhcmFtMahvcGVyYXRvcqJndLRSZXBvcnRpbmdBZGp1c3RtZW50c98AAAADrlJlcG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zU4gpEhUVFDfAAAABKNVUky7aHR0cHM6Ly9zdGJydGwucjUzLnhjYWwudHYvq0NvbXByZXNzaW9upE5vbmWmTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When TC Reference is not given in the profile
TEST(PROCESSCONFIGURATION_MSGPACK, TC_NO_REFERENCE)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDSldmFsdWXfAAAADqROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGoUHJvdG9jb2ykSFRUUKxFbmNvZGluZ1R5cGWkSlNPTrFBY3RpdmF0aW9uVGltZU91dM0OEK1UaW1lUmVmZXJlbmNltDIwMjItMTItMjBUMDk6MzM6NTZaq0dlbmVyYXRlTm93wqhSb290TmFtZapGUjJfVVNfVEM1qVBhcmFtZXRlct0AAAAC3wAAAASkdHlwZaVldmVudKlldmVudE5hbWWvVVNFRF9NRU0xX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRl3wAAAAWkdHlwZaRncmVwpm1hcmtlctkiU1lTX0lORk9fQ3Jhc2hQb3J0YWxVcGxvYWRfc3VjY2Vzc6ZzZWFyY2ixU3VjY2VzcyB1cGxvYWRpbmenbG9nRmlsZaxjb3JlX2xvZy50eHSjdXNlpWNvdW50sFRyaWdnZXJDb25kaXRpb27dAAAAAd8AAAACpHR5cGWpZGF0YU1vZGVsqG9wZXJhdG9yo2FuebRSZXBvcnRpbmdBZGp1c3RtZW50c98AAAADrlJlcG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zU4gpEhUVFDfAAAABKNVUky7aHR0cHM6Ly9zdGJydGwucjUzLnhjYWwudHYvq0NvbXByZXNzaW9upE5vbmWmTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When TC Reference is empty for a profile

TEST(PROCESSCONFIGURATION_MSGPACK, TC_REFERENCE_EMPTY)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char* data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDWldmFsdWXfAAAAD6ROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGoUHJvdG9jb2ykSFRUUKxFbmNvZGluZ1R5cGWkSlNPTrFSZXBvcnRpbmdJbnRlcnZhbB6xQWN0aXZhdGlvblRpbWVPdXTNDhCtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTIwVDA5OjMzOjU2WqtHZW5lcmF0ZU5vd8KoUm9vdE5hbWWqRlIyX1VTX1RDNalQYXJhbWV0ZXLdAAAAAt8AAAAEpHR5cGWlZXZlbnSpZXZlbnROYW1lr1VTRURfTUVNMV9zcGxpdKljb21wb25lbnSmc3lzaW50o3VzZahhYnNvbHV0Zd8AAAAFpHR5cGWkZ3JlcKZtYXJrZXLZIlNZU19JTkZPX0NyYXNoUG9ydGFsVXBsb2FkX3N1Y2Nlc3Omc2VhcmNosVN1Y2Nlc3MgdXBsb2FkaW5np2xvZ0ZpbGWsY29yZV9sb2cudHh0o3VzZaVjb3VudLBUcmlnZ2VyQ29uZGl0aW9u3QAAAAHfAAAAA6R0eXBlqWRhdGFNb2RlbKlyZWZlcmVuY2WgqG9wZXJhdG9yo2FuebRSZXBvcnRpbmdBZGp1c3RtZW50c98AAAADrlJlcG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zU4gpEhUVFDfAAAABKNVUky7aHR0cHM6Ly9zdGJydGwucjUzLnhjYWwudHYvq0NvbXByZXNzaW9upE5vbmWmTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

//When Reference is not given for datamodel parameters
TEST(PROCESSCONFIGURATION_MSGPACK, DATAMODEL_REF_MANDATORY)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDWldmFsdWXfAAAADqROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGoUHJvdG9jb2ykSFRUUKxFbmNvZGluZ1R5cGWkSlNPTrFSZXBvcnRpbmdJbnRlcnZhbB6xQWN0aXZhdGlvblRpbWVPdXTNDhCrR2VuZXJhdGVOb3fCqFJvb3ROYW1lqkZSMl9VU19UQzKtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTE5VDA5OjMzOjU2WqlQYXJhbWV0ZXLdAAAAA98AAAADpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1Fo3VzZahhYnNvbHV0Zd8AAAAEpHR5cGWlZXZlbnSpZXZlbnROYW1lr1VTRURfTUVNMV9zcGxpdKljb21wb25lbnSmc3lzaW50o3VzZahhYnNvbHV0Zd8AAAAFpHR5cGWkZ3JlcKZtYXJrZXLZIlNZU19JTkZPX0NyYXNoUG9ydGFsVXBsb2FkX3N1Y2Nlc3Omc2VhcmNosVN1Y2Nlc3MgdXBsb2FkaW5np2xvZ0ZpbGWsY29yZV9sb2cudHh0o3VzZaVjb3VudLRSZXBvcnRpbmdBZGp1c3RtZW50c98AAAADrlJlcG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zROIpEhUVFDfAAAABKNVUky7aHR0cHM6Ly9zdGJydGwucjUzLnhjYWwudHYvq0NvbXByZXNzaW9upE5vbmWmTWV0aG9kpFBPU1SzUmVxdWVzdFVSSVBhcmFtZXRlct0AAAAB3wAAAAKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5n3wAAAAKsUmVwb3J0Rm9ybWF0rU5hbWVWYWx1ZVBhaXKvUmVwb3J0VGltZXN0YW1wpE5vbmU=";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0; 
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_SUCCESS,processMsgPackConfiguration(singleProfile, &profile));     
}

TEST(PROCESSCONFIGURATION_MSGPACK, REPORTFORMAT_NULL)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDeldmFsdWXfAAAADqROYW1lrFJES0JfUHJvZmlsZatEZXNjcmlwdGlvbqxSREtCX1Byb2ZpbGWnVmVyc2lvbqMwLjGoUHJvdG9jb2ykSFRUUKxFbmNvZGluZ1R5cGWkSlNPTrFSZXBvcnRpbmdJbnRlcnZhbB6xQWN0aXZhdGlvblRpbWVPdXTNDhCrR2VuZXJhdGVOb3fCqFJvb3ROYW1lqkZSMl9VU19UQzKtVGltZVJlZmVyZW5jZbQyMDIyLTEyLTE5VDA5OjMzOjU2WqlQYXJhbWV0ZXLdAAAAA98AAAAEpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRl3wAAAASkdHlwZaVldmVudKlldmVudE5hbWWvVVNFRF9NRU0xX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRl3wAAAAWkdHlwZaRncmVwpm1hcmtlctkiU1lTX0lORk9fQ3Jhc2hQb3J0YWxVcGxvYWRfc3VjY2Vzc6ZzZWFyY2ixU3VjY2VzcyB1cGxvYWRpbmenbG9nRmlsZaxjb3JlX2xvZy50eHSjdXNlpWNvdW50tFJlcG9ydGluZ0FkanVzdG1lbnRz3wAAAAOuUmVwb3J0T25VcGRhdGXCtkZpcnN0UmVwb3J0aW5nSW50ZXJ2YWwPsE1heFVwbG9hZExhdGVuY3nNE4ikSFRUUN8AAAAEo1VSTLtodHRwczovL3N0YnJ0bC5yNTMueGNhbC50di+rQ29tcHJlc3Npb26kTm9uZaZNZXRob2SkUE9TVLNSZXF1ZXN0VVJJUGFyYW1ldGVy3QAAAAHfAAAAAqROYW1lqnJlcG9ydE5hbWWpUmVmZXJlbmNlrFByb2ZpbGUuTmFtZaxKU09ORW5jb2RpbmffAAAAAa9SZXBvcnRUaW1lc3RhbXCkTm9uZQ==";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0;
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_FAILURE,processMsgPackConfiguration(singleProfile, &profile));     
}

TEST(PROCESSCONFIGURATION_MSGPACK, PARAMETER_TYPE_INVALID)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    guchar *webConfigString = NULL;
    char* data = "gahwcm9maWxlc5GDpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDeldmFsdWWOpE5hbWWsUkRLQl9Qcm9maWxlq0Rlc2NyaXB0aW9urFJES0JfUHJvZmlsZadWZXJzaW9uozAuMahQcm90b2NvbKRIVFRQrEVuY29kaW5nVHlwZaRKU09OsVJlcG9ydGluZ0ludGVydmFsHrFBY3RpdmF0aW9uVGltZU91dM0OEKtHZW5lcmF0ZU5vd8KoUm9vdE5hbWWqRlIyX1VTX1RDMq1UaW1lUmVmZXJlbmNltDIwMjItMTItMTlUMDk6MzM6NTZaqVBhcmFtZXRlcpOEpHR5cGWsdHlwZV9pbnZhbGlkpG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRlhKR0eXBlpWV2ZW50qWV2ZW50TmFtZa9VU0VEX01FTTFfc3BsaXSpY29tcG9uZW50pnN5c2ludKN1c2WoYWJzb2x1dGWFpHR5cGWkZ3JlcKZtYXJrZXLZIlNZU19JTkZPX0NyYXNoUG9ydGFsVXBsb2FkX3N1Y2Nlc3Omc2VhcmNosVN1Y2Nlc3MgdXBsb2FkaW5np2xvZ0ZpbGWsY29yZV9sb2cudHh0o3VzZaVjb3VudLRSZXBvcnRpbmdBZGp1c3RtZW50c4OuUmVwb3J0T25VcGRhdGXCtkZpcnN0UmVwb3J0aW5nSW50ZXJ2YWwPsE1heFVwbG9hZExhdGVuY3nNE4ikSFRUUISjVVJMu2h0dHBzOi8vc3RicnRsLnI1My54Y2FsLnR2L6tDb21wcmVzc2lvbqROb25lpk1ldGhvZKRQT1NUs1JlcXVlc3RVUklQYXJhbWV0ZXKRgqROYW1lqnJlcG9ydE5hbWWpUmVmZXJlbmNlrFByb2ZpbGUuTmFtZaxKU09ORW5jb2RpbmeCrFJlcG9ydEZvcm1hdK1OYW1lVmFsdWVQYWlyr1JlcG9ydFRpbWVzdGFtcKROb25l";
    gsize decodedDataLen = 0;
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;                                                                                 
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");                                              
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);                                                   
    EXPECT_EQ(T2ERROR_SUCCESS, processMsgPackConfiguration(singleProfile, &profile));                                               
}

TEST(PROCESSCONFIGURATION_MSGPACK, WORKING_CASE)
{
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;
    Profile *profile = NULL;
    char* data = "gahwcm9maWxlc5GDpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDeldmFsdWWPpE5hbWWsUkRLQl9Qcm9maWxlq0Rlc2NyaXB0aW9urFJES0JfUHJvZmlsZadWZXJzaW9uozAuMahQcm90b2NvbKRIVFRQrEVuY29kaW5nVHlwZaRKU09OsVJlcG9ydGluZ0ludGVydmFsHrFBY3RpdmF0aW9uVGltZU91dM0OEKtHZW5lcmF0ZU5vd8KoUm9vdE5hbWWqRlIyX1VTX1RDMq1UaW1lUmVmZXJlbmNltDIwMjItMTItMTlUMDk6MzM6NTZaqVBhcmFtZXRlcpOEpHR5cGWsdHlwZV9pbnZhbGlkpG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRlhKR0eXBlpWV2ZW50qWV2ZW50TmFtZa9VU0VEX01FTTFfc3BsaXSpY29tcG9uZW50pnN5c2ludKN1c2WoYWJzb2x1dGWFpHR5cGWkZ3JlcKZtYXJrZXLZIlNZU19JTkZPX0NyYXNoUG9ydGFsVXBsb2FkX3N1Y2Nlc3Omc2VhcmNosVN1Y2Nlc3MgdXBsb2FkaW5np2xvZ0ZpbGWsY29yZV9sb2cudHh0o3VzZaVjb3VudLBUcmlnZ2VyQ29uZGl0aW9ukYSkdHlwZalkYXRhTW9kZWypcmVmZXJlbmNl2TlEZXZpY2UuRGV2aWNlSW5mby5YX1JES0NFTlRSQUwtQ09NX1JGQy5GZWF0dXJlLk9WUy5FbmFibGWob3BlcmF0b3KiZ3SpdGhyZXNob2xkoTC0UmVwb3J0aW5nQWRqdXN0bWVudHODrlJlcG9ydE9uVXBkYXRlwrZGaXJzdFJlcG9ydGluZ0ludGVydmFsD7BNYXhVcGxvYWRMYXRlbmN5zROIpEhUVFCEo1VSTLtodHRwczovL3N0YnJ0bC5yNTMueGNhbC50di+rQ29tcHJlc3Npb26kTm9uZaZNZXRob2SkUE9TVLNSZXF1ZXN0VVJJUGFyYW1ldGVykYKkTmFtZapyZXBvcnROYW1lqVJlZmVyZW5jZaxQcm9maWxlLk5hbWWsSlNPTkVuY29kaW5ngqxSZXBvcnRGb3JtYXStTmFtZVZhbHVlUGFpcq9SZXBvcnRUaW1lc3RhbXCkTm9uZQ==";
    guchar *webConfigString = NULL;
    gsize decodedDataLen = 0;
    webConfigString = g_base64_decode(data, &decodedDataLen);
    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, (char*)webConfigString, decodedDataLen, &off);
    msgpack_object *profiles_root =  &result.data;
    msgpack_object *profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    msgpack_object *singleProfile = msgpack_get_array_element(profilesArray, 0);
    EXPECT_EQ(T2ERROR_SUCCESS, processMsgPackConfiguration(singleProfile, &profile));
}


/* Helper to create a TriggerCondition JSON object */
static cJSON* create_tc_obj(const char* type, const char* op, const char* ref, int threshold, int report)
{
    cJSON* obj = cJSON_CreateObject();
    if (type) cJSON_AddStringToObject(obj, "type", type);
    if (op) cJSON_AddStringToObject(obj, "operator", op);
    if (ref) cJSON_AddStringToObject(obj, "reference", ref);
    if (threshold >= 0) cJSON_AddNumberToObject(obj, "threshold", threshold);
    if (report >= 0) cJSON_AddBoolToObject(obj, "report", report);
    return obj;
}

/* verifyTriggerCondition failure: missing type */
TEST(T2ParserVerifyTC, MissingTypeShouldFail)
{
    cJSON* arr = cJSON_CreateArray();
    cJSON* tc = cJSON_CreateObject();
    // intentionally no "type"
    cJSON_AddStringToObject(tc, "operator", "any");
    cJSON_AddStringToObject(tc, "reference", "Device.Param");
    cJSON_AddItemToArray(arr, tc);

    EXPECT_EQ(T2ERROR_FAILURE, verifyTriggerCondition(arr));
    cJSON_Delete(arr);
}
/* verifyTriggerCondition failure: wrong type string */
TEST(T2ParserVerifyTC, WrongTypeShouldFail)
{
    cJSON* arr = cJSON_CreateArray();
    cJSON* tc = create_tc_obj("notDataModel", "any", "Device.Param", -1, -1);
    cJSON_AddItemToArray(arr, tc);
    EXPECT_EQ(T2ERROR_FAILURE, verifyTriggerCondition(arr));
    cJSON_Delete(arr);
}

/* verifyTriggerCondition failure: missing operator */
TEST(T2ParserVerifyTC, MissingOperatorShouldFail)
{
    cJSON* arr = cJSON_CreateArray();
    cJSON* tc = create_tc_obj("dataModel", NULL, "Device.Param", -1, -1);
    cJSON_AddItemToArray(arr, tc);
    EXPECT_EQ(T2ERROR_FAILURE, verifyTriggerCondition(arr));
    cJSON_Delete(arr);
}

/* verifyTriggerCondition failure: invalid operator */
TEST(T2ParserVerifyTC, InvalidOperatorShouldFail)
{
    cJSON* arr = cJSON_CreateArray();
    cJSON* tc = create_tc_obj("dataModel", "badop", "Device.Param", -1, -1);
    cJSON_AddItemToArray(arr, tc);
    EXPECT_EQ(T2ERROR_FAILURE, verifyTriggerCondition(arr));
    cJSON_Delete(arr);
}

/* verifyTriggerCondition failure: missing threshold for lt operator */
TEST(T2ParserVerifyTC, ThresholdMissingForLtShouldFail)
{
    cJSON* arr = cJSON_CreateArray();
    cJSON* tc = create_tc_obj("dataModel", "lt", "Device.Param", -1, -1); // no threshold
    cJSON_AddItemToArray(arr, tc);
    EXPECT_EQ(T2ERROR_FAILURE, verifyTriggerCondition(arr));
    cJSON_Delete(arr);
}

/* verifyTriggerCondition failure: missing reference */
TEST(T2ParserVerifyTC, MissingReferenceShouldFail)
{
    cJSON* arr = cJSON_CreateArray();
    cJSON* tc = cJSON_CreateObject();
    cJSON_AddStringToObject(tc, "type", "dataModel");
    cJSON_AddStringToObject(tc, "operator", "any");
    // no reference
    cJSON_AddItemToArray(arr, tc);
    EXPECT_EQ(T2ERROR_FAILURE, verifyTriggerCondition(arr));
    cJSON_Delete(arr);
}

/* verifyTriggerCondition failure: empty reference */
TEST(T2ParserVerifyTC, EmptyReferenceShouldFail)
{
    cJSON* arr = cJSON_CreateArray();
    cJSON* tc = create_tc_obj("dataModel", "any", "", -1, -1);
    cJSON_AddItemToArray(arr, tc);
    EXPECT_EQ(T2ERROR_FAILURE, verifyTriggerCondition(arr));
    cJSON_Delete(arr);
}

/* verifyTriggerCondition success: valid tc */
TEST(T2ParserVerifyTC, ValidShouldSucceed)
{
    cJSON* arr = cJSON_CreateArray();
    cJSON* tc = create_tc_obj("dataModel", "any", "Device.Param", -1, -1);
    cJSON_AddItemToArray(arr, tc);
    EXPECT_EQ(T2ERROR_SUCCESS, verifyTriggerCondition(arr));
    cJSON_Delete(arr);
}
/* addTriggerCondition should accept a valid TC array and return success */
TEST(T2ParserAddTC, AddTriggerConditionSuccess)
{
    Profile p;
    memset(&p, 0, sizeof(p));
    // Initialize the vector pointer (function will Vector_Create)
    p.triggerConditionList = NULL;

    cJSON* arr = cJSON_CreateArray();
    cJSON* tc = create_tc_obj("dataModel", "eq", "Device.Param", 5, 1);
    cJSON_AddItemToArray(arr, tc);

    EXPECT_EQ(T2ERROR_SUCCESS, addTriggerCondition(&p, arr));
    // function should have created the triggerConditionList (non-NULL)
    EXPECT_NE((void*)NULL, (void*)p.triggerConditionList);

    // clean up: free internal structures created by addTriggerCondition
    // We don't have a public destructor here; best-effort free for the test's allocations:
    // iterate vector if available (Vector API may provide size/get functions in the project),
    // but to stay robust, simply free profile memory fields that addTriggerCondition could set.
    // For safety, reset pointer to avoid double-free during test teardown.
    // (The CI process reclaims process memory after tests.)
    cJSON_Delete(arr);
}
/* encodingSet should set jsonEncoding fields based on JSON nodes */
TEST(T2ParserEncodingSet, JSONEncodingMapping)
{
    Profile p;
    memset(&p, 0, sizeof(p));
    p.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    memset(p.jsonEncoding, 0, sizeof(JSONEncoding));

    cJSON* jEncodingType = cJSON_CreateString("JSON");
    cJSON* jJSONReportFormat = cJSON_CreateString("ObjectHierarchy");
    cJSON* jJSONReportTimestamp = cJSON_CreateString("Unix-Epoch");

    EXPECT_EQ(T2ERROR_SUCCESS, encodingSet(&p, jEncodingType, jJSONReportFormat, jJSONReportTimestamp));
    EXPECT_EQ(JSONRF_OBJHIERARCHY, p.jsonEncoding->reportFormat);
    EXPECT_EQ(TIMESTAMP_UNIXEPOCH, p.jsonEncoding->tsFormat);

    if (p.jsonEncoding)
    {
        free(p.jsonEncoding);
        p.jsonEncoding = NULL;
    }

    if (p.name)
    {
        free(p.name);
        p.name = NULL;
    }

    cJSON_Delete(jEncodingType);
    cJSON_Delete(jJSONReportFormat);
    cJSON_Delete(jJSONReportTimestamp);
}
/* protocolSet tests: HTTP branch */
TEST(T2ParserProtocolSet, HTTPBranchSetsURLAndParams)
{
    Profile p;
    memset(&p, 0, sizeof(p));
    p.t2HTTPDest = (T2HTTP*)malloc(sizeof(T2HTTP));
    memset(p.t2HTTPDest, 0, sizeof(T2HTTP));
    p.name = strdup("TestProfile");

    cJSON* jProtocol = cJSON_CreateString("HTTP");
    // create HTTP nested object
    cJSON* jHTTP = cJSON_CreateObject();
    cJSON_AddStringToObject(jHTTP, "URL", "http://upload.test");
    cJSON_AddStringToObject(jHTTP, "Compression", "None");
    cJSON_AddStringToObject(jHTTP, "Method", "POST");

    // RequestURIParameter array with one valid entry
    cJSON* arr = cJSON_CreateArray();
    cJSON* entry = cJSON_CreateObject();
    cJSON_AddStringToObject(entry, "Reference", "someRef");
    cJSON_AddStringToObject(entry, "Name", "someName");
    cJSON_AddItemToArray(arr, entry);
    cJSON_AddItemToObject(jHTTP, "RequestURIParameter", arr);

    // Call protocolSet (note: jprofileHTTPRequestURIParameter_count must be set properly by caller)
    EXPECT_EQ(T2ERROR_SUCCESS, protocolSet(&p, jProtocol, cJSON_GetObjectItem(jHTTP, "URL"), cJSON_GetObjectItem(jHTTP, "RequestURIParameter"), 1, NULL, NULL, 0));

    EXPECT_STREQ("http://upload.test", p.t2HTTPDest->URL);
        // Cleanup to avoid memory leaks
    if (p.t2HTTPDest)
    {
        if (p.t2HTTPDest->URL)
        {
            free(p.t2HTTPDest->URL);
            p.t2HTTPDest->URL = NULL;
        }
        // If protocolSet allocated other heap members inside T2HTTP, free them here as needed.

        free(p.t2HTTPDest);
        p.t2HTTPDest = NULL;
    }

    if (p.name)
    {
        free(p.name);
        p.name = NULL;
    }

    cJSON_Delete(jProtocol);
    cJSON_Delete(jHTTP);
}

/* protocolSet tests: RBUS_METHOD branch */
TEST(T2ParserProtocolSet, RBUSMethodBranchSetsMethodAndParams)
{
    Profile p;
    memset(&p, 0, sizeof(p));
    p.t2RBUSDest = (T2RBUS*)malloc(sizeof(T2RBUS));
    memset(p.t2RBUSDest, 0, sizeof(T2RBUS));
    p.name = strdup("RbusProfile");

    cJSON* jProtocol = cJSON_CreateString("RBUS_METHOD");
    cJSON* jRBUS = cJSON_CreateObject();
    cJSON_AddStringToObject(jRBUS, "Method", "TestMethod");

    cJSON* params = cJSON_CreateArray();
    cJSON* param = cJSON_CreateObject();
    cJSON_AddStringToObject(param, "name", "param1");
    cJSON_AddStringToObject(param, "value", "val1");
    cJSON_AddItemToArray(params, param);
    cJSON_AddItemToObject(jRBUS, "Parameters", params);

    // protocolSet expects jprofileRBUSMethodName and jprofileRBUSMethodParamArr separately; pass appropriate items
    EXPECT_EQ(T2ERROR_SUCCESS, protocolSet(&p, jProtocol, NULL, NULL, 0, cJSON_GetObjectItem(jRBUS, "Method"), cJSON_GetObjectItem(jRBUS, "Parameters"), 1));

    EXPECT_STREQ("TestMethod", p.t2RBUSDest->rbusMethodName);
        // Cleanup to avoid memory leaks
    if (p.t2RBUSDest)
    {
        // free any strings allocated by protocolSet if present
        if (p.t2RBUSDest->rbusMethodName)
        {
            free(p.t2RBUSDest->rbusMethodName);
            p.t2RBUSDest->rbusMethodName = NULL;
        }
        // If protocolSet allocated other heap members inside T2RBUS, free them here as needed.
        free(p.t2RBUSDest);
        p.t2RBUSDest = NULL;
    }

    if (p.name)
    {
        free(p.name);
        p.name = NULL;
    }

    cJSON_Delete(jProtocol);
    cJSON_Delete(jRBUS);
}

void StaticParam_Destroy(void* param) {
    StaticParam* sparam = (StaticParam*)param;
    if (sparam) {
        if (sparam->paramType) free(sparam->paramType);
        if (sparam->name) free(sparam->name);
        if (sparam->value) free(sparam->value);
        free(sparam);
    }
}

#ifdef GTEST_ENABLE
#endif
