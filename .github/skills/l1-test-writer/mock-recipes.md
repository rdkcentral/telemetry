# L1 mock recipes

Reference for `l1-test-writer`. Mocks live in `source/test/mocks/`; module-specific mocks live next to the module test (for example `source/test/scheduler/schedulerMock.cpp`).

## How interception works

Each mock `.cpp` defines `extern "C"` functions with the same name as the real C function. The wrapper forwards to the mock object through a global pointer. Where a `dlsym(RTLD_NEXT, ...)` pointer exists (`SystemMock`, `FileioMock`), a NULL mock pointer falls through to the real function. A mock only affects a test binary that compiles its `.cpp` (check `*_gtest_bin_SOURCES` in the module `Makefile.am`).

Unexpected calls on a plain gmock object return the default value (0, NULL, false) and print an "uninteresting call" warning. Use `EXPECT_CALL(...).Times(::testing::AnyNumber())` or `::testing::NiceMock<T>` where calls are incidental.

## Catalog

| Mock (header) | Pointer | Defined in | Intercepts |
|---|---|---|---|
| `SystemMock.h` | `g_systemMock` | the test file | `system`, `unlink`, `access`, `remove`, `inotify_*`; `clock_gettime` and `select` only when `g_mockClockGettime` / `g_mockSelect` is true |
| `FileioMock.h` (`FileMock`) | `g_fileIOMock` | the test file | `fopen`, `fclose`, `fgets`, `fread`, `fwrite`, `fseek`, `ftell`, `fscanf`, `popen`, `pclose`, `pipe`, `open`, `close`, `read`, `write`, `opendir`, `readdir`, `closedir`, `mkdir`, `fstat`, `getline`, `fork`, `mmap`, `munmap`, `mkstemp`, `sendfile`, `exit`, `getpid`, curl easy and slist calls, `rdkcertselector_*` |
| `rbusMock.h` | `g_rbusMock` | the test file | `rbus_*`, `rbusValue_*`, `rbusObject_*`, `rbusProperty_*`, `rbusEvent_*`, `rbusMethod_*`, `rbusMethodCaller` |
| `VectorMock.h` | `g_vectorMock` | `VectorMock.cpp` (do not redefine) | `Vector_Create`, `Vector_Destroy`, `Vector_Clear`, `Vector_PushBack`, `Vector_RemoveItem`, `Vector_Sort`, `Vector_At`, `Vector_Size`; a built-in fallback runs when the pointer is NULL |
| `rdkconfigMock.h` | `g_rdkconfigMock` | the test file | `rdkconfig_get`, `rdkconfig_set`, `rdkconfig_free` (`RDKCONFIG_OK` / `RDKCONFIG_FAIL`) |
| `rdklogMock.h` | `m_rdklogMock` | the test file | `rdk_logger_init` only; returns 1 when the pointer is NULL |
| `protocolMock.h` | `g_protocolMock` | the test file | `isMtlsEnabled` |
| `profileStub.c` | n/a | compiled in | profile function stubs for parser tests |
| `<module>Mock.cpp/.h` | `m_<module>Mock` | the test file | functions of other modules the module depends on |

Check the header for the exact pointer name before use.

## Recipes

Force a file open failure:

```cpp
EXPECT_CALL(*g_fileIOMock, fopen(::testing::_, ::testing::_)).WillOnce(::testing::Return(nullptr));
EXPECT_EQ(T2ERROR_FAILURE, functionUnderTest("/tmp/does_not_matter"));
```

Force an rbus call to fail:

```cpp
EXPECT_CALL(*g_rbusMock, rbus_open(::testing::_, ::testing::_)).WillOnce(::testing::Return(RBUS_ERROR_BUS_ERROR));
```

Force a `system()` failure and assert it is called once:

```cpp
EXPECT_CALL(*g_systemMock, system(::testing::StrEq("expected command"))).Times(1).WillOnce(::testing::Return(-1));
```

Control time for one test only:

```cpp
g_mockClockGettime = true;
EXPECT_CALL(*g_systemMock, clock_gettime(::testing::_, ::testing::_))
    .WillRepeatedly(::testing::DoAll(::testing::SetArgPointee<1>(ts), ::testing::Return(0)));
/* ... exercise ... */
g_mockClockGettime = false;
```

Make a vector operation fail (allocation and push-back error paths):

```cpp
EXPECT_CALL(*g_vectorMock, Vector_PushBack(::testing::_, ::testing::_)).WillOnce(::testing::Return(T2ERROR_FAILURE));
```

Fill an output argument and return a value:

```cpp
EXPECT_CALL(*g_rbusMock, rbus_get(::testing::_, ::testing::_, ::testing::_))
    .WillOnce(::testing::DoAll(::testing::SetArgPointee<2>(value), ::testing::Return(RBUS_ERROR_SUCCESS)));
```

## Adding a module mock for another module's function

Follow `source/test/scheduler/schedulerMock.h` and `.cpp`:

1. An interface class and a gmock class with `MOCK_METHOD` entries for only the functions the code under test calls.
2. An `extern "C"` function with the real signature that forwards to `m_<module>Mock`, and returns a safe default (for example `T2ERROR_FAILURE`) when the pointer is NULL.
3. Add the `.cpp` to the module's `*_gtest_bin_SOURCES`; define the pointer in the test file.

Reuse an existing mock before adding a new one. Do not mock the function under test.
