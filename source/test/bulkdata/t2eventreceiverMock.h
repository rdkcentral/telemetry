#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "telemetry2_0.h"

// Mocks for the queue API
class MockEventQueue {
public:
    MOCK_METHOD(int, count, (), ());
    MOCK_METHOD(void, push, (void *), ());
    MOCK_METHOD(void*, pop, (), ());
    MOCK_METHOD(void, destroy, (void (*free_fn)(void*)), ());
};

class MockProfileMarker {
public:
    MOCK_METHOD(int, getProfiles, (const char*, void**), ());
    MOCK_METHOD(int, storeMarkerEvent, (char*, void*), ());
};

// Declare globals to be set/reset in your test fixture
extern MockEventQueue* gEventQueueMock;
extern MockProfileMarker* gProfileMarkerMock;
