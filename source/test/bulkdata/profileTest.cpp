#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <pthread.h>
#include <string>
#include <vector>
#include "profile.h"

// Mocks and stubs for dependencies
extern "C" {
#include "busInterface.h"

// Minimal stubs (add as needed for compilation, can be extended for more coverage)
/*
T2ERROR Vector_Create(Vector **v) { *v = (Vector*)calloc(1, sizeof(Vector)); return T2ERROR_SUCCESS; }
void Vector_Destroy(Vector* v, void (*free_fn)(void *)) { if(v) free(v); }
size_t Vector_Size(const Vector* v) { return v ? v->count : 0; }
void* Vector_At(const Vector* v, size_t idx) { return nullptr; }
void Vector_PushBack(Vector* v, void* item) {}
bool Vector_RemoveItem(Vector* v, void* item, void (*free_fn)(void*)) { return true; }
hash_map_t* hash_map_create() { return (hash_map_t*)calloc(1, sizeof(hash_map_t)); }
void hash_map_put(hash_map_t*, char*, char*, void(*)(void*)) {}
void hash_map_destroy(hash_map_t *m, void(*free_fn)(void*)) { if(m) free(m); }
void T2Debug(const char*, ...) {}
void T2Error(const char*, ...) {}
void T2Info(const char*, ...) {}
void T2Warning(const char*, ...) {}

int pthread_mutex_init(pthread_mutex_t* m, const pthread_mutexattr_t*) { *m = PTHREAD_MUTEX_INITIALIZER; return 0; }
int pthread_mutex_destroy(pthread_mutex_t* m) { return 0; }
int pthread_mutex_lock(pthread_mutex_t* m) { return 0; }
int pthread_mutex_unlock(pthread_mutex_t* m) { return 0; }
int pthread_cond_init(pthread_cond_t* c, const pthread_condattr_t*) { *c = PTHREAD_COND_INITIALIZER; return 0; }
int pthread_cond_destroy(pthread_cond_t* c) { return 0; }
int pthread_cond_wait(pthread_cond_t* c, pthread_mutex_t* m) { return 0; }
int pthread_cond_signal(pthread_cond_t* c) { return 0; }
int pthread_create(pthread_t*, const pthread_attr_t*, void*(*)(void*), void*) { return 0; }
int pthread_join(pthread_t, void**) { return 0; }
int pthread_mutex_trylock(pthread_mutex_t*) { return 0; }
void* malloc(size_t sz) { return ::malloc(sz); }
void free(void* p) { ::free(p); }
void sleep(int) {}
void* memset(void* s, int c, size_t n) { return std::memset(s, c, n); }
void* memcpy(void* d, const void* s, size_t n) { return std::memcpy(d, s, n); }
char* strdup(const char* s) { return ::strdup(s); }

void addT2EventMarker(const char*, const char*, const char*, int) {}
bool registerProfileWithScheduler(const char*, int, int, bool, bool, bool, int) { return true; }
bool unregisterProfileFromScheduler(const char*) { return true; }
void T2ER_StartDispatchThread() {}
void removeProfileFromDisk(const char*, const char*) {}
void SendInterruptToTimeoutThread(const char*) {}
void encodeStaticParamsInJSON(void*, void*) {}
void* getProfileParameterValues(void*, int) { return nullptr; }
void encodeParamResultInJSON(void*, void*, void*) {}
void freeProfileValues(void* v) { free(v); }
void* processTopPattern(const char*, void*, void*, int) { return nullptr; }
void encodeGrepResultInJSON(void*, void*) {}
void* getGrepResults(void**, void*, void**, bool, bool, char*) { return nullptr; }
void encodeEventMarkersInJSON(void*, void*) {}
T2ERROR prepareJSONReport(void*, char**) { if (char** p = reinterpret_cast<char**>(char**)(char*)p) { *p = strdup("{}"); } return T2ERROR_SUCCESS; }
void destroyJSONReport(void*) {}
void* saveSeekConfigtoFile(const char*, void*) { return nullptr; }
void* populateCachedReportList(const char*, void*) { return nullptr; }
T2ERROR processConfiguration(char**, const char*, void*, Profile**) { return T2ERROR_SUCCESS; }
void* fetchLocalConfigs(const char*, void*) { return nullptr; }
void* clearPersistenceFolder(const char*) { return nullptr; }
void* saveCachedReportToPersistenceFolder(const char*, void*) { return nullptr; }
void* loadSavedSeekConfig(const char*, void*) { return nullptr; }
void* rbusT2ConsumerReg(void*) { return nullptr; }
void* rbusT2ConsumerUnReg(void*) { return nullptr; }
void* freeGrepSeekProfile(void*) { return nullptr; }
void* freeEMarker(void*) { return nullptr; }
void* freeGMarker(void*) { return nullptr; }
void* freeParam(void*) { return nullptr; }
void* freeStaticParam(void*) { return nullptr; }
void* freeTriggerCondition(void*) { return nullptr; }
void* freeGResult(void*) { return nullptr; }
void* sendReportOverHTTP(const char*, const char*, void*) { return nullptr; }
void* sendCachedReportsOverHTTP(const char*, void*) { return nullptr; }
void* sendReportsOverRBUSMethod(const char*, void*, const char*) { return nullptr; }
void* sendCachedReportsOverRBUSMethod(const char*, void*, void*) { return nullptr; }
bool rbusCheckMethodExists(const char*) { return true; }
int cJSON_GetArraySize(const void*) { return 1; }
void* cJSON_CreateObject() { return malloc(1); }
void cJSON_Delete(void* o) { free(o); }
void* cJSON_CreateArray() { return malloc(1); }
void cJSON_AddItemToObject(void*, const char*, void*) {}
void* cJSON_Parse(const char*) { return malloc(1); }
void* cJSON_GetObjectItem(void*, const char*) { return malloc(1); }
void cJSON_AddStringToObject(void*, const char*, const char*) {}
void* t2_queue_create() { return malloc(1); }
void t2_queue_destroy(void* q, void(*free_fn)(void*)) { free(q); }
void t2_queue_push(void* q, void* d) {}
void* t2_queue_pop(void* q) { return nullptr; }
int t2_queue_count(void* q) { return 0; }
void T2totalmem_calculate() {}
*/

} // extern "C"

// Test fixture
class ProfileTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/*
// Test freeReportProfileConfig
TEST_F(ProfileTest, FreeReportProfileConfig_Null) {
    freeReportProfileConfig(nullptr); // Should not crash
}

TEST_F(ProfileTest, FreeReportProfileConfig_Valid) {
    Config* cfg = (Config*)malloc(sizeof(Config));
    cfg->name = strdup("cfgname");
    cfg->configData = strdup("cfgdata");
    freeReportProfileConfig(cfg); // Should free without crash
}

// Test freeRequestURIparam
TEST_F(ProfileTest, FreeRequestURIparam_Null) {
    freeRequestURIparam(nullptr); // Should not crash
}

TEST_F(ProfileTest, FreeRequestURIparam_Valid) {
    HTTPReqParam* param = (HTTPReqParam*)malloc(sizeof(HTTPReqParam));
    param->HttpName = strdup("name");
    param->HttpRef = strdup("ref");
    param->HttpValue = strdup("value");
    freeRequestURIparam(param); // Should free all
}

// Test freeProfile
TEST_F(ProfileTest, FreeProfile_Null) {
    freeProfile(nullptr); // Should not crash
}

TEST_F(ProfileTest, FreeProfile_Valid) {
    Profile* p = (Profile*)malloc(sizeof(Profile));
    memset(p, 0, sizeof(Profile));
    p->name = strdup("p");
    p->hash = strdup("h");
    p->protocol = strdup("proto");
    p->encodingType = strdup("enc");
    p->RootName = strdup("root");
    p->Description = strdup("desc");
    p->version = strdup("ver");
    p->jsonEncoding = strdup("json");
    p->timeRef = strdup("time");
    p->t2HTTPDest = (T2HTTPDest*)malloc(sizeof(T2HTTPDest));
    memset(p->t2HTTPDest, 0, sizeof(T2HTTPDest));
    p->t2HTTPDest->URL = strdup("url");
    freeProfile(p); // Should free all
}
*/

// Test profileWithNameExists
TEST_F(ProfileTest, ProfileWithNameExists_NotInitialized) {
    bool exists = false;
    //extern bool initialized;
    //initialized = false;
    EXPECT_EQ(profileWithNameExists("test", &exists), T2ERROR_FAILURE);
}

TEST_F(ProfileTest, ProfileWithNameExists_NullName) {
    bool exists = false;
    //extern bool initialized;
    //initialized = true;
    EXPECT_EQ(profileWithNameExists(nullptr, &exists), T2ERROR_FAILURE);
    EXPECT_FALSE(exists);
}

// Test addProfile
TEST_F(ProfileTest, AddProfile_NotInitialized) {
    //extern bool initialized;
    //initialized = false;
    Profile dummy;
    EXPECT_EQ(addProfile(&dummy), T2ERROR_FAILURE);
}

// Test enableProfile
TEST_F(ProfileTest, EnableProfile_NotInitialized) {
    //extern bool initialized;
    //initialized = false;
    EXPECT_EQ(enableProfile("abc"), T2ERROR_FAILURE);
}

// Test disableProfile
TEST_F(ProfileTest, DisableProfile_NotInitialized) {
    //extern bool initialized;
    //initialized = false;
    bool isDeleteRequired = false;
    EXPECT_EQ(disableProfile("abc", &isDeleteRequired), T2ERROR_FAILURE);
}

// Test deleteProfile
TEST_F(ProfileTest, DeleteProfile_NotInitialized) {
    //extern bool initialized;
    //initialized = false;
    EXPECT_EQ(deleteProfile("abc"), T2ERROR_FAILURE);
}

// Test deleteAllProfiles
TEST_F(ProfileTest, DeleteAllProfiles_ProfileListNull) {
    //extern Vector* profileList;
    //profileList = nullptr;
    EXPECT_EQ(deleteAllProfiles(true), T2ERROR_FAILURE);
}

// Test initProfileList
TEST_F(ProfileTest, InitProfileList_Success) {
    //extern bool initialized;
    //initialized = false;
    EXPECT_EQ(initProfileList(false), T2ERROR_SUCCESS);
}

// Test uninitProfileList
TEST_F(ProfileTest, UninitProfileList_Success) {
    //extern bool initialized;
    //initialized = true;
    EXPECT_EQ(uninitProfileList(), T2ERROR_SUCCESS);
}

// Test getProfileCount
TEST_F(ProfileTest, GetProfileCount_NotInitialized) {
    //extern bool initialized;
    //initialized = false;
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
    // Simulate lock already held
    extern int pthread_mutex_trylock(pthread_mutex_t*);
    struct Override {
        static int locked(pthread_mutex_t*) { return 1; }
    };
    auto orig = pthread_mutex_trylock;
    //pthread_mutex_trylock = Override::locked;
    EXPECT_EQ(appendTriggerCondition(&tmp, "refname", "refval"), T2ERROR_FAILURE);
    //pthread_mutex_trylock = orig;
}


