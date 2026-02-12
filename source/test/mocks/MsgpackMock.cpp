#include "test/mocks/MsgpackMock.h"

MsgpackMock* g_msgpackMock = nullptr;

#if 0
// Wrapper for msgpack_unpacked_init
extern "C" void msgpack_unpacked_init(msgpack_unpacked *result) {
    if (g_msgpackMock) return g_msgpackMock->msgpack_unpacked_init(result);
    // Real or stub implementation here if not mocked
    if (result) {
        memset(result, 0, sizeof(*result));
    }
}
#endif
// Wrapper for msgpack_unpack_next
extern "C" msgpack_unpack_return msgpack_unpack_next(msgpack_unpacked *result, const char *data, size_t len, size_t *off) {
    if (g_msgpackMock) return g_msgpackMock->msgpack_unpack_next(result, data, len, off);
    // Real or stub implementation if not mocked
    return MSGPACK_UNPACK_SUCCESS;
}
#if 0
// Wrapper for msgpack_unpacked_destroy
extern "C" void msgpack_unpacked_destroy(msgpack_unpacked *result) {
    if (g_msgpackMock) return g_msgpackMock->msgpack_unpacked_destroy(result);
    // Real or stub implementation if needed
}
#endif
// Wrapper for msgpack_get_map_value
extern "C" msgpack_object* msgpack_get_map_value(const msgpack_object* map, const char* key) {
    if (g_msgpackMock) return g_msgpackMock->msgpack_get_map_value(map, key);
    // Real or stub implementation if not mocked
    return nullptr;
}

// Wrapper for msgpack_strcmp
extern "C" int msgpack_strcmp(const msgpack_object* obj, const char* str) {
    if (g_msgpackMock) return g_msgpackMock->msgpack_strcmp(obj, str);
    // Real or stub implementation if not mocked
    return 0;
}

// Wrapper for msgpack_strdup
extern "C" char* msgpack_strdup(const msgpack_object* obj) {
    if (g_msgpackMock) return g_msgpackMock->msgpack_strdup(obj);
    // Real or stub implementation if not mocked
    return nullptr;
}

// Wrapper for msgpack_get_array_element
extern "C" msgpack_object* msgpack_get_array_element(const msgpack_object* array, int idx) {
    if (g_msgpackMock) return g_msgpackMock->msgpack_get_array_element(array, idx);
    // Real or stub implementation if not mocked
    return nullptr;
}

// Macro for MSGPACK_GET_ARRAY_SIZE
extern "C" int msgpack_mock_array_size(const msgpack_object* array) {
    if (g_msgpackMock) return g_msgpackMock->msgpack_mock_array_size(array);
    // Real or stub implementation if not mocked
    return 0;
}
