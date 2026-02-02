#include <pthread.h>
#include "test/bulkdata/t2eventreceiverMock.h"


int g_mock_push_call_count = 0;

MockEventQueue* gEventQueueMock = nullptr;
MockProfileMarker* gProfileMarkerMock = nullptr;

extern "C" {

// C bindings to redirect calls in t2eventreceiver.c to the mocks

int __wrap_t2_queue_count(void *queue) {
    return gEventQueueMock ? gEventQueueMock->count() : 0;
}

void __wrap_t2_queue_push(void *queue, void *data) {
    ++g_mock_push_call_count;	
    if(gEventQueueMock) gEventQueueMock->push(data);
}

void*  __wrap_t2_queue_pop(void *queue) {
    return gEventQueueMock ? gEventQueueMock->pop() : nullptr;
}

void  __wrap_t2_queue_destroy(void *queue, void (*free_func)(void*)) {
    if(gEventQueueMock) gEventQueueMock->destroy(free_func);
}

int __wrap_getMarkerProfileList(const char* marker, void* list) {
    return gProfileMarkerMock ? gProfileMarkerMock->getProfiles(marker, (void**)list) : -1;
}

int __wrap_ReportProfiles_storeMarkerEvent(char* name, void* event) {
    return gProfileMarkerMock ? gProfileMarkerMock->storeMarkerEvent(name, event) : -1;
}

int __wrap_pthread_mutex_lock(pthread_mutex_t *mutex) {
    (void)mutex; // To suppress unused variable warning
    return 0;
}
int __wrap_pthread_mutex_unlock(pthread_mutex_t *mutex) {
    (void)mutex;
    return 0;
}
int __wrap_pthread_cond_signal(pthread_cond_t *cond) {
    (void)cond;
    return 0;
}
int __wrap_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    (void)cond; (void)mutex;
    return 0;
}
int __wrap_pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
    (void)mutex; (void)attr;
    return 0;
}
int __wrap_pthread_mutex_destroy(pthread_mutex_t *mutex) {
    (void)mutex; return 0;
}
int __wrap_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
    (void)cond; (void)attr;
    return 0;
}
int __wrap_pthread_cond_destroy(pthread_cond_t *cond) {
    (void)cond; return 0;
}
} // extern "C"
