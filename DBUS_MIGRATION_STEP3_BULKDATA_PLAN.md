# DBUS Migration Plan: STEP 3 - BulkData Side (T2 Daemon)

## Overview

This step implements the **server side** of the DBUS communication in the T2 daemon (BulkData). The daemon will:
1. **Receive telemetry events** via DBUS signals from commonlib clients
2. **Provide marker lists** via DBUS method responses
3. **Provide operational status** via DBUS method responses  
4. **Send profile update notifications** via DBUS signals to commonlib clients

---

## Current Status

**STEP 1 & 2 COMPLETED:**
✅ CommonLib modified to use DBUS (telemetry_busmessage_sender.c)
✅ DBUS interface APIs implemented (dbusInterface.h/c)
✅ Makefile.am updated with DBUS dependencies

**STEP 3 PENDING:**
❌ Implement DBUS signal receiver for telemetry events in BulkData
❌ Implement DBUS method providers (GetMarkerList, GetOperationalStatus)
❌ Implement DBUS signal sender for profile updates
❌ Update BulkData initialization to register DBUS handlers

---

## Architecture Overview

```
┌──────────────────┐                        ┌───────────────────────┐
│  CommonLib       │                        │  BulkData (T2 Daemon) │
│  (Client Apps)   │                        │                       │
├──────────────────┤                        ├───────────────────────┤
│                  │  DBUS Signal           │                       │
│  dbusPublishEvent├───────────────────────>│ dbusEventHandler()    │
│  (marker, value) │   "TelemetryEvent"     │   ↓                   │
│                  │                        │ T2ER_Push()           │
│                  │                        │   ↓                   │
│                  │                        │ Event Queue           │
└──────────────────┘                        └───────────────────────┘
         │                                            │
         │  DBUS Method Call                          │
         │  "GetMarkerList"                           │
         └───────────────────────────────────────────>│
         │                                            │ dbusGetMarkerListHandler()
         │<───────────────────────────────────────────┤ getComponentMarkerList()
         │  DBUS Method Reply                         │
         │  (marker1,marker2,...)                     │
         │                                            │
         │  DBUS Method Call                          │
         │  "GetOperationalStatus"                    │
         └───────────────────────────────────────────>│
         │                                            │ dbusGetOperationalStatusHandler()
         │<───────────────────────────────────────────┤ t2ReadyStatus
         │  DBUS Method Reply (uint32)                │
         │                                            │
         │                                            │
         │  DBUS Signal                               │
         │  "ProfileUpdate"                           │
         │<───────────────────────────────────────────┤ publishEventsProfileUpdates()
         │                                            │ (when profiles change)
```

---

## Files to Modify

### Primary Files:
1. **`telemetry/source/bulkdata/t2eventreceiver.c`** - Add DBUS event reception
2. **`telemetry/source/bulkdata/t2markers.c`** - Add DBUS method provider for marker list
3. **`telemetry/source/ccspinterface/rbusInterface.c`** - Add DBUS method providers and signal sender
4. **`telemetry/source/telemetry2_0.c`** - Update daemon initialization

### Supporting Files:
5. **`telemetry/source/ccspinterface/Makefile.am`** - Already has DBUS (verify)
6. **`telemetry/source/bulkdata/Makefile.am`** - May need DBUS includes

---

## Detailed Implementation Plan

### **STEP 3.1: Implement DBUS Event Reception in BulkData**

**File:** [`t2eventreceiver.c`](telemetry/source/bulkdata/t2eventreceiver.c)

**Current RBUS Implementation:**
- Function: `registerRbusT2EventListener()` registers callback for RBUS events
- Callback: `t2rbusEventCallback()` receives RBUS property set events
- Processing: Extracts event name/value and calls `T2ER_Push()`

**New DBUS Implementation:**

```c
/**
 * @brief DBUS signal handler for telemetry events
 * Called when TelemetryEvent signal is received
 */
static DBusHandlerResult dbusEventSignalHandler(DBusConnection *connection, 
                                                 DBusMessage *message, 
                                                 void *user_data)
{
    (void)connection;
    (void)user_data;
    
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!dbus_message_is_signal(message, T2_DBUS_INTERFACE_NAME, T2_DBUS_SIGNAL_EVENT)) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    
    const char* eventName = NULL;
    const char* eventValue = NULL;
    DBusError error;
    dbus_error_init(&error);
    
    // Extract event name and value from signal
    if (!dbus_message_get_args(message, &error,
                               DBUS_TYPE_STRING, &eventName,
                               DBUS_TYPE_STRING, &eventValue,
                               DBUS_TYPE_INVALID)) {
        T2Error("Failed to parse TelemetryEvent signal: %s\n", error.message);
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    T2Info("Received DBUS event: %s = %s\n", eventName, eventValue);
    
    // Create rbusProperty for backward compatibility with existing code
    rbusProperty_t properties = NULL;
    rbusValue_t value;
    rbusValue_Init(&value);
    rbusValue_SetString(value, eventValue);
    rbusProperty_Init(&properties, eventName, value);
    
    // Push to event queue (reuse existing T2ER_Push logic)
    T2ER_Push(properties);
    
    // Cleanup
    rbusValue_Release(value);
    rbusProperty_Release(properties);
    
    T2Debug("%s --out\n", __FUNCTION__);
    return DBUS_HANDLER_RESULT_HANDLED;
}

/**
 * @brief Register DBUS telemetry event listener
 */
T2ERROR registerDbusT2EventListener(void)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!isDbusInitialized()) {
        T2Error("DBUS not initialized\n");
        return T2ERROR_FAILURE;
    }
    
    // Subscribe to TelemetryEvent signal
    T2ERROR ret = registerDbusT2EventListener(dbusEventSignalHandler);
    if (ret != T2ERROR_SUCCESS) {
        T2Error("Failed to register DBUS event listener\n");
        return T2ERROR_FAILURE;
    }
    
    T2Info("DBUS telemetry event listener registered\n");
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}
```

**Changes Required:**
- Add `dbusEventSignalHandler()` function
- Add `registerDbusT2EventListener()` function  
- Update `T2ER_Init()` to call `registerDbusT2EventListener()` instead of/alongside RBUS

---

### **STEP 3.2: Implement DBUS Method Provider for Marker List**

**File:** [`rbusInterface.c`](telemetry/source/ccspinterface/rbusInterface.c)

**Current RBUS Implementation:**
- Data element: `"Telemetry.ReportProfiles.{component}.EventMarkerList"`
- Handler: `t2PropertyDataGetHandler()` calls `getMarkerListCallBack()`
- Returns: RBUS_OBJECT containing marker names as properties

**New DBUS Implementation:**

```c
/**
 * @brief DBUS method handler for GetMarkerList
 */
static DBusMessage* dbusGetMarkerListHandler(DBusMessage *msg)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    
    const char* componentName = NULL;
    DBusError error;
    dbus_error_init(&error);
    
    // Extract component name from method call
    if (!dbus_message_get_args(msg, &error,
                               DBUS_TYPE_STRING, &componentName,
                               DBUS_TYPE_INVALID)) {
        T2Error("Failed to parse GetMarkerList arguments: %s\n", error.message);
        dbus_error_free(&error);
        return dbus_message_new_error(msg, DBUS_ERROR_INVALID_ARGS, "Missing component name");
    }
    
    T2Debug("GetMarkerList request for component: %s\n", componentName);
    
    // Get marker list from callback (reuse existing getComponentMarkerList)
    Vector* markerList = NULL;
    if (getMarkerListCallBack) {
        getMarkerListCallBack(componentName, (void**)&markerList);
    }
    
    // Build comma-separated string
    char markerListStr[4096] = {0};
    if (markerList && Vector_Size(markerList) > 0) {
        for (size_t i = 0; i < Vector_Size(markerList); i++) {
            char* marker = (char*)Vector_At(markerList, i);
            if (marker) {
                if (i > 0) strcat(markerListStr, ",");
                strcat(markerListStr, marker);
            }
        }
    }
    
    T2Debug("Marker list: %s\n", markerListStr);
    
    // Create reply
    DBusMessage* reply = dbus_message_new_method_return(msg);
    const char* markerListPtr = markerListStr;
    dbus_message_append_args(reply,
                             DBUS_TYPE_STRING, &markerListPtr,
                             DBUS_TYPE_INVALID);
    
    T2Debug("%s --out\n", __FUNCTION__);
    return reply;
}
```

---

### **STEP 3.3: Implement DBUS Method Provider for Operational Status**

**File:** [`rbusInterface.c`](telemetry/source/ccspinterface/rbusInterface.c)

**Current RBUS Implementation:**
- Data element: `"Telemetry.OperationalStatus"`
- Handler: `t2PropertyDataGetHandler()` returns `t2ReadyStatus` (uint32)

**New DBUS Implementation:**

```c
/**
 * @brief DBUS method handler for GetOperationalStatus
 */
static DBusMessage* dbusGetOperationalStatusHandler(DBusMessage *msg)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    
    // Get current operational status (global variable)
    uint32_t status = t2ReadyStatus;
    
    T2Debug("Operational status: %u\n", status);
    
    // Create reply
    DBusMessage* reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
                             DBUS_TYPE_UINT32, &status,
                             DBUS_TYPE_INVALID);
    
    T2Debug("%s --out\n", __FUNCTION__);
    return reply;
}
```

---

### **STEP 3.4: Implement DBUS Signal Sender for Profile Updates**

**File:** [`rbusInterface.c`](telemetry/source/ccspinterface/rbusInterface.c)

**Current RBUS Implementation:**
- Function: `publishEventsProfileUpdates()`
- Uses: `rbusEvent_Publish()` on `"Telemetry.ReportProfiles.ProfilesUpdated"`

**New DBUS Implementation:**

```c
/**
 * @brief Publish profile update notification via DBUS signal
 */
void dbusPublishProfileUpdate(void)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!isDbusInitialized()) {
        T2Error("DBUS not initialized\n");
        return;
    }
    
    DBusMessage *signal = dbus_message_new_signal(T2_DBUS_OBJECT_PATH,
                                                   T2_DBUS_INTERFACE_NAME,
                                                   T2_DBUS_SIGNAL_PROFILE_UPDATE);
    if (!signal) {
        T2Error("Failed to create ProfileUpdate signal\n");
        return;
    }
    
    // Send signal (no arguments needed)
    if (!dbus_connection_send(getDbuConnection(), signal, NULL)) {
        T2Error("Failed to send ProfileUpdate signal\n");
    } else {
        dbus_connection_flush(getDbusConnection());
        T2Info("ProfileUpdate signal sent\n");
    }
    
    dbus_message_unref(signal);
    T2Debug("%s --out\n", __FUNCTION__);
}
```

**Update existing function:**
```c
void publishEventsProfileUpdates(void)
{
    // Keep RBUS for internal components (if needed)
    // Add DBUS for commonlib clients
    dbusPublishProfileUpdate();
}
```

---

### **STEP 3.5: Register DBUS Method Handlers**

**File:** [`rbusInterface.c`](telemetry/source/ccspinterface/rbusInterface.c) or new `dbusMethodProvider.c`

**Create DBUS method object registration:**

```c
/**
 * @brief DBUS method dispatch function
 */
static DBusHandlerResult dbusMethodDispatcher(DBusConnection *connection,
                                               DBusMessage *message,
                                               void *user_data)
{
    (void)connection;
    (void)user_data;
    
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (dbus_message_is_method_call(message, T2_DBUS_INTERFACE_NAME, "GetMarkerList")) {
        DBusMessage* reply = dbusGetMarkerListHandler(message);
        dbus_connection_send(connection, reply, NULL);
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (dbus_message_is_method_call(message, T2_DBUS_INTERFACE_NAME, "GetOperationalStatus")) {
        DBusMessage* reply = dbusGetOperationalStatusHandler(message);
        dbus_connection_send(connection, reply, NULL);
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    T2Debug("%s --out\n", __FUNCTION__);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/**
 * @brief Register DBUS method providers
 */
T2ERROR registerDbusMethodProviders(void)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!isDbusInitialized()) {
        T2Error("DBUS not initialized\n");
        return T2ERROR_FAILURE;
    }
    
    // Register object path with method handler
    DBusObjectPathVTable vtable = {
        .message_function = dbusMethodDispatcher,
        .unregister_function = NULL
    };
    
    if (!dbus_connection_register_object_path(getDbusConnection(),
                                               T2_DBUS_OBJECT_PATH,
                                               &vtable,
                                               NULL)) {
        T2Error("Failed to register DBUS object path\n");
        return T2ERROR_FAILURE;
    }
    
    T2Info("DBUS method providers registered\n");
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}
```

---

### **STEP 3.6: Update T2 Daemon Initialization**

**File:** [`telemetry2_0.c`](telemetry/source/telemetry2_0.c)

**Current Initialization:**
```c
int main(int argc, char *argv[])
{
    ...
    // Initialize RBUS
    initRbusInterface();
    ...
    // Initialize event receiver
    T2ER_Init();
    ...
}
```

**Updated Initialization:**
```c
int main(int argc, char *argv[])
{
    ...
    // Initialize DBUS (alongside or instead of RBUS)
    T2ERROR ret = dBusInterface_Init("telemetry2_0");
    if (ret != T2ERROR_SUCCESS) {
        T2Error("DBUS initialization failed\n");
        // Handle error
    }
    
    // Register DBUS method providers
    registerDbusMethodProviders();
    
    // Initialize event receiver (will register DBUS event listener)
    T2ER_Init();
    ...
}
```

---

## Summary of Changes

| Component | Current RBUS | New DBUS | File |
|-----------|-------------|----------|------|
| **Event Reception** | `t2rbusEventCallback()` | `dbusEventSignalHandler()` | t2eventreceiver.c |
| **Marker List Provider** | `t2PropertyDataGetHandler()` | `dbusGetMarkerListHandler()` | rbusInterface.c |
| **Status Provider** | `t2PropertyDataGetHandler()` | `dbusGetOperationalStatusHandler()` | rbusInterface.c |
| **Profile Update Notification** | `rbusEvent_Publish()` | `dbusPublishProfileUpdate()` | rbusInterface.c |
| **Method Registration** | `rbus_regDataElements()` | `registerDbusMethodProviders()` | rbusInterface.c |
| **Daemon Init** | `initRbusInterface()` | `dBusInterface_Init()` | telemetry2_0.c |

---

## API Flow Summary

### 1. **Telemetry Event Flow** (CommonLib → BulkData)
```
Client App
  ↓ t2_event_s("marker", "value")
CommonLib  
  ↓ dbusPublishEvent()
  ↓ DBUS Signal "TelemetryEvent"
BulkData
  ↓ dbusEventSignalHandler()
  ↓ T2ER_Push()
  ↓ Event Queue Processing
```

### 2. **Marker List Request Flow** (CommonLib → BulkData)
```
Client App
  ↓ isCachingRequired() / doPopulateEventMarkerList()
CommonLib
  ↓ dbusGetMarkerList("component")
  ↓ DBUS Method Call "GetMarkerList"
BulkData
  ↓ dbusGetMarkerListHandler()
  ↓ getComponentMarkerList()
  ↓ DBUS Method Reply("marker1,marker2,...")
CommonLib
  ↓ Parse and store in eventMarkerMap
```

### 3. **Operational Status Check Flow** (CommonLib → BulkData)
```
Client App
  ↓ isCachingRequired()
CommonLib
  ↓ dbusGetOperationalStatus()
  ↓ DBUS Method Call "GetOperationalStatus"
BulkData
  ↓ dbusGetOperationalStatusHandler()
  ↓ Return t2ReadyStatus
  ↓ DBUS Method Reply(uint32)
CommonLib
  ↓ Check status and decide caching
```

### 4. **Profile Update Notification Flow** (BulkData → CommonLib)
```
BulkData
  ↓ Profile configuration changed
  ↓ publishEventsProfileUpdates()
  ↓ dbusPublishProfileUpdate()
  ↓ DBUS Signal "ProfileUpdate"
CommonLib
  ↓ dbusProfileUpdateHandler()
  ↓ doPopulateEventMarkerList()
  ↓ Refresh marker list from daemon
```

---

## Testing Strategy

### Unit Tests:
1. **Event Reception:** Send DBUS signal, verify T2ER_Push() called
2. **Marker List:** Call GetMarkerList method, verify correct response
3. **Status Check:** Call GetOperationalStatus, verify status value
4. **Profile Update:** Emit ProfileUpdate signal, verify clients notified

### Integration Tests:
1. **End-to-End Event:** Client sends event → Daemon receives → Stored in queue
2. **Marker List Sync:** Client requests list → Daemon provides → Client filters events
3. **Status Check:** Client checks status → Decides caching → Sends when ready
4. **Profile Update:** Daemon updates profile → Sends signal → Client refreshes list

### System Tests:
1. **Full Profile Cycle:** Load profile → Send events → Update profile → Events filtered
2. **Multi-Client:** Multiple clients sending events simultaneously
3. **Stress Test:** 10,000 events from 10 clients concurrently
4. **Failover:** Daemon restart → Clients reconnect → Events cached/restored

---

## Compilation and Testing Commands

```bash
# Compile
cd telemetry
./configure
make clean
make

# Check DBUS symbols in daemon
nm -D .libs/telemetry2_0 | grep dbus

# Run daemon with DBUS monitoring
# Terminal 1: Monitor DBUS
dbus-monitor --session "interface='telemetry.t2.interface'"

# Terminal 2: Run T2 daemon
./telemetry2_0

# Terminal 3: Run test client
./telemetry2_0_client

# Expected DBUS traffic:
# - Signal: TelemetryEvent(marker, value)
# - Method call: GetMarkerList(component) → Reply(markers)
# - Method call: GetOperationalStatus() → Reply(status)
# - Signal: ProfileUpdate()
```

---

## Questions for Review

1. **Backward Compatibility:** Do we need to keep RBUS alongside DBUS for internal components?
2. **Error Handling:** Should DBUS failures fall back to RBUS or cache events?
3. **Threading:** Should DBUS method handlers run in separate thread or main loop?
4. **Data Format:** Is comma-separated marker list format acceptable, or prefer JSON?
5. **Registration:** Should we register DBUS object path in dbusInterface.c or rbusInterface.c?

---

## Next Steps After Review

Once you review and approve this plan:
1. I'll implement all BulkData changes
2. Update necessary build files (Makefile.am)
3. You test the end-to-end flow
4. We move to Step 4: Complete testing and deployment

**Ready for your review and feedback!**
