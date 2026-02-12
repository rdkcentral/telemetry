
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "msgpack.h"

#include "telemetry2_0.h"

class MsgpackMock {
public:
//    MOCK_METHOD(void, msgpack_unpacked_init, (msgpack_unpacked*), ());
    MOCK_METHOD(msgpack_unpack_return, msgpack_unpack_next, (msgpack_unpacked*, const char*, size_t, size_t*), ());
  //  MOCK_METHOD(void, msgpack_unpacked_destroy, (msgpack_unpacked*), ());
    MOCK_METHOD(msgpack_object*, msgpack_get_map_value, (const msgpack_object*, const char*), ());
    MOCK_METHOD(int, msgpack_strcmp, (const msgpack_object*, const char*), ());
    MOCK_METHOD(char*, msgpack_strdup, (const msgpack_object*), ());
    MOCK_METHOD(msgpack_object*, msgpack_get_array_element, (const msgpack_object*, int), ());
    MOCK_METHOD(int, msgpack_mock_array_size, (const msgpack_object*), ());
};

// Global pointer for the mock instance
extern MsgpackMock* g_msgpackMock;
