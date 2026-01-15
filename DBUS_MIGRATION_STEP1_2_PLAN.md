# DBUS Migration Plan: Step 1 & 2 - CommonLib Event Sending

## Current Status

**COMPLETED:**
✅ Updated dbusInterface.h with new service name "telemetry.t2"
✅ Added new APIs: `dbusGetMarkerList()`, `dbusGetOperationalStatus()`, `dbusSubscribeProfileUpdate()`
✅ Updated dbusInterface.c with implementations
✅ Includes in telemetry_busmessage_sender.c already changed to use DBUS

**PENDING:**
❌ Remove all RBUS/CCSP specific code from telemetry_busmessage_sender.c
❌ Replace RBUS operations with DBUS equivalents
❌ Update Makefile.am to link with DBUS libraries

---

## Changes Required in telemetry_busmessage_sender.c

### 1. Remove These Functions (CCSP/RBUS specific):

```c
// Lines 138-178: Remove entire CCSP block
#if defined(CCSP_SUPPORT_ENABLED)
T2ERROR getParamValues(...) {...}
static void freeParamValue(...) {...}
static T2ERROR getCCSPParamVal(...) {...}
#endif

// Lines 183-184: Remove RBUS uninit
static void rBusInterface_Uninit() {...}

// Lines 186-235: Remove initMessageBus - Replace with DBUS init
static T2ERROR initMessageBus() {...}

// Lines 237-268: Remove getRbusParameterVal - not needed for commonlib
static T2ERROR getRbusParameterVal(...) {...}
```

### 2. Replace filtered_event_send() Function (Lines 407-500)

**Current RBUS Implementation:**
```c
int filtered_event_send(const char* data, const char *markerName) {
    // Uses rbus_set() with rbusProperty, rbusValue
    // Complex object creation for RBUS
    ...
}
```

**New DBUS Implementation:**
```c
int filtered_event_send(const char* data, const char *markerName) {
    T2ERROR ret = T2ERROR_SUCCESS;
    
    // Check DBUS initialized
    if(!isDbusInitialized()) {
        EVENT_ERROR("DBUS not initialized\n");
        return -1;
    }
    
    // Keep event filtering logic (lines 420-445) AS-IS
    // ... marker list filtering code ...
    
    // Replace RBUS rbus_set() with DBUS signal
    ret = dbusPublishEvent(markerName, data);
    if(ret != T2ERROR_SUCCESS) {
        EVENT_ERROR("dbusPublishEvent Failed\n");
        return -1;
    }
    return 0;
}
```

### 3. Replace doPopulateEventMarkerList() Function (Lines 505-598)

**Current RBUS Implementation:**
```c
static T2ERROR doPopulateEventMarkerList(void) {
    // Uses rbus_get() to fetch marker list as RBUS_OBJECT
    // Iterates through rbusProperty list
    ...
}
```

**New DBUS Implementation:**
```c
static T2ERROR doPopulateEventMarkerList(void) {
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    T2ERROR status = T2ERROR_SUCCESS;
    
    if(!isDbusInitialized()) {
        EVENT_ERROR("DBUS not initialized\n");
        return T2ERROR_FAILURE;
    }
    
    pthread_mutex_lock(&markerListMutex);
    
    if(eventMarkerMap) {
        hash_map_destroy(eventMarkerMap);
        eventMarkerMap = NULL;
    }
    
    // Get marker list via DBUS method call
    char* markerListStr = NULL;
    status = dbusGetMarkerList(componentName, &markerListStr);
    
    if(status != T2ERROR_SUCCESS || !markerListStr) {
        EVENT_ERROR("dbusGetMarkerList failed\n");
        pthread_mutex_unlock(&markerListMutex);
        return T2ERROR_FAILURE;
    }
    
    // Parse marker list string (format: "marker1,marker2,marker3")
    eventMarkerMap = hash_map_create();
    char* token = strtok(markerListStr, ",");
    while(token != NULL) {
        // Trim whitespace
        while(*token == ' ') token++;
        if(strlen(token) > 0) {
            EVENT_DEBUG("\t %s\n", token);
            hash_map_put(eventMarkerMap, strdup(token), strdup(token), free);
        }
        token = strtok(NULL, ",");
    }
    
    free(markerListStr);
    pthread_mutex_unlock(&markerListMutex);
    
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return status;
}
```

### 4. Replace rbusEventReceiveHandler() with DBUS Callback (Lines 600-615)

**Current RBUS Implementation:**
```c
static void rbusEventReceiveHandler(rbusHandle_t handle, rbusEvent_t const* event, ...) {
    // RBUS event subscription callback
    if(strcmp(eventName, T2_PROFILE_UPDATED_NOTIFY) == 0) {
        doPopulateEventMarkerList();
    }
}
```

**New DBUS Implementation:**
```c
static void dbusProfileUpdateHandler(void) {
    EVENT_DEBUG("Profile update notification received\n");
    doPopulateEventMarkerList();
}
```

### 5. Replace isCachingRequired() Function (Lines 617-690)

**Current RBUS Implementation:**
```c
static bool isCachingRequired() {
    // Uses rbus_getUint() to check T2_OPERATIONAL_STATUS
    retVal = rbus_getUint(bus_handle, T2_OPERATIONAL_STATUS, &t2ReadyStatus);
    
    // Uses rbusEvent_Subscribe() for profile updates
    ret = rbusEvent_Subscribe(bus_handle, T2_PROFILE_UPDATED_NOTIFY, rbusEventReceiveHandler, ...);
    ...
}
```

**New DBUS Implementation:**
```c
static bool isCachingRequired() {
    // Keep RFC and PAM initialization checks AS-IS (lines 624-633)
    
    if(!initRFC()) {
        EVENT_ERROR("initRFC failed - cache the events\n");
        return true;
    }
    
    if(!isRFCT2Enable) {
        return false;
    }
    
    // Replace rbus_getUint() with DBUS method call
    uint32_t t2ReadyStatus;
    T2ERROR retVal = dbusGetOperationalStatus(&t2ReadyStatus);
    
    if(retVal != T2ERROR_SUCCESS) {
        return true;
    }
    
    EVENT_DEBUG("Operational status: %d\n", t2ReadyStatus);
    if((t2ReadyStatus & T2_STATE_COMPONENT_READY) == 0) {
        return true;
    }
    
    if(!isT2Ready) {
        if(componentName && (strcmp(componentName, "telemetry_client") != 0)) {
            if((t2ReadyStatus & T2_STATE_COMPONENT_READY) == 0) {
                return true;
            } else {
                // Fetch marker list and subscribe to profile updates
                doPopulateEventMarkerList();
                
                // Subscribe to profile update signal
                T2ERROR ret = dbusSubscribeProfileUpdate(dbusProfileUpdateHandler);
                if(ret != T2ERROR_SUCCESS) {
                    EVENT_ERROR("Failed to subscribe to profile updates\n");
                }
                isT2Ready = true;
            }
        } else {
            isT2Ready = true;
        }
    }
    
    return false;
}
```

### 6. Update t2_init() and t2_uninit() Functions (Lines 734-752)

**Current Implementation:**
```c
void t2_init(char *component) {
    componentName = strdup(component);
}

void t2_uninit(void) {
    if(componentName) {
        free(componentName);
        componentName = NULL;
    }
    if(isRbusEnabled) {
        rBusInterface_Uninit();
    }
    uninitMutex();
}
```

**New DBUS Implementation:**
```c
void t2_init(char *component) {
    componentName = strdup(component);
    
    // Initialize DBUS connection
    T2ERROR ret = dBusInterface_Init(componentName);
    if(ret != T2ERROR_SUCCESS) {
        EVENT_ERROR("DBUS initialization failed for %s\n", componentName);
    }
}

void t2_uninit(void) {
    if(componentName) {
        free(componentName);
        componentName = NULL;
    }
    
    // Uninitialize DBUS
    dBusInterface_Uninit();
    
    uninitMutex();
}
```

---

## Changes Required in Makefile.am

### File: telemetry/source/commonlib/Makefile.am

**Add DBUS dependencies:**

```makefile
# Add DBUS CFLAGS and LIBS
AM_CFLAGS = -I$(top_srcdir)/include \
            -I$(top_srcdir)/source/ccspinterface \
            $(DBUS_CFLAGS) \
            ... (existing flags)

libt2commonlib_la_LIBADD = $(DBUS_LIBS) ... (existing libs)
```

---

## Testing Plan

After making these changes, you should:

1. **Compile:**
   ```bash
   cd telemetry
   ./configure --enable-dbus
   make clean
   make
   ```

2. **Check for DBUS symbols:**
   ```bash
   nm -D .libs/libt2commonlib.so | grep dbus
   ```

3. **Test with simple client:**
   ```bash
   # Terminal 1: Monitor DBUS
   dbus-monitor --session "interface='telemetry.t2.interface'"
   
   # Terminal 2: Run test client
   ./test_client
   ```

4. **Expected DBUS traffic:**
   - Signal: `TelemetryEvent` with (marker_name, event_value)
   - Method call: `GetMarkerList` with component_name
   - Method call: `GetOperationalStatus`
   - Signal: `ProfileUpdate`

---

## Summary of API Mappings

| RBUS API | DBUS API | Type | Purpose |
|----------|----------|------|---------|
| `rbus_set(T2_EVENT_PARAM, ...)` | `dbusPublishEvent(marker, value)` | Signal | Send telemetry event |
| `rbusEvent_Subscribe(T2_PROFILE_UPDATED_NOTIFY)` | `dbusSubscribeProfileUpdate(callback)` | Signal | Listen for profile updates |
| `rbus_get("...EventMarkerList", ...)` | `dbusGetMarkerList(component, &list)` | Method | Get marker list |
| `rbus_getUint(T2_OPERATIONAL_STATUS)` | `dbusGetOperationalStatus(&status)` | Method | Check T2 daemon status |

---

## Questions?

1. Should I proceed with creating the complete modified telemetry_busmessage_sender.c file?
2. Do you want me to create a patch file instead for easier review?
3. Should we test incrementally or make all changes at once?

Please confirm and I'll proceed with the implementation.
