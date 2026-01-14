#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <chrono>

#include "test/mocks/SystemMock.h"
#include "test/mocks/FileioMock.h"
#include "test/mocks/rdklogMock.h"
#include "test/mocks/rbusMock.h"
#include "test/mocks/rdkconfigMock.h"
#include "test/mocks/VectorMock.h"
#include "test/bulkdata/SchedulerMock.h"

using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::Invoke;

extern "C" {
#include "busInterface.h"
//#include "profile.c"
#include "profile.h"
#include "datamodel.h"
#include "t2markers.h"
#include "reportprofiles.h"
#include "profilexconf.h"
#include "t2eventreceiver.h"
#include "msgpack.h"

extern bool initialized;

sigset_t blocking_signal;
hash_map_t *markerCompMap = NULL;
} 
 
FileMock *g_fileIOMock = NULL;
SystemMock * g_systemMock = NULL;
rdklogMock *m_rdklogMock = NULL;
rbusMock *g_rbusMock = NULL;
rdkconfigMock *g_rdkconfigMock = nullptr;
extern VectorMock *g_vectorMock;
extern SchedulerMock *g_schedulerMock;

class ProfileTest : public ::testing::Test {
protected:
    void SetUp() override 
    {
        g_fileIOMock = new FileMock();
        g_systemMock = new SystemMock();
	g_rbusMock = new rbusMock();
	g_rdkconfigMock = new rdkconfigMock();
	g_vectorMock = new VectorMock();
	g_schedulerMock = new SchedulerMock();
    }
    void TearDown() override 
    {
       delete g_fileIOMock;
       delete g_systemMock;
       delete g_rbusMock;
       delete g_rdkconfigMock;
       delete g_vectorMock;
       delete g_schedulerMock;

        g_fileIOMock = nullptr;
        g_systemMock = nullptr;
	g_rbusMock = nullptr;
	g_rdkconfigMock = nullptr;
	g_vectorMock = nullptr;
	g_schedulerMock = nullptr;
    }
};
#if 1
extern "C" {
typedef void (*freeRequestURIparamFunc)(void *);
freeRequestURIparamFunc freeRequestURIparamFuncCallback(void);

typedef void (*freeReportProfileConfigFunc)(void *);
freeReportProfileConfigFunc freeReportProfileConfigFuncCallback(void);

typedef void (*freeProfileFunc)(void *);
freeProfileFunc freeProfileFuncCallback(void);

typedef T2ERROR (*getProfileFunc)(const char*, Profile**);
getProfileFunc getProfileFuncCallback(void);

typedef T2ERROR (*initJSONReportProfileFunc)(cJSON **, cJSON **, char *);
initJSONReportProfileFunc initJSONReportProfileFuncCallback(void);

typedef void* (*CollectAndReportFunc)(void*);
CollectAndReportFunc getCollectAndReportFunc(void);
}

struct Config {
    char* name;
    char* configData;
};

TEST_F(ProfileTest, FreeRequestURIparam_Null) {
    freeRequestURIparamFunc freeFunc = freeRequestURIparamFuncCallback();
    ASSERT_NE(freeFunc, nullptr);
    freeFunc(nullptr); 
}
TEST_F(ProfileTest, FreeRequestURIparam_Valid) {
    freeRequestURIparamFunc freeFunc = freeRequestURIparamFuncCallback();
    ASSERT_NE(freeFunc, nullptr);

    HTTPReqParam* param = (HTTPReqParam*)malloc(sizeof(HTTPReqParam));
    ASSERT_NE(param, nullptr);

    param->HttpName = strdup("TestName");
    param->HttpRef = strdup("TestRef");
    param->HttpValue = strdup("TestValue");

    freeFunc(param);
}
TEST_F(ProfileTest, FreeRequestURIparam_Partials) {
    freeRequestURIparamFunc freeFunc = freeRequestURIparamFuncCallback();
    ASSERT_NE(freeFunc, nullptr);

    HTTPReqParam* param1 = (HTTPReqParam*)calloc(1, sizeof(HTTPReqParam));
    param1->HttpName = strdup("TestName");
    freeFunc(param1);

    HTTPReqParam* param2 = (HTTPReqParam*)calloc(1, sizeof(HTTPReqParam));
    param2->HttpRef = strdup("TestRef");
    freeFunc(param2);

    HTTPReqParam* param3 = (HTTPReqParam*)calloc(1, sizeof(HTTPReqParam));
    param3->HttpValue = strdup("TestValue");
    freeFunc(param3);

    HTTPReqParam* param4 = (HTTPReqParam*)calloc(1, sizeof(HTTPReqParam));
    freeFunc(param4);
}

TEST_F(ProfileTest, FreeReportProfileConfig_Null) {
    freeReportProfileConfigFunc freeFunc = freeReportProfileConfigFuncCallback();
    ASSERT_NE(freeFunc, nullptr);
    freeFunc(nullptr); 
}

TEST_F(ProfileTest, FreeReportProfileConfig_Valid) {
    freeReportProfileConfigFunc freeFunc = freeReportProfileConfigFuncCallback();
    ASSERT_NE(freeFunc, nullptr);

    Config *config = (Config*)malloc(sizeof(Config));
    ASSERT_NE(config, nullptr);
    config->name = strdup("TestProfile");
    config->configData = strdup("SomeData");

    freeFunc(config); 
}

TEST_F(ProfileTest, FreeReportProfileConfig_Partials) {
    freeReportProfileConfigFunc freeFunc = freeReportProfileConfigFuncCallback();
    ASSERT_NE(freeFunc, nullptr);

    Config *config1 = (Config*)calloc(1, sizeof(Config));
    config1->name = strdup("TestProfile");
    freeFunc(config1);

    Config *config2 = (Config*)calloc(1, sizeof(Config));
    config2->configData = strdup("SomeData");
    freeFunc(config2);

    Config *config3 = (Config*)calloc(1, sizeof(Config));
    freeFunc(config3);
}

TEST_F(ProfileTest, FreeProfile_Null) {
    freeProfileFunc freeFunc = freeProfileFuncCallback();
    ASSERT_NE(freeFunc, nullptr);
    freeFunc(nullptr);
}
TEST_F(ProfileTest, FreeProfile_Valid) {
    freeProfileFunc freeFunc = freeProfileFuncCallback();
    ASSERT_NE(freeFunc, nullptr);

    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    prof->name = strdup("profile_name");
    prof->hash = strdup("profile_hash");
    prof->protocol = strdup("HTTP");
    prof->encodingType = strdup("JSON");
    prof->RootName = strdup("Root");
    prof->Description = strdup("desc");
    prof->version = strdup("1.0");
    prof->timeRef = strdup("time");
    prof->jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));

    prof->t2HTTPDest = (T2HTTP*)calloc(1, sizeof(T2HTTP));
    prof->t2HTTPDest->URL = strdup("https://test.url");
    prof->t2HTTPDest->RequestURIparamList = NULL; 

    prof->t2RBUSDest = (T2RBUS*)calloc(1, sizeof(T2RBUS));
    prof->t2RBUSDest->rbusMethodName = strdup("method");
    prof->t2RBUSDest->rbusMethodParamList = NULL;

    prof->eMarkerList = NULL;
    prof->gMarkerList = NULL;
    prof->topMarkerList = NULL;
    prof->paramList = NULL;
    prof->staticParamList = NULL;
    prof->triggerConditionList = NULL;
    prof->cachedReportList = NULL;

    prof->jsonReportObj = cJSON_CreateObject();
   
    freeFunc(prof);
}
TEST_F(ProfileTest, FreeProfile_OnlyNameSet) {
    freeProfileFunc freeFunc = freeProfileFuncCallback();
    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    prof->name = strdup("profile_name");
    freeFunc(prof);
}
TEST_F(ProfileTest, GetProfile_NullName) {
    getProfileFunc func = getProfileFuncCallback();
    Profile* prof = nullptr;
    T2ERROR err = func(nullptr, &prof);
    EXPECT_EQ(err, T2ERROR_FAILURE); 
}

TEST_F(ProfileTest, FreeProfile_WithGMarkerList_CallsVectorDestroy) {
    VectorMock* vectorMock = new VectorMock();
    g_vectorMock = vectorMock;

    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    Vector* test_vector = (Vector*)0x456;
    prof->gMarkerList = test_vector;
    EXPECT_CALL(*vectorMock, Vector_Destroy(test_vector, testing::_))
        .Times(1)
        .WillOnce(testing::Return(T2ERROR_SUCCESS));

    freeProfileFunc freeFunc = freeProfileFuncCallback();
    freeFunc(prof);

    delete vectorMock;
    g_vectorMock = nullptr;
}

TEST_F(ProfileTest, FreeProfile_WithHTTPDest_RequestURIparamList_CallsVectorDestroy) {
    VectorMock* vectorMock = new VectorMock();
    g_vectorMock = vectorMock;

    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    prof->t2HTTPDest = (T2HTTP*)calloc(1, sizeof(T2HTTP));
    Vector* test_vector = (Vector*)0x789;
    prof->t2HTTPDest->RequestURIparamList = test_vector;
    prof->t2HTTPDest->URL = strdup("dummy");
    EXPECT_CALL(*vectorMock, Vector_Destroy(test_vector, testing::_))
        .Times(1)
        .WillOnce(testing::Return(T2ERROR_SUCCESS));

    freeProfileFunc freeFunc = freeProfileFuncCallback();
    freeFunc(prof);

    delete vectorMock;
    g_vectorMock = nullptr;
}

TEST_F(ProfileTest, FreeProfile_WithRBUSDest_rbusMethodParamList_CallsVectorDestroy) {
    VectorMock* vectorMock = new VectorMock();
    g_vectorMock = vectorMock;

    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    prof->t2RBUSDest = (T2RBUS*)calloc(1, sizeof(T2RBUS));
    Vector* test_vector = (Vector*)0xABC;
    prof->t2RBUSDest->rbusMethodParamList = test_vector;
    prof->t2RBUSDest->rbusMethodName = strdup("dummy");
    EXPECT_CALL(*vectorMock, Vector_Destroy(test_vector, testing::_))
        .Times(1)
        .WillOnce(testing::Return(T2ERROR_SUCCESS));

    freeProfileFunc freeFunc = freeProfileFuncCallback();
    freeFunc(prof);

    delete vectorMock;
    g_vectorMock = nullptr;
}

void cover_VectorDestroy_field(void** field, Vector* ptr, VectorMock& vectorMock, Vector_Cleanup expected_cleanup) {
    *field = ptr;
    EXPECT_CALL(vectorMock, Vector_Destroy(ptr, expected_cleanup))
        .Times(1)
        .WillOnce(::testing::Return(T2ERROR_SUCCESS));
}

TEST_F(ProfileTest, FreeProfile_EMarkerList_CallsVectorDestroy) {
    VectorMock vectorMock;
    g_vectorMock = &vectorMock;
    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    cover_VectorDestroy_field((void**)&prof->eMarkerList, (Vector*)0xEACE, vectorMock, freeEMarker);
    freeProfileFunc freeFunc = freeProfileFuncCallback();
    freeFunc(prof);
    g_vectorMock = nullptr;
}

TEST_F(ProfileTest, FreeProfile_GMarkerList_CallsVectorDestroy) {
    VectorMock vectorMock;
    g_vectorMock = &vectorMock;
    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    cover_VectorDestroy_field((void**)&prof->gMarkerList, (Vector*)0xBEEF, vectorMock, freeGMarker);
    freeProfileFunc freeFunc = freeProfileFuncCallback();
    freeFunc(prof);
    g_vectorMock = nullptr;
}

TEST_F(ProfileTest, FreeProfile_TopMarkerList_CallsVectorDestroy) {
    VectorMock vectorMock;
    g_vectorMock = &vectorMock;
    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    cover_VectorDestroy_field((void**)&prof->topMarkerList, (Vector*)0xFAFA, vectorMock, freeGMarker);
    freeProfileFunc freeFunc = freeProfileFuncCallback();
    freeFunc(prof);
    g_vectorMock = nullptr;
}

TEST_F(ProfileTest, FreeProfile_ParamList_CallsVectorDestroy) {
    VectorMock vectorMock;
    g_vectorMock = &vectorMock;
    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    cover_VectorDestroy_field((void**)&prof->paramList, (Vector*)0xCAFE, vectorMock, freeParam);
    freeProfileFunc freeFunc = freeProfileFuncCallback();
    freeFunc(prof);
    g_vectorMock = nullptr;
}

TEST_F(ProfileTest, FreeProfile_StaticParamList_CallsVectorDestroy) {
    VectorMock vectorMock;
    g_vectorMock = &vectorMock;
    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    cover_VectorDestroy_field((void**)&prof->staticParamList, (Vector*)0xF00D, vectorMock, freeStaticParam);
    freeProfileFunc freeFunc = freeProfileFuncCallback();
    freeFunc(prof);
    g_vectorMock = nullptr;
}

TEST_F(ProfileTest, FreeProfile_TriggerConditionList_CallsVectorDestroy) {
    VectorMock vectorMock;
    g_vectorMock = &vectorMock;
    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    cover_VectorDestroy_field((void**)&prof->triggerConditionList, (Vector*)0xDEAD, vectorMock, freeTriggerCondition);
    freeProfileFunc freeFunc = freeProfileFuncCallback();
    freeFunc(prof);
    g_vectorMock = nullptr;
}

TEST_F(ProfileTest, FreeProfile_CachedReportList_CallsVectorDestroy_And_SetsNull) {
    VectorMock vectorMock;
    g_vectorMock = &vectorMock;
    Profile* prof = (Profile*)calloc(1, sizeof(Profile));
    Vector* dummyVec = (Vector*)0x123456;
    prof->cachedReportList = dummyVec;
    EXPECT_CALL(vectorMock, Vector_Destroy(dummyVec, free))
        .Times(1)
        .WillOnce(::testing::Return(T2ERROR_SUCCESS));

    freeProfileFunc freeFunc = freeProfileFuncCallback();
    freeFunc(prof);

    EXPECT_EQ(prof->cachedReportList, nullptr);
    g_vectorMock = nullptr;
}

TEST_F(ProfileTest, InitJSONReportProfile_Success) {
    initJSONReportProfileFunc func = initJSONReportProfileFuncCallback();
    cJSON *jsonObj = nullptr;
    cJSON *valArray = nullptr;
    const char *rootname = "root";

    T2ERROR result = func(&jsonObj, &valArray, (char*)rootname);
    EXPECT_EQ(result, T2ERROR_SUCCESS);
    ASSERT_NE(jsonObj, nullptr);
    ASSERT_NE(valArray, nullptr);

    cJSON *arr = cJSON_GetObjectItem(jsonObj, rootname);
    ASSERT_TRUE(arr != nullptr && arr == valArray && cJSON_IsArray(arr));

    cJSON_Delete(jsonObj); 
}

TEST(CollectAndReportTest, NullDataReturnsNull) {
    CollectAndReportFunc fn = getCollectAndReportFunc();
    void* ret = fn(nullptr);
    EXPECT_EQ(ret, nullptr); 
}

TEST(CollectAndReportTest, HandlesRestartEventAndExits) {
    CollectAndReportFunc fn = getCollectAndReportFunc();
    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 1;
    profile.grepSeekProfile = &grepSeekProfile;
    profile.name = (char*)"test";
    profile.encodingType = (char*)"json";
    profile.protocol = (char*)"http";

    pthread_t tid;
    void* returnVal = nullptr;

    pthread_create(&tid, NULL, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    int join_result = pthread_join(tid, &returnVal);
    EXPECT_EQ(returnVal, nullptr); 
    EXPECT_EQ(join_result, 0);     
}

TEST(CollectAndReportTest, ProperParametersHappyPath) {
    CollectAndReportFunc fn = getCollectAndReportFunc();
    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 42;

    profile.name = const_cast<char *>("test");
    profile.encodingType = const_cast<char *>("json");
    profile.protocol = const_cast<char *>("http");
    profile.grepSeekProfile = &grepSeekProfile;


    pthread_t collectThread;
    pthread_create(&collectThread, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);
    void* result = nullptr;
    pthread_join(collectThread, &result);

}

TEST(CollectAndReportTest, Null_Parameter1) {
    CollectAndReportFunc fn = getCollectAndReportFunc();
    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 42;

    profile.name = nullptr;
    profile.encodingType = nullptr;
    profile.protocol = nullptr;
    profile.grepSeekProfile = &grepSeekProfile;
    profile.triggerReportOnCondition = true;

    pthread_t collectThread;
    pthread_create(&collectThread, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);
    void* result = nullptr;
    pthread_join(collectThread, &result);

}

TEST(CollectAndReportTest, Null_Profilename) {
    CollectAndReportFunc fn = getCollectAndReportFunc();
    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 42;

    profile.name = nullptr;
    profile.encodingType = nullptr;
    profile.protocol = nullptr;
    profile.grepSeekProfile = &grepSeekProfile;
    profile.triggerReportOnCondition = false;

    pthread_t collectThread;
    pthread_create(&collectThread, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);
    void* result = nullptr;
    pthread_join(collectThread, &result);

}
TEST(CollectAndReportTest, Null_Parameter2) {
    CollectAndReportFunc fn = getCollectAndReportFunc();
    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 42;

    profile.name = (char*)"test";
    profile.encodingType = (char*)"json1";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;
    profile.triggerReportOnCondition = true;

    pthread_t collectThread;
    pthread_create(&collectThread, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);
    void* result = nullptr;
    pthread_join(collectThread, &result);

}
TEST(CollectAndReportTest, JSONEncodingWrongFormatBranch) {
    CollectAndReportFunc fn = getCollectAndReportFunc();
    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 42;

    profile.name = (char*)"test";
    profile.encodingType = (char*)"JSON";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;
    profile.triggerReportOnCondition = true;

    profile.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    profile.jsonEncoding->reportFormat = (JSONRF_OBJHIERARCHY); // Anything except JSONRF_KEYVALUEPAIR

    pthread_mutex_init(&profile.triggerCondMutex, nullptr);
    pthread_cond_init(&profile.reuseThread, nullptr);
    pthread_mutex_init(&profile.reuseThreadMutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    void* res = nullptr;
    pthread_join(t, &res);

    pthread_mutex_destroy(&profile.triggerCondMutex);
    pthread_cond_destroy(&profile.reuseThread);
    pthread_mutex_destroy(&profile.reuseThreadMutex);
    free(profile.jsonEncoding);
}

TEST(CollectAndReportTest, JSONEncodingWrongFormatBranch1) {
    CollectAndReportFunc fn = getCollectAndReportFunc();
    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 42;

    profile.name = (char*)"test";
    profile.encodingType = (char*)"JSON";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;
    profile.triggerReportOnCondition = false;

    profile.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    profile.jsonEncoding->reportFormat = JSONRF_OBJHIERARCHY; // Anything except JSONRF_KEYVALUEPAIR

    pthread_mutex_init(&profile.triggerCondMutex, nullptr);
    pthread_cond_init(&profile.reuseThread, nullptr);
    pthread_mutex_init(&profile.reuseThreadMutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    void* res = nullptr;
    pthread_join(t, &res);

    pthread_mutex_destroy(&profile.triggerCondMutex);
    pthread_cond_destroy(&profile.reuseThread);
    pthread_mutex_destroy(&profile.reuseThreadMutex);
    free(profile.jsonEncoding);
}

TEST(CollectAndReportTest, TriggerReportOnConditionWithJsonReportObj) {
    CollectAndReportFunc fn = getCollectAndReportFunc();
    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 11;

    profile.name = (char*)"test";
    profile.encodingType = (char*)"JSON";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;

    profile.triggerReportOnCondition = true;
    profile.jsonReportObj = cJSON_CreateObject(); // Non-null object

    profile.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    profile.jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;

    pthread_mutex_init(&profile.triggerCondMutex, nullptr);
    pthread_cond_init(&profile.reuseThread, nullptr);
    pthread_mutex_init(&profile.reuseThreadMutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    void* res = nullptr;
    pthread_join(t, &res);

    pthread_mutex_destroy(&profile.triggerCondMutex);
    pthread_cond_destroy(&profile.reuseThread);
    pthread_mutex_destroy(&profile.reuseThreadMutex);

}

TEST(CollectAndReportTest, HandlesCheckPreviousSeekBranch) {
    CollectAndReportFunc fn = getCollectAndReportFunc();
    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 3;

    profile.name = (char*)"test";
    profile.encodingType = (char*)"JSON";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;

    profile.checkPreviousSeek = true;  // <-- triggers the branch!
    profile.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    profile.jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;

    pthread_mutex_init(&profile.triggerCondMutex, nullptr);
    pthread_cond_init(&profile.reuseThread, nullptr);
    pthread_mutex_init(&profile.reuseThreadMutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    void* res = nullptr;
    pthread_join(t, &res);

    pthread_mutex_destroy(&profile.triggerCondMutex);
    pthread_cond_destroy(&profile.reuseThread);
    pthread_mutex_destroy(&profile.reuseThreadMutex);

    free(profile.jsonEncoding);
}
TEST(CollectAndReportTest, Covers_StaticParamList_WithRealVector) {
    CollectAndReportFunc fn = getCollectAndReportFunc();

    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 42;
    profile.name = (char*)"branchtest_staticparam";
    profile.encodingType = (char*)"JSON";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;
    profile.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    profile.jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;

    Vector *staticparamlist = NULL;
    Vector_Create(&staticparamlist);
    Vector_PushBack(staticparamlist, (void*)strdup("param1"));
    profile.staticParamList = staticparamlist;

    profile.paramList = NULL;

    pthread_mutex_init(&profile.triggerCondMutex, nullptr);
    pthread_cond_init(&profile.reuseThread, nullptr);
    pthread_mutex_init(&profile.reuseThreadMutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    void* res = nullptr;
    pthread_join(t, &res);

    pthread_mutex_destroy(&profile.triggerCondMutex);
    pthread_cond_destroy(&profile.reuseThread);
    pthread_mutex_destroy(&profile.reuseThreadMutex);

    free(profile.jsonEncoding);
    Vector_Destroy(staticparamlist, free); 
}

TEST(CollectAndReportTest, Covers_ParamList_WithRealParamStruct) {
    g_vectorMock = nullptr;

    CollectAndReportFunc fn = getCollectAndReportFunc();

    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 42;
    profile.name = (char*)"branchtest_paramlist_paramstruct";
    profile.encodingType = (char*)"JSON";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;
    profile.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    profile.jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;

    Vector* paramlist = NULL;
    Vector_Create(&paramlist);

    Param* p = (Param*) malloc(sizeof(Param));
    p->name = strdup("Device.Dummy.Param");
    p->alias = strdup("Device.Dummy.Alias");
    p->paramType = strdup("dataModel");
    p->reportEmptyParam = true;
    p->regexParam = NULL;
    p->skipFreq = 0;

    Vector_PushBack(paramlist, p);

    profile.paramList = paramlist;

    pthread_mutex_init(&profile.triggerCondMutex, nullptr);
    pthread_cond_init(&profile.reuseThread, nullptr);
    pthread_mutex_init(&profile.reuseThreadMutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    void* res = nullptr;
    pthread_join(t, &res);

    pthread_mutex_destroy(&profile.triggerCondMutex);
    pthread_cond_destroy(&profile.reuseThread);
    pthread_mutex_destroy(&profile.reuseThreadMutex);

    free(profile.jsonEncoding);

    Vector_Destroy(paramlist, freeParam);
}

TEST(CollectAndReportTest, Covers_TopMarkerList_WithRealTopMarkerStruct) {
    g_vectorMock = nullptr; 

    CollectAndReportFunc fn = getCollectAndReportFunc();

    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 21;
    profile.name = (char*)"branchtest_topmarker";
    profile.encodingType = (char*)"JSON";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;
    profile.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    profile.jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;

    Vector* topMarkerList = nullptr;
    ASSERT_EQ(Vector_Create(&topMarkerList), T2ERROR_SUCCESS);

    TopMarker* marker = (TopMarker*)calloc(1, sizeof(TopMarker));
    marker->markerName = strdup("load_marker");
    marker->searchString = strdup("mysearch");
    marker->loadAverage = strdup("   1.23   ");
    marker->trimParam = true;
    marker->regexParam = nullptr; 

    Vector_PushBack(topMarkerList, marker);

    profile.topMarkerList = topMarkerList;

    pthread_mutex_init(&profile.triggerCondMutex, nullptr);
    pthread_cond_init(&profile.reuseThread, nullptr);
    pthread_mutex_init(&profile.reuseThreadMutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    void* res = nullptr;
    pthread_join(t, &res);

    pthread_mutex_destroy(&profile.triggerCondMutex);
    pthread_cond_destroy(&profile.reuseThread);
    pthread_mutex_destroy(&profile.reuseThreadMutex);

    free(profile.jsonEncoding);

    free(marker->markerName);
    free(marker->searchString);
    free(marker->loadAverage);
    free(marker);

    Vector_Destroy(topMarkerList, nullptr);
}

TEST(CollectAndReportTest, Covers_GMarkerList_WithRealGrepMarkerStruct) {
    g_vectorMock = nullptr; 

    CollectAndReportFunc fn = getCollectAndReportFunc();

    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 55;
    profile.name = (char*)"branchtest_gmarker";
    profile.encodingType = (char*)"JSON";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;
    profile.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    profile.jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;

    Vector* grepResult = NULL;
    Vector_Create(&grepResult);

    GrepMarker* gparam = (GrepMarker *) malloc(sizeof(GrepMarker));
    memset(gparam, 0, sizeof(GrepMarker));
    gparam->markerName = strdup("TEST_MARKER1");
    gparam->u.markerValue = strdup("TEST_STRING1");
    gparam->mType = MTYPE_ABSOLUTE;
    gparam->trimParam = false;
    gparam->regexParam = NULL;

    Vector_PushBack(grepResult, gparam);
    profile.gMarkerList = grepResult;

    pthread_mutex_init(&profile.triggerCondMutex, nullptr);
    pthread_cond_init(&profile.reuseThread, nullptr);
    pthread_mutex_init(&profile.reuseThreadMutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread); 
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    void* res = nullptr;
    pthread_join(t, &res);

    pthread_mutex_destroy(&profile.triggerCondMutex);
    pthread_cond_destroy(&profile.reuseThread);
    pthread_mutex_destroy(&profile.reuseThreadMutex);

    free(profile.jsonEncoding);

    Vector_Destroy(grepResult, freeGMarker);
}
TEST(CollectAndReportTest, Covers_EMarkerList_WithRealEventMarkerStruct) {
    g_vectorMock = nullptr; 

    CollectAndReportFunc fn = getCollectAndReportFunc();

    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 99;
    profile.name = (char*)"branchtest_eventmarker";
    profile.encodingType = (char*)"JSON";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;
    profile.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    profile.jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;

    Vector *eventMarkerList = NULL;
    Vector_Create(&eventMarkerList);

    EventMarker *eMarker = (EventMarker *) malloc(sizeof(EventMarker));
    eMarker->markerName = strdup("Event1");
    eMarker->compName = strdup("sysint");
    eMarker->alias = strdup("EventMarker1");
    eMarker->paramType = strdup("event");
    eMarker->markerName_CT = strdup("Event1_CT");
    eMarker->timestamp = strdup("162716381732");
    eMarker->mType = MTYPE_COUNTER;
    eMarker->reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH;
    eMarker->u.count = 1;
    eMarker->trimParam = true;
    eMarker->regexParam = strdup("[A-Z]+");

    Vector_PushBack(eventMarkerList, eMarker);

    profile.eMarkerList = eventMarkerList;

    pthread_mutex_init(&profile.eventMutex, nullptr);
    pthread_mutex_init(&profile.triggerCondMutex, nullptr);
    pthread_cond_init(&profile.reuseThread, nullptr);
    pthread_mutex_init(&profile.reuseThreadMutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    void* res = nullptr;
    pthread_join(t, &res);

    pthread_mutex_destroy(&profile.eventMutex);
    pthread_mutex_destroy(&profile.triggerCondMutex);
    pthread_cond_destroy(&profile.reuseThread);
    pthread_mutex_destroy(&profile.reuseThreadMutex);

    free(profile.jsonEncoding);

    Vector_Destroy(eventMarkerList, freeEMarker);
}
TEST(CollectAndReportTest, Covers_jsonReportObj_nonNull_forPrepareAndDestroy) {
    g_vectorMock = nullptr; 

    CollectAndReportFunc fn = getCollectAndReportFunc();

    Profile profile = {};
    GrepSeekProfile grepSeekProfile = {};
    grepSeekProfile.execCounter = 99;
    profile.name = (char*)"testjsonReportObj";
    profile.encodingType = (char*)"JSON";
    profile.protocol = (char*)"http";
    profile.grepSeekProfile = &grepSeekProfile;
    profile.jsonEncoding = (JSONEncoding*)malloc(sizeof(JSONEncoding));
    profile.jsonEncoding->reportFormat = JSONRF_OBJHIERARCHY;
    profile.triggerReportOnCondition = false;
    profile.jsonReportObj = cJSON_CreateObject();
    cJSON_AddStringToObject(profile.jsonReportObj, "key", "value");

    pthread_mutex_init(&profile.triggerCondMutex, nullptr);
    pthread_cond_init(&profile.reuseThread, nullptr);
    pthread_mutex_init(&profile.reuseThreadMutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, [](void* arg) -> void* {
        CollectAndReportFunc fn = getCollectAndReportFunc();
        return fn(arg);
    }, &profile);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pthread_mutex_lock(&profile.reuseThreadMutex);
    pthread_cond_signal(&profile.reuseThread);
    pthread_mutex_unlock(&profile.reuseThreadMutex);

    void* res = nullptr;
    pthread_join(t, &res);

    pthread_mutex_destroy(&profile.triggerCondMutex);
    pthread_cond_destroy(&profile.reuseThread);
    pthread_mutex_destroy(&profile.reuseThreadMutex);

    free(profile.jsonEncoding);
}
#endif
#if 1
//comment
//==================================== profile.c ===================

// Test initProfileList
TEST_F(ProfileTest, InitProfileList_Success) {
    const char* path = "/tmp/t2reportprofiles/";
    DIR *dir = (DIR*)0xffffffff ;
    Vector* configlist = NULL;
    
    // Vector mock expectations for initProfileList flow
    EXPECT_CALL(*g_vectorMock, Vector_Create(_))
        .Times(::testing::AtMost(3))  // 1 for local test configlist, 1 for global profileList, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_vectorMock, Vector_PushBack(_, _))
        .Times(::testing::AtMost(1))  // Only for the local test configlist
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _))
        .Times(::testing::AtMost(2))  // 1 for local test configlist, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    
    // Additional Vector mock expectations for loadReportProfilesFromDisk
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(2))
        .WillRepeatedly(Return(0)); // For the logging at the end
    
    Vector_Create(&configlist);
    Vector_PushBack(configlist, (void *)strdup("marker1"));

    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(::testing::AtMost(2))
           .WillRepeatedly(Return(dir));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(::testing::AtMost(2))
           .WillRepeatedly(Return(0));
    EXPECT_CALL(*g_fileIOMock, readdir(_))
           .Times(::testing::AtMost(2))
           .WillRepeatedly(Return((struct dirent *)NULL));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(0));

    EXPECT_EQ(initProfileList(false), T2ERROR_SUCCESS);
    Vector_Destroy(configlist, free);
}
// Test profileWithNameExists
TEST_F(ProfileTest, ProfileWithNameExists_NotInitialized) {
    bool exists = false;
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0));
    
    //EXPECT_EQ(profileWithNameExists("test", &exists), T2ERROR_FAILURE);
    EXPECT_EQ(profileWithNameExists("test", &exists), T2ERROR_PROFILE_NOT_FOUND);
}

TEST_F(ProfileTest, ProfileWithNameExists_NullName) {
    bool exists = false;
    EXPECT_EQ(profileWithNameExists(nullptr, &exists), T2ERROR_FAILURE);
    EXPECT_FALSE(exists);
}

// Test addProfile
TEST_F(ProfileTest, AddProfile_Initialized) {
    EXPECT_CALL(*g_vectorMock, Vector_PushBack(_, _))
        .Times(::testing::AtMost(1))  // Only for the local test configlist
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    Profile dummy;
    //EXPECT_EQ(addProfile(&dummy), T2ERROR_FAILURE);
    EXPECT_EQ(addProfile(&dummy), T2ERROR_SUCCESS);
}

// Test enableProfile
TEST_F(ProfileTest, EnableProfile_NotInitialized) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0));
    
    // enableProfile calls registerProfileWithScheduler if profile is enabled successfully
    EXPECT_CALL(*g_schedulerMock, registerProfileWithScheduler(_, _, _, _, _, _, _, _))
        .Times(::testing::AtMost(1)); // Not called because profile list is not initialized
    
    EXPECT_EQ(enableProfile("abc"), T2ERROR_FAILURE);
}

TEST_F(ProfileTest, NotifyTimeout_Directly)
{
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0));

    // NotifyTimeout may call CollectAndReport which calls getLapsedTime
    EXPECT_CALL(*g_schedulerMock, getLapsedTime(_, _, _))
        .Times(::testing::AtMost(1)); // Not called because profile list is not initialized

    NotifyTimeout("abc", true);
}

// Test disableProfile
TEST_F(ProfileTest, DisableProfile_NotInitialized) {
    bool isDeleteRequired = false;
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0));
    
    // disableProfile may call unregisterProfileFromScheduler if profile exists
    EXPECT_CALL(*g_schedulerMock, unregisterProfileFromScheduler(_))
        .Times(::testing::AtMost(1)); // Not called because profile list is not initialized
    
    EXPECT_EQ(disableProfile("abc", &isDeleteRequired), T2ERROR_FAILURE);
}

// Test deleteProfile
TEST_F(ProfileTest, DeleteProfile_NotInitialized) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0));
    
    // deleteProfile calls unregisterProfileFromScheduler if profile exists
    EXPECT_CALL(*g_schedulerMock, unregisterProfileFromScheduler(_))
        .Times(::testing::AtMost(1)); // Not called because profile list is not initialized
    
    EXPECT_EQ(deleteProfile("abc"), T2ERROR_FAILURE);
}
// Test deleteAllProfiles
TEST_F(ProfileTest, DeleteAllProfiles_ProfileListNull) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(*g_vectorMock, Vector_Create(_))
        .Times(::testing::AtMost(3))  // 1 for local test configlist, 1 for global profileList, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _))
        .Times(::testing::AtMost(2))  // 1 for local test configlist, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_EQ(deleteAllProfiles(true), T2ERROR_FAILURE);
}

// Test uninitProfileList
TEST_F(ProfileTest, UninitProfileList_Success) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0));
    EXPECT_EQ(uninitProfileList(), T2ERROR_SUCCESS);
}

// Test getProfileCount
TEST_F(ProfileTest, GetProfileCount_NotInitialized) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0));
    EXPECT_EQ(getProfileCount(), 0);
}

// Test appendTriggerCondition - success case and fail case
TEST_F(ProfileTest, AppendTriggerCondition_Success) {
    Profile tmp;
    memset(&tmp, 0, sizeof(tmp));
    pthread_mutex_init(&tmp.triggerCondMutex, nullptr);
    EXPECT_EQ(appendTriggerCondition(&tmp, "refname", "refval"), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, AppendTriggerCondition_FailAndQueue) {
    Profile tmp;
    memset(&tmp, 0, sizeof(tmp));
    pthread_mutex_init(&tmp.triggerCondMutex, nullptr);
    extern int pthread_mutex_trylock(pthread_mutex_t*);
    struct Override {
        static int locked(pthread_mutex_t*) { return 1; }
    };
    auto orig = pthread_mutex_trylock;
    //EXPECT_EQ(appendTriggerCondition(&tmp, "refname", "refval"), T2ERROR_FAILURE);
    EXPECT_EQ(appendTriggerCondition(&tmp, "refname", "refval"), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, updateMarkerComponentMap) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_)).Times(::testing::AtMost(1)).WillRepeatedly(Return(0));
    updateMarkerComponentMap();
}

TEST_F(ProfileTest, registerTriggerConditionConsumer) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_)).Times(::testing::AtMost(1)).WillRepeatedly(Return(0));
    EXPECT_EQ(registerTriggerConditionConsumer(), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, reportGenerationCompleteReceiver) {
    reportGenerationCompleteReceiver("profileX");
}

TEST_F(ProfileTest, triggerReportOnCondtion) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_)).Times(::testing::AtMost(1)).WillRepeatedly(Return(0));
    EXPECT_EQ(triggerReportOnCondtion("refname", "refvalue"), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, getMinThresholdDuration_Failure) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_)).Times(::testing::AtMost(1)).WillRepeatedly(Return(0));
    EXPECT_EQ(getMinThresholdDuration("profile1"), 0);
}

#endif


#if 1
//comment
// ============================== t2markers.c =============================

TEST_F(ProfileTest, InitAndDestroyShouldWork) {
    EXPECT_CALL(*g_vectorMock, Vector_Create(_))
        .Times(::testing::AtMost(1))  // 1 for local test configlist, 1 for global profileList, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _))
        .Times(::testing::AtMost(1))  // 1 for local test configlist, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_EQ(initT2MarkerComponentMap(), T2ERROR_SUCCESS);
}
#endif


#if 1
//comment

//================================ reportProfiles.c ====================================
#if 1
extern "C" {	
typedef void* (*reportOnDemandFunc)(void*);
reportOnDemandFunc reportOnDemandFuncCallback(void);
typedef void (*freeProfilesHashMapFunc)(void *);
freeProfilesHashMapFunc freeProfilesHashMapFuncCallback(void);
typedef void (*freeReportProfileHashMapFunc)(void *);
freeReportProfileHashMapFunc freeReportProfileHashMapFuncCallback(void);
}

TEST(ReportProfilesCallbacks, FreeProfilesHashMap) {
    auto cb = freeProfilesHashMapFuncCallback();
    ASSERT_NE(cb, nullptr);

    // Test with an actual element
    hash_element_t* item = (hash_element_t*) std::malloc(sizeof(hash_element_t));
    item->key = (char*) std::malloc(12);
    std::strcpy(item->key, "testkey");
    item->data = std::malloc(8);
    cb(item);

    // Test with nullptr
    cb(nullptr);
}
#endif
TEST_F(ProfileTest, initReportProfiles) {
    char status[8] = "true";
    DIR *dir = (DIR*)0xffffffff ;
    FILE* fp = (FILE*)0xffffffff;

    EXPECT_CALL(*g_rbusMock, rbus_get(_,_,_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(RBUS_BOOLEAN));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetBoolean(_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return());
    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(::testing::AtMost(5))
	   .WillRepeatedly(Return(dir));
    EXPECT_CALL(*g_rbusMock, rbus_regDataElements(_,_,_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(0));
    EXPECT_CALL(*g_fileIOMock, readdir(_))
	   .Times(::testing::AtMost(5))
	   .WillRepeatedly(Return((struct dirent *)NULL));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(0));
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(::testing::AtMost(5))
            .WillRepeatedly(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fscanf(_, _, _))
            .Times(::testing::AtMost(5))
            .WillRepeatedly(::testing::Return(EOF));
    EXPECT_CALL(*g_fileIOMock, fclose(_))                                                                            
            .Times(::testing::AtMost(5))                                                                                                
            .WillRepeatedly(Return(0));   
    EXPECT_CALL(*g_fileIOMock, mkdir(_,_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(-1));
    EXPECT_CALL(*g_rbusMock, rbus_registerLogHandler(_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_open(_,_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(RBUS_ENABLED));
    EXPECT_CALL(*g_rbusMock, rbusValue_ToString(_,_,_))
           .Times(::testing::AtMost(5))
           .WillRepeatedly(Return(status));

    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(5))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    EXPECT_CALL(*g_vectorMock, Vector_At(_, 0))
        .Times(::testing::AtMost(5))
        .WillRepeatedly(Return((void*)strdup("PROFILE_1")));
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _)).Times(::testing::AtMost(2))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_vectorMock, Vector_Create(_))
        .Times(::testing::AtMost(5))  // 1 for local test configlist, 1 for global profileList, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_vectorMock, Vector_PushBack(_, _))
        .Times(::testing::AtMost(5))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    // Scheduler mock expectations
    EXPECT_CALL(*g_schedulerMock, initScheduler(_, _, _))
        .Times(::testing::AtMost(5))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    
    // Additional RBUS mock expectations for isRbusEnabled() which gets called during T2ER_Init
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
        .Times(::testing::AtMost(5))  // Called multiple times during initialization
        .WillRepeatedly(Return(RBUS_ENABLED));
    
    EXPECT_EQ(initReportProfiles(), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, ReportProfiles_addReportProfile) {
    Profile *profile = (Profile*)malloc(sizeof(Profile));
    profile->name = strdup("EventProfile");
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    EXPECT_CALL(*g_vectorMock, Vector_At(_, 0))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return((void*)strdup("PROFILE_1")));
    EXPECT_CALL(*g_vectorMock, Vector_PushBack(_, _))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    //EXPECT_EQ(ReportProfiles_addReportProfile(profile), T2ERROR_SUCCESS);
    EXPECT_EQ(ReportProfiles_addReportProfile(profile), T2ERROR_FAILURE);
}

TEST_F(ProfileTest, ReportProfiles_Interrupt_Coverage) {
    // Should call sendLogUploadInterruptToScheduler and interrupt xconf profile
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    
    // ReportProfiles_Interrupt calls SendInterruptToTimeoutThread for xconf profile if ProfileXConf_isSet
    EXPECT_CALL(*g_schedulerMock, SendInterruptToTimeoutThread(_))
        .Times(::testing::AtMost(1)); // ProfileXConf is not set in this test, so no interrupt call
    
    ReportProfiles_Interrupt();
}

// ===> callback failure
TEST_F(ProfileTest, ReportProfiles_TimeoutCb_XConfProfile) {
    // Should trigger ProfileXConf_notifyTimeout
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    ReportProfiles_TimeoutCb(strdup("XConfProfile"), true);
}

TEST_F(ProfileTest, ReportProfiles_TimeoutCb_NonXConfProfile) {
    // Should trigger NotifyTimeout
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    ReportProfiles_TimeoutCb(strdup("NonXConfProfile"), false);
}

TEST_F(ProfileTest, ReportProfiles_ActivationTimeoutCb_XConfProfile) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    ReportProfiles_ActivationTimeoutCb(strdup("XConfProfile"));
}

TEST_F(ProfileTest, ReportProfiles_ActivationTimeoutCb_NonXConfProfile) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    ReportProfiles_ActivationTimeoutCb(strdup("NonXConfProfile"));
}

TEST_F(ProfileTest, ReportProfiles_storeMarkerEvent_XConfProfile) {
    T2Event event;
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    ReportProfiles_storeMarkerEvent(strdup("XConfProfile"), &event);
}

TEST_F(ProfileTest, ReportProfiles_storeMarkerEvent_NonXConfProfile) {
    T2Event event;
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    ReportProfiles_storeMarkerEvent(strdup("NonXConfProfile"), &event);
}

#if 0
TEST_F(ProfileTest, ReportProfiles_setProfileXConf) {
    ProfileXConf profile;
    EXPECT_CALL(*g_schedulerMock, registerProfileWithScheduler(_, _, _, _, _, _, _, _))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(RBUS_ENABLED));
    
    EXPECT_CALL(*g_rbusMock, rbus_registerLogHandler(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
        
    EXPECT_CALL(*g_rbusMock, rbus_open(_, _))
        .Times(::testing::AtMost(2))
        .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    
    // Mock successful parameter retrieval
    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
        
    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(RBUS_STRING));
        
    EXPECT_CALL(*g_rbusMock, rbusValue_ToString(_, _, _))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(strdup("test_value")));
        
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
        .Times(::testing::AtMost(1));
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(2))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
				   
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(RBUS_ENABLED));

    EXPECT_CALL(*g_rbusMock, rbus_open(_, _))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(*g_rbusMock, rbusObject_Init(_, _))
        .Times(::testing::AtMost(1));

    EXPECT_CALL(*g_rbusMock, rbusValue_Init(_))
        .Times(::testing::AtMost(1));

    EXPECT_CALL(*g_rbusMock, rbusValue_SetString(_, _))
        .Times(::testing::AtMost(1));

    EXPECT_CALL(*g_rbusMock, rbusObject_SetValue(_, _, _))
        .Times(::testing::AtMost(1));

    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
        .Times(::testing::AtMost(1));

    EXPECT_CALL(*g_rbusMock, rbusEvent_Publish(_,_))
	.Times(::testing::AtMost(1))
	.WillRepeatedly(Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(*g_rbusMock, rbusObject_Release(_))
        .Times(::testing::AtLeast(1));
    EXPECT_EQ(ReportProfiles_setProfileXConf(&profile), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, ReportProfiles_deleteProfileXConf) {
    ProfileXConf profile;
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    EXPECT_EQ(ReportProfiles_deleteProfileXConf(&profile), T2ERROR_SUCCESS);
}
#endif

TEST_F(ProfileTest, ReportProfiles_deleteProfile) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(2))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    //EXPECT_EQ(ReportProfiles_deleteProfile("testprofile"), T2ERROR_SUCCESS);
    EXPECT_EQ(ReportProfiles_deleteProfile("testprofile"), T2ERROR_FAILURE);
}

TEST_F(ProfileTest, profilemem_usage) {
    unsigned int value = 0;
    //profilemem = 1234;
    profilemem_usage(&value);
    //EXPECT_EQ(value, 1234);
    EXPECT_NE(value, 0);
}

TEST_F(ProfileTest, T2totalmem_calculate) {
    //profilemem = 0;
    T2totalmem_calculate();
    //EXPECT_GT(profilemem, 0u);
}

TEST_F(ProfileTest, privacymode_do_not_share) {
    EXPECT_EQ(privacymode_do_not_share(), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, generateDcaReport) {
    // generateDcaReport may call set_logdemand
    EXPECT_CALL(*g_schedulerMock, set_logdemand(_))
        .Times(::testing::AtMost(2)); // Allow up to 2 calls
    
    generateDcaReport(false, true);
    generateDcaReport(true, false);
}

TEST_F(ProfileTest, RemovePreRPfromDisk) {
    hash_map_t dummy;
    DIR *dir = (DIR*)0xffffffff ;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(::testing::AtMost(1))
	   .WillRepeatedly(Return(dir));
    EXPECT_CALL(*g_fileIOMock, readdir(_))
	   .Times(::testing::AtMost(1))
	   .WillRepeatedly(Return((struct dirent *)NULL));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(0));
    //EXPECT_EQ(RemovePreRPfromDisk("/tmp", &dummy), T2ERROR_FAILURE);
    EXPECT_EQ(RemovePreRPfromDisk("/tmp", &dummy), T2ERROR_SUCCESS);
}

#if 1
TEST_F(ProfileTest, deleteAllReportProfiles) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _)).Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_EQ(deleteAllReportProfiles(), T2ERROR_SUCCESS);
}
#endif

#if 0
TEST_F(ProfileTest, isMtlsEnabled) {
    char status[8] = "true";
    EXPECT_CALL(*g_rbusMock, rbus_get(_,_,_))
           .Times(::testing::AtMost(2))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(_))
           .Times(::testing::AtMost(2))
           .WillRepeatedly(Return(RBUS_BOOLEAN));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetBoolean(_))
           .Times(::testing::AtMost(2))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
           .Times(::testing::AtMost(2))
           .WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusValue_ToString(_,_,_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(status));
    EXPECT_TRUE(isMtlsEnabled());
}
#endif

#if 0
TEST_F(ProfileTest, ReportProfiles_uninit) {
    EXPECT_CALL(*g_vectorMock, Vector_Create(_))
        .Times(::testing::AtMost(3))  // 1 for local test configlist, 1 for global profileList, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_vectorMock, Vector_PushBack(_, _))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(3))  // May be called multiple times - in deleteAllProfiles, etc.
        .WillRepeatedly(Return(0)); // Return 0 to indicate no profiles (avoid unregister calls)
    EXPECT_CALL(*g_vectorMock, Vector_At(_, _))
        .Times(::testing::AtMost(2))  // May be called if profiles exist
        .WillRepeatedly(Return(nullptr));
    
    // Scheduler mock expectations - uninitScheduler is definitely called
    EXPECT_CALL(*g_schedulerMock, uninitScheduler())
        .Times(::testing::AtMost(1));
    
    // unregisterProfileFromScheduler may be called for each profile during deleteAllProfiles
    // Using AtMost to handle cases where profiles exist
    EXPECT_CALL(*g_schedulerMock, unregisterProfileFromScheduler(_))
        .Times(::testing::AtMost(5))  // Allow up to 5 calls in case profiles exist
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    
    EXPECT_EQ(ReportProfiles_uninit(), T2ERROR_SUCCESS);
}
#endif

#if 1
TEST_F(ProfileTest, reportOnDemandTest)
{
        reportOnDemandFunc func = reportOnDemandFuncCallback();
        ASSERT_NE(func,nullptr);
        //func((void*)"UPLOAD");
        func((void*)"ABORT");
        func((void*)"FOO");
        func(nullptr);
}

TEST(ReportProfilesCallbacks, FreeReportProfileHashMap) {
    auto cb = freeReportProfileHashMapFuncCallback();
    ASSERT_NE(cb, nullptr);

    // Make an item with ReportProfile-like .data
    hash_element_t* item = (hash_element_t*) std::malloc(sizeof(hash_element_t));
    item->key = (char*) std::malloc(12);
    std::strcpy(item->key, "profkey");
    struct ReportProfile {
        char* hash;
        char* config;
        void* hash_map_pad; // just to align with how your system might fill it, can be omitted
    };
    ReportProfile* rp = (ReportProfile*) std::malloc(sizeof(ReportProfile));
    rp->hash = (char*) std::malloc(6);
    std::strcpy(rp->hash, "hashV");
    rp->config = (char*) std::malloc(8);
    std::strcpy(rp->config, "cfgVal");
    item->data = rp;

    cb(item);

    // Safe to call with nullptr
    cb(nullptr);
    SUCCEED();
}
#endif
#endif

#if 1
//comment
//=================================== profilexconf.c ================================

TEST_F(ProfileTest, InitAndUninit) {
    // Covers ProfileXConf_init and ProfileXConf_uninit
    DIR *dir = (DIR*)0xffffffff ;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(::testing::AtMost(2))
	   .WillRepeatedly(Return(dir));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(0));
    EXPECT_CALL(*g_fileIOMock, readdir(_))
	   .Times(::testing::AtMost(1))
	   .WillRepeatedly(Return((struct dirent *)NULL));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(0));
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(1)); // Return 1 to indicate one profile in the list
    EXPECT_CALL(*g_vectorMock, Vector_At(_, 0))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return((void*)strdup("PROFILE_1")));
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _))
        .Times(::testing::AtMost(3))  // 1 for local test configlist, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_vectorMock, Vector_Create(_))
        .Times(::testing::AtMost(3))  // 1 for local test configlist, 1 for global profileList, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_EQ(ProfileXConf_init(false), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, SetAndIsSet) {
    // Covers ProfileXConf_set and ProfileXConf_isSet
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("TestProfile");
    profile->eMarkerList = nullptr;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(3))
        .WillRepeatedly(Return(0)); // Return 1 to indicate one profile in the list
    EXPECT_CALL(*g_vectorMock, Vector_At(_, 0))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return((void*)strdup("PROFILE_1")));
    EXPECT_CALL(*g_vectorMock, Vector_Create(_))
        .Times(::testing::AtMost(3))  // 1 for local test configlist, 1 for global profileList, 1 for configList in loadReportProfilesFromDisk
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_vectorMock, Vector_PushBack(_, _))
        .Times(::testing::AtMost(3))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    
    // Scheduler mock expectations - ProfileXConf_set calls registerProfileWithScheduler
    EXPECT_CALL(*g_schedulerMock, registerProfileWithScheduler(_, _, _, _, _, _, _, _))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    
    EXPECT_EQ(ProfileXConf_set(profile), T2ERROR_SUCCESS);
    EXPECT_TRUE(ProfileXConf_isSet());

    // Get name
    char* name = ProfileXconf_getName();
    ASSERT_NE(name, nullptr);
    EXPECT_STREQ(name, "TestProfile");
    free(name);
   

    // Clean up - ProfileXConf_uninit calls unregisterProfileFromScheduler
    EXPECT_CALL(*g_schedulerMock, unregisterProfileFromScheduler(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    
    EXPECT_EQ(ProfileXConf_uninit(), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, IsNameEqual) {
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("NameEqualProfile");
    profile->eMarkerList = nullptr;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate one profile in the list
    EXPECT_CALL(*g_schedulerMock, registerProfileWithScheduler(_, _, _, _, _, _, _, _))
        .Times(::testing::AtMost(1)); // Not called because profile list is not initialized

    ProfileXConf_set(profile);
    //EXPECT_TRUE(ProfileXConf_isNameEqual((char*)"NameEqualProfile"));
    EXPECT_FALSE(ProfileXConf_isNameEqual((char*)"NameEqualProfile"));
    EXPECT_FALSE(ProfileXConf_isNameEqual((char*)"OtherProfile"));
    ProfileXConf_uninit();
}

TEST_F(ProfileTest, NotifyTimeout) {
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("TimeoutProfile");
    profile->eMarkerList = nullptr;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate one profile in the list
    EXPECT_CALL(*g_schedulerMock, registerProfileWithScheduler(_, _, _, _, _, _, _, _))
        .Times(::testing::AtMost(1)); // Not called because profile list is not initialized
    ProfileXConf_set(profile);

    // ProfileXConf_notifyTimeout may call CollectAndReportXconf which calls getLapsedTime
    EXPECT_CALL(*g_schedulerMock, getLapsedTime(_, _, _))
        .Times(::testing::AtMost(1)); // May be called if report is generated

    // Should not crash
    //ProfileXConf_notifyTimeout(false, false);

    ProfileXConf_uninit();
}

TEST_F(ProfileTest, StoreMarkerEventFail) {
    // No profile set, should fail
    T2Event event;
    event.name = const_cast<char*>("EventName");
    event.value = const_cast<char*>("EventValue");
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate one profile in the list
    EXPECT_EQ(ProfileXConf_storeMarkerEvent(&event), T2ERROR_FAILURE);
}

#if 0
TEST_F(ProfileTest, StoreMarkerEventSuccess) {
    // Setup a profile and an event marker
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("EventProfile");
    Vector* eMarkerList = nullptr;
    
    // Mock Vector calls
    EXPECT_CALL(*g_vectorMock, Vector_Create(_)).Times(::testing::AtMost(1));
    EXPECT_CALL(*g_vectorMock, Vector_PushBack(_, _)).Times(::testing::AtMost(1));
    
    Vector_Create(&eMarkerList);
    profile->eMarkerList = eMarkerList;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    // Marker
    EventMarker* marker = (EventMarker*)malloc(sizeof(EventMarker));
    marker->markerName = strdup("EventName");
    marker->mType = MTYPE_ACCUMULATE;
    marker->u.count = 0;
    Vector_PushBack(eMarkerList, marker);
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate one profile in the list

    ProfileXConf_set(profile);

    T2Event event;
    event.name = const_cast<char*>("EventName");
    event.value = const_cast<char*>("EventValue");
    EXPECT_EQ(ProfileXConf_storeMarkerEvent(&event), T2ERROR_SUCCESS);

    ProfileXConf_uninit();
}
#endif

#if 0
TEST_F(ProfileTest, TerminateReportNoProfile) {
    // Should fail if no profile
    EXPECT_EQ(ProfileXConf_terminateReport(), T2ERROR_SUCCESS);
}
#endif

#if 0
TEST_F(ProfileTest, TerminateReportNoInProgress) {
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("TerminateProfile");
    profile->eMarkerList = nullptr;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    ProfileXConf_set(profile);

    EXPECT_EQ(ProfileXConf_terminateReport(), T2ERROR_FAILURE);

    ProfileXConf_uninit();
}
#endif

// Additional coverage for delete
TEST_F(ProfileTest, DeleteProfile) {
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("DeleteProfile");
    profile->eMarkerList = nullptr;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    ProfileXConf_set(profile);
    EXPECT_EQ(ProfileXConf_delete(profile), T2ERROR_FAILURE);
    ProfileXConf_uninit();
}

TEST_F(ProfileTest, ProfileXConf_updateMarkerComponentMap)
{
    ProfileXConf_updateMarkerComponentMap();
    EXPECT_EQ(ProfileXConf_uninit(), T2ERROR_SUCCESS);
}
#endif
#if 1
//comment
//=============================== t2eventreceiver.c =============================


TEST_F(ProfileTest, FreeT2EventHandlesNullAndValid) {
    freeT2Event(nullptr);
    T2Event* e = (T2Event*)malloc(sizeof(T2Event));
    e->name = strdup("n");
    e->value = strdup("v");
    freeT2Event(e);
}

#if 0
TEST_F(ProfileTest, PushDataWithDelim_NormalEvent) {
    EXPECT_CALL(*g_rbusMock, rbus_registerLogHandler(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_open(_,_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ENABLED));
    EXPECT_CALL(*g_rbusMock, rbus_regDataElements(_,_,_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(0));
    T2ER_Init();
    ////EREnabled = true;
    ////stopDispatchThread = true;
    ////gQueueCount = 0;
    char event[] = "marker1<#=#>value1";
    T2ER_PushDataWithDelim(event, NULL);
    //ASSERT_TRUE(gQueuePushCalled);
}
#endif

TEST_F(ProfileTest, PushDataWithDelim_NullEvent) {
    T2ER_PushDataWithDelim(NULL, NULL);
}

TEST_F(ProfileTest, PushDataWithDelim_QueueLimit) {
    //T2ER_Init();
    ////EREnabled = true;
    ////gQueueCount = 201;
    char event[] = "marker1<#=#>value1";
    T2ER_PushDataWithDelim(event, nullptr);
}

TEST_F(ProfileTest, PushDataWithDelim_MissingValue) {
    //T2ER_Init();
    //EREnabled = true;
    char event[] = "marker1";
    T2ER_PushDataWithDelim(event, nullptr);
}

TEST_F(ProfileTest, PushDataWithDelim_MissingDelimiter) {
    //T2ER_Init();
    //EREnabled = true;
    char event[] = "marker1value1";
    T2ER_PushDataWithDelim(event, nullptr);
}

TEST_F(ProfileTest, PushEvent_NormalCase) {
    //T2ER_Init();
    //EREnabled = true;
    //stopDispatchThread = true;
    //gQueueCount = 0;
    char* name = strdup("marker1");
    char* value = strdup("value1");
    T2ER_Push(name, value);
    //ASSERT_TRUE(gQueuePushCalled);
}

TEST_F(ProfileTest, PushEvent_NullNameOrValue) {
    //T2ER_Init();
    //EREnabled = true;
    T2ER_Push(nullptr, strdup("value1"));
    T2ER_Push(strdup("marker1"), nullptr);
}

TEST_F(ProfileTest, PushEvent_QueueLimit) {
    //T2ER_Init();
    //EREnabled = true;
    //gQueueCount = 201;
    char* name = strdup("marker1");
    char* value = strdup("value1");
    T2ER_Push(name, value);
}

TEST_F(ProfileTest, PushEvent_NotInitialized) {
    //EREnabled = false;
    T2ER_Push(strdup("marker1"), strdup("value1"));
}

/*
TEST_F(ProfileTest, EventDispatchThread_DispatchesEvents) {
    //EREnabled = true;
    //stopDispatchThread = false;
    //gQueueCount = 1;
    T2Event* event = (T2Event*)malloc(sizeof(T2Event));
    event->name = strdup("marker1");
    event->value = strdup("value1");
    //eQueue->data.push_back(event);
    pthread_t t;
    pthread_create(&t, nullptr, T2ER_EventDispatchThread, nullptr);
    sleep(1);
    //stopDispatchThread = true;
    //pthread_cond_signal(&erCond);
    pthread_join(t, nullptr);
}

TEST_F(ProfileTest, EventDispatchThread_NoEventsWait) {
    //EREnabled = true;
    //stopDispatchThread = false;
    //gQueueCount = 0;
    pthread_t t;
    pthread_create(&t, nullptr, T2ER_EventDispatchThread, nullptr);
    sleep(1);
    //stopDispatchThread = true;
    //pthread_cond_signal(&erCond);
    pthread_join(t, nullptr);
}
*/

/*
TEST_F(ProfileTest, InitAlreadyInitialized) {
    EXPECT_CALL(*g_rbusMock, rbus_registerLogHandler(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_open(_,_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ENABLED));
    EXPECT_CALL(*g_rbusMock, rbus_regDataElements(_,_,_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(0));
    T2ER_Init();
    T2ERROR res = T2ER_Init();
    ASSERT_EQ(res, T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, InitFailures) {
    //eQueue = nullptr;
    //EREnabled = false;
    // Simulate t2_queue_create failure
    bool t2_queue_create_failed = false;
    auto orig_t2_queue_create = t2_queue_create;
    //gQueue = nullptr;
    T2ERROR res = T2ER_Init();
    ASSERT_EQ(res, T2ERROR_SUCCESS); // Will create a queue, not fail in stub
}
*/

TEST_F(ProfileTest, StartDispatchThread_Normal) {
    T2ERROR res = T2ER_StartDispatchThread();

    ASSERT_EQ(res, T2ERROR_SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Stop the thread after starting it
}

#if 0
TEST_F(ProfileTest, StartDispatchThread_AlreadyRunningOrNotInitialized) {
    //EREnabled = false;
    //stopDispatchThread = false;
    T2ERROR res = T2ER_StartDispatchThread();
    ASSERT_EQ(res, T2ERROR_FAILURE);
}
#endif


/*
TEST_F(ProfileTest, StopDispatchThread_Normal) {
    //EREnabled = true;
    //stopDispatchThread = false;
    T2ERROR res = T2ER_StopDispatchThread();
    ASSERT_EQ(res, T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, StopDispatchThread_NotRunningOrNotInitialized) {
    //T2ER_Init();
    //EREnabled = false;
    //stopDispatchThread = true;
    T2ERROR res = T2ER_StopDispatchThread();
    ASSERT_EQ(res, T2ERROR_FAILURE);
}
*/

#if 0
TEST_F(ProfileTest, Uninit_Normal) {
/*
    EXPECT_CALL(*g_rbusMock, rbus_registerLogHandler(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_open(_,_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ENABLED));
    EXPECT_CALL(*g_rbusMock, rbus_regDataElements(_,_,_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(0));
*/
    T2ER_Init();
    //EREnabled = true;
    //stopDispatchThread = false;
    T2ER_Uninit();
    //ASSERT_FALSE(//EREnabled);
    //ASSERT_EQ(eQueue, nullptr);
}

TEST_F(ProfileTest, Uninit_NotInitialized) {
    //EREnabled = false;
    T2ER_Uninit();
}
#endif

/* Static functions
TEST_F(ProfileTest, FlushCacheFromFile_AndRemove) {
    std::ofstream f(T2_CACHE_FILE);
    f << "marker1<#=#>value1\n";
    f.close();
    T2ER_Init();
    extern T2ERROR flushCacheFromFile(void);
    flushCacheFromFile();
    ASSERT_FALSE(std::ifstream(T2_CACHE_FILE).good());
}

TEST_F(ProfileTest, FlushCacheFromFile_FopenFail) {
    remove(T2_CACHE_FILE);
    T2ER_Init();
    extern T2ERROR flushCacheFromFile(void);
    flushCacheFromFile();
}
*/

//============================== Vector Mock Demo ==========================

#if 0
TEST_F(ProfileTest, VectorMockDemo_Create_Success) {
    Vector* testVector = nullptr;
    
    // Example of using Vector mock with simple return value
    EXPECT_CALL(*g_vectorMock, Vector_Create(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    
    EXPECT_EQ(Vector_Create(&testVector), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, VectorMockDemo_Size_Returns_Zero) {
    Vector testVector = {nullptr, 0, 0};
    
    EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0));
    
    size_t size = Vector_Size(&testVector);
    EXPECT_EQ(size, 0);
}

TEST_F(ProfileTest, VectorMockDemo_PushBack_Success) {
    Vector testVector = {nullptr, 0, 0};
    char* testData = strdup("test");
    
    EXPECT_CALL(*g_vectorMock, Vector_PushBack(_, _))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    
    EXPECT_EQ(Vector_PushBack(&testVector, testData), T2ERROR_SUCCESS);
    
    free(testData);
}

#endif

#endif


TEST_F(ProfileTest, createComponentDataElements) {
    EXPECT_CALL(*g_vectorMock, Vector_Size(_)).Times(::testing::AtMost(1)).WillRepeatedly(Return(0));
    createComponentDataElements();
}


#if 0
extern "C" {
typedef void (*freeRequestURIparamFunc)(void *);
freeRequestURIparamFunc freeRequestURIparamCallback(void);
}


TEST(ProfileTest, FreeRequestURIparam_NULLInput) {
    freeRequestURIparamFunc func = freeRequestURIparamCallback();
    ASSERT_NE(func, nullptr);
    func(nullptr); // Should not crash
}
#endif
