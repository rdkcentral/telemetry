/*
 * D-Bus Test Client for Telemetry T2
 * Simplified version with only D-Bus support, no CCSP/RBUS, no mutexes
 * 
 * Compilation command:
 * gcc -o dbus_test_client dbus_test_client.c -ldbus-1 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -lpthread -I../../include
 * 
 * Alternative:
 * gcc -o dbus_test_client dbus_test_client.c $(pkg-config --cflags --libs dbus-1) -lpthread -I../../include
 * 
 * Usage:
 * ./dbus_test_client
 * 
 * Copyright 2026
 */
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <dbus/dbus.h>

/* Component Name */
#define COMP_NAME "client_one"

/* D-Bus Configuration */
#define T2_DBUS_SERVICE_NAME        "telemetry.t2"
#define T2_DBUS_OBJECT_PATH         "/telemetry/t2"
#define T2_DBUS_INTERFACE_NAME      "telemetry.t2.interface"
#define T2_DBUS_EVENT_INTERFACE_NAME "telemetry.t2.event.interface"

/* Log file */
#define CLIENT_LOG_FILE "/tmp/t2_client_debug.log"

/* T2 Configuration */
#define T2_OPERATIONAL_STATUS       "Telemetry.OperationalStatus"
#define T2_STATE_COMPONENT_READY    0x01
#define MAX_DATA_LEN                512

/* Error codes */
typedef enum {
    T2ERROR_SUCCESS = 0,
    T2ERROR_FAILURE,
    T2ERROR_INVALID_ARGS
} T2ERROR;

/* Hash Map - Simple implementation for marker filtering */
typedef struct hash_node {
    char *key;
    char *value;
    struct hash_node *next;
} hash_node_t;

typedef struct {
    hash_node_t **buckets;
    size_t size;
} hash_map_t;

/* Global Variables */
static char *componentName = NULL;
static void *bus_handle = NULL;
static bool isRFCT2Enable = false;
static bool isT2Ready = false;
static bool isDbusEnabled = false;
static bool isInitialized = false;  // Track if initialization is complete
static hash_map_t *eventMarkerMap = NULL;
static pthread_t dbus_event_thread;
static bool dbus_event_thread_running = false;

/* Logging */
#define EVENT_DEBUG(format, ...) do { \
    struct timespec ts; \
    struct tm timeinfo; \
    char timeBuffer[32]; \
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) { \
        localtime_r(&ts.tv_sec, &timeinfo); \
        long msecs = ts.tv_nsec / 1000000; \
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeinfo); \
        snprintf(timeBuffer + strlen(timeBuffer), sizeof(timeBuffer) - strlen(timeBuffer), ".%03ld", msecs); \
        fprintf(stderr, "[%s] T2DEBUG:%s %s:%d: ", timeBuffer, __func__ , __FILE__, __LINE__ ); \
    } else { \
        fprintf(stderr, "T2DEBUG:%s %s:%d: ", __func__ , __FILE__, __LINE__ ); \
    } \
    fprintf(stderr, (format), ##__VA_ARGS__ ); \
} while(0)

#define EVENT_ERROR(format, ...) do { \
    fprintf(stderr, "T2ERROR:%s %s:%d: ", __func__ , __FILE__, __LINE__ ); \
    fprintf(stderr, (format), ##__VA_ARGS__ ); \
} while(0)

/* ============== Hash Map Implementation ============== */

static hash_map_t* hash_map_create(void) {
    hash_map_t *map = (hash_map_t*)malloc(sizeof(hash_map_t));
    if (!map) return NULL;
    map->size = 256;
    map->buckets = (hash_node_t**)calloc(map->size, sizeof(hash_node_t*));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    return map;
}

static unsigned int hash_function(const char *key, size_t size) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash * 31) + (*key++);
    }
    return hash % size;
}

static void hash_map_put(hash_map_t *map, char *key, char *value, void (*free_func)(void*)) {
    (void)free_func;
    if (!map || !key) return;
    
    unsigned int index = hash_function(key, map->size);
    hash_node_t *node = (hash_node_t*)malloc(sizeof(hash_node_t));
    if (!node) return;
    
    node->key = key;
    node->value = value;
    node->next = map->buckets[index];
    map->buckets[index] = node;
}

static char* hash_map_get(hash_map_t *map, const char *key) {
    if (!map || !key) return NULL;
    
    unsigned int index = hash_function(key, map->size);
    hash_node_t *node = map->buckets[index];
    
    while (node) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

static void hash_map_destroy(hash_map_t *map, void (*free_func)(void*)) {
    if (!map) return;
    
    for (size_t i = 0; i < map->size; i++) {
        hash_node_t *node = map->buckets[i];
        while (node) {
            hash_node_t *temp = node;
            node = node->next;
            if (free_func) {
                free_func(temp->key);
                free_func(temp->value);
            }
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}

/* ============== D-Bus Functions ============== */

static int dbus_checkStatus(void)
{
    DBusError error;
    dbus_error_init(&error);
    DBusConnection *test_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    
    if (dbus_error_is_set(&error))
    {
        dbus_error_free(&error);
        return -1;
    }
    
    if (test_conn)
    {
        dbus_connection_unref(test_conn);
        return 0;
    }
    
    return -1;
}

static T2ERROR dbus_getGetOperationalStatus(const char* paramName, uint32_t* value)
{
    if (!paramName || !value)
    {
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!bus_handle)
    {
        return T2ERROR_FAILURE;
    }
    
    DBusMessage *msg = NULL;
    DBusMessage *reply = NULL;
    DBusError error;
    dbus_error_init(&error);
    
    msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                       T2_DBUS_OBJECT_PATH,
                                       T2_DBUS_INTERFACE_NAME,
                                       "GetOperationalStatus");
    if (!msg)
    {
        EVENT_ERROR("D-Bus failed to create method call message\n");
        return T2ERROR_FAILURE;
    }
    
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &paramName, DBUS_TYPE_INVALID))
    {
        dbus_message_unref(msg);
        return T2ERROR_FAILURE;
    }
    
    reply = dbus_connection_send_with_reply_and_block((DBusConnection*)bus_handle, msg, 1000, &error);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&error))
    {
        EVENT_ERROR("D-Bus error: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    if (!reply)
    {
        return T2ERROR_FAILURE;
    }

    if (dbus_message_get_args(reply, &error, DBUS_TYPE_UINT32, value, DBUS_TYPE_INVALID))
    {
        dbus_message_unref(reply);
        EVENT_DEBUG("D-Bus got uint32 value: %u\n", *value);
        return T2ERROR_SUCCESS;
    }
    else
    {
        if (dbus_error_is_set(&error))
        {
            EVENT_ERROR("D-Bus error: %s\n", error.message);
            dbus_error_free(&error);
        }
        dbus_message_unref(reply);
        return T2ERROR_FAILURE;
    }
}

static void dBusInterface_Uninit(void)
{
    if(isDbusEnabled && bus_handle)
    {
        if (dbus_event_thread_running)
        {
            EVENT_DEBUG("D-Bus: Stopping event loop thread\n");
            dbus_event_thread_running = false;
            pthread_join(dbus_event_thread, NULL);
        }
        
        dbus_connection_unref((DBusConnection*)bus_handle);
        bus_handle = NULL;
        isDbusEnabled = false;
    }
}

static T2ERROR initMessageBus(void)
{
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    T2ERROR status = T2ERROR_SUCCESS;
    
    if(0 == dbus_checkStatus())
    {
        char dbusName[124] = { '\0' };
        if(componentName)
        {
            snprintf(dbusName, 124, "telemetry.t2.lib_%s", componentName);
        }
        else
        {
            snprintf(dbusName, 124, "telemetry.t2.lib_default");
        }
        
        DBusError error;
        dbus_error_init(&error);
        bus_handle = (void*)dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        
        if (dbus_error_is_set(&error))
        {
            EVENT_ERROR("D-Bus init failed: %s\n", error.message);
            dbus_error_free(&error);
            status = T2ERROR_FAILURE;
        }
        else if (bus_handle)
        {
            int ret = dbus_bus_request_name((DBusConnection*)bus_handle, dbusName,
                                           DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
            if (dbus_error_is_set(&error))
            {
                EVENT_ERROR("D-Bus name request failed: %s\n", error.message);
                dbus_error_free(&error);
                dbus_connection_unref((DBusConnection*)bus_handle);
                bus_handle = NULL;
                status = T2ERROR_FAILURE;
            }
            else if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER && ret != DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER)
            {
                EVENT_ERROR("D-Bus name request returned: %d\n", ret);
                status = T2ERROR_FAILURE;
            }
            else
            {
                isDbusEnabled = true;
                status = T2ERROR_SUCCESS;
                EVENT_DEBUG("D-Bus initialized successfully as: %s\n", dbusName);
            }
        }
        else
        {
            status = T2ERROR_FAILURE;
        }
    }
    else
    {
        EVENT_ERROR("D-Bus not available\n");
        status = T2ERROR_FAILURE;
    }
    
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return status;
}

static bool initRFC(void)
{
    bool status = true;
    if(!bus_handle)
    {
        if(initMessageBus() != T2ERROR_SUCCESS)
        {
            EVENT_ERROR("initMessageBus failed\n");
            status = false;
        }
        else
        {
            EVENT_DEBUG("initMessageBus successful\n");
            status = true;
        }
        isRFCT2Enable = true;
    }
    return status;
}

/* ============== Event Marker List Management ============== */

static T2ERROR doPopulateEventMarkerList(void)
{
    EVENT_DEBUG("%s ++in (D-Bus mode)\n", __FUNCTION__);
    
    if(!bus_handle && T2ERROR_SUCCESS != initMessageBus())
    {
        EVENT_ERROR("Unable to get message bus handles\n");
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    EVENT_DEBUG("Clean up eventMarkerMap\n");
    if(eventMarkerMap != NULL)
    {
        hash_map_destroy(eventMarkerMap, free);
        eventMarkerMap = NULL;
    }

    // Get marker list via D-Bus method call
    DBusMessage *msg = NULL;
    DBusMessage *reply = NULL;
    DBusError error;
    dbus_error_init(&error);
    
    msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                       T2_DBUS_OBJECT_PATH,
                                       T2_DBUS_INTERFACE_NAME,
                                       "GetMarkerList");
    if (!msg)
    {
        EVENT_ERROR("D-Bus mode: Failed to create method call message\n");
        return T2ERROR_FAILURE;
    }
    
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &componentName, DBUS_TYPE_INVALID))
    {
        EVENT_ERROR("D-Bus mode: Failed to append arguments\n");
        dbus_message_unref(msg);
        return T2ERROR_FAILURE;
    }
    
    reply = dbus_connection_send_with_reply_and_block((DBusConnection*)bus_handle, msg, 1000, &error);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&error))
    {
        EVENT_ERROR("D-Bus mode: Method call failed: %s\n", error.message);
        dbus_error_free(&error);
        EVENT_DEBUG("D-Bus mode: No event list configured\n");
        return T2ERROR_SUCCESS;
    }

    if (!reply)
    {
        EVENT_ERROR("D-Bus mode: No reply received\n");
        return T2ERROR_SUCCESS;
    }

    // Parse reply - expecting a string containing comma-separated marker list
    char *markerListStr = NULL;
    if (dbus_message_get_args(reply, &error, 
                             DBUS_TYPE_STRING, &markerListStr,
                             DBUS_TYPE_INVALID))
    {
        EVENT_DEBUG("D-Bus mode: Received marker list: %s\n", markerListStr ? markerListStr : "NULL");
        
        if (markerListStr && strlen(markerListStr) > 0)
        {
            eventMarkerMap = hash_map_create();
            EVENT_DEBUG("Update event map for component %s with markers:\n", componentName);
            
            // Parse comma-separated marker list
            char *markerListCopy = strdup(markerListStr);
            char *token = strtok(markerListCopy, ",");
            while (token != NULL)
            {
                // Trim whitespace
                while (*token == ' ') token++;
                char *end = token + strlen(token) - 1;
                while (end > token && *end == ' ') end--;
                *(end + 1) = '\0';
                
                if (strlen(token) > 0)
                {
                    EVENT_DEBUG("  %s\n", token);
                    hash_map_put(eventMarkerMap, strdup(token), strdup(token), free);
                }
                token = strtok(NULL, ",");
            }
            free(markerListCopy);
        }
        else
        {
            EVENT_DEBUG("D-Bus mode: Empty marker list for %s\n", componentName);
        }
        
        dbus_message_unref(reply);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_SUCCESS;
    }
    else
    {
        if (dbus_error_is_set(&error))
        {
            EVENT_ERROR("D-Bus mode: Failed to parse reply: %s\n", error.message);
            dbus_error_free(&error);
        }
        dbus_message_unref(reply);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
}

/* ============== ProfileUpdate Signal Handler ============== */

static DBusHandlerResult dbusEventReceiveHandler(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    (void)connection;
    (void)user_data;
    
    if (dbus_message_is_signal(message, T2_DBUS_EVENT_INTERFACE_NAME, "ProfileUpdate"))
    {
        EVENT_DEBUG("D-Bus: *** ProfileUpdate signal RECEIVED - updating marker list ***\n");
        doPopulateEventMarkerList();
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

// D-Bus event loop thread function
static void* dbus_event_loop_thread(void *arg)
{
    DBusConnection *connection = (DBusConnection*)arg;
    
    EVENT_DEBUG("D-Bus: Event loop thread started\n");
    
    while (dbus_event_thread_running)
    {
        dbus_connection_read_write_dispatch(connection, 100);
    }
    
    EVENT_DEBUG("D-Bus: Event loop thread exiting\n");
    return NULL;
}

/* ============== T2 Ready Check ============== */

static bool isCachingRequired(void)
{
    if(!initRFC())
    {
        EVENT_ERROR("initRFC failed\n");
        return true;
    }

    if(!isRFCT2Enable)
    {
        return true;
    }

    uint32_t t2ReadyStatus;
    T2ERROR retVal = T2ERROR_FAILURE;

    if(isDbusEnabled)
    {
        retVal = dbus_getGetOperationalStatus(T2_OPERATIONAL_STATUS, &t2ReadyStatus);
        EVENT_DEBUG("D-Bus: GetOperationalStatus returned: %d, status: 0x%08X\n", retVal, t2ReadyStatus);
    }

    if(retVal != T2ERROR_SUCCESS)
    {
        EVENT_ERROR("Failed to get operational status\n");
        return true;
    }
    else
    {
        if(t2ReadyStatus & T2_STATE_COMPONENT_READY)
        {
            isT2Ready = true;
            EVENT_DEBUG("T2 is READY - fetching marker list\n");
        }
        else
        {
            EVENT_DEBUG("T2 is NOT ready yet\n");
        }
    }

    // Always get marker list and subscribe to ProfileUpdate (whether T2 is ready or not)
    if(isDbusEnabled)
    {
        EVENT_DEBUG("D-Bus: Fetching marker list and setting up ProfileUpdate subscription\n");
        
        // Get initial marker list
        doPopulateEventMarkerList();
        
        // Subscribe to D-Bus ProfileUpdate signal for dynamic updates
        char rule[512];
        DBusError error;
        dbus_error_init(&error);
        
        snprintf(rule, sizeof(rule), 
                 "type='signal',path='%s',interface='%s',member='ProfileUpdate'",
                 T2_DBUS_OBJECT_PATH, T2_DBUS_EVENT_INTERFACE_NAME);
        
        EVENT_DEBUG("D-Bus: Adding match rule: %s\n", rule);
        EVENT_DEBUG("D-Bus: Event Interface: %s, Path: %s\n", T2_DBUS_EVENT_INTERFACE_NAME, T2_DBUS_OBJECT_PATH);
        
        dbus_bus_add_match((DBusConnection*)bus_handle, rule, &error);
        
        if (dbus_error_is_set(&error))
        {
            EVENT_ERROR("Unable to subscribe to ProfileUpdate signal: %s\n", error.message);
            dbus_error_free(&error);
        }
        else
        {
            EVENT_DEBUG("D-Bus: Match rule added successfully\n");
            dbus_connection_add_filter((DBusConnection*)bus_handle, dbusEventReceiveHandler, NULL, NULL);
            EVENT_DEBUG("D-Bus: Message filter registered - dbusEventReceiveHandler installed\n");
            EVENT_DEBUG("D-Bus: Now listening for ProfileUpdate signals on interface '%s'\n", T2_DBUS_EVENT_INTERFACE_NAME);
            
            // Start D-Bus event loop thread to process incoming messages
            if (!dbus_event_thread_running)
            {
                dbus_event_thread_running = true;
                if (pthread_create(&dbus_event_thread, NULL, dbus_event_loop_thread, bus_handle) != 0)
                {
                    EVENT_ERROR("Failed to create D-Bus event loop thread\n");
                    dbus_event_thread_running = false;
                }
                else
                {
                    EVENT_DEBUG("D-Bus: Event loop thread created successfully\n");
                }
            }
        }
    }
    else
    {
        EVENT_ERROR("D-Bus not enabled\n");
    }

    // Return true to cache if T2 is not ready, false if ready
    return !isT2Ready;
}

/* ============== Event Sending ============== */

static int filtered_event_send(const char* data, const char *markerName)
{
    int status = 0;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    
    if(!bus_handle)
    {
        EVENT_ERROR("bus_handle is null .. exiting !!! \n");
        return -1;
    }

    if(isDbusEnabled)
    {
        // Filter data from marker list
        if(componentName)
        {
            EVENT_DEBUG("Check marker list for component %s\n", componentName);
            bool isEventingEnabled = false;
            if(markerName && eventMarkerMap)
            {
                if(hash_map_get(eventMarkerMap, markerName))
                {
                    isEventingEnabled = true;
                }
            }
            else
            {
                EVENT_DEBUG("eventMarkerMap for component %s is empty\n", componentName);
            }
            
            if(!isEventingEnabled)
            {
                EVENT_DEBUG("markerName %s not found in event list for component %s\n", markerName, componentName);
                return 0;
            }
        }

        // D-Bus method call to send event
        DBusMessage *msg = NULL;
        DBusError error;
        dbus_error_init(&error);
        
        msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                           T2_DBUS_OBJECT_PATH,
                                           T2_DBUS_INTERFACE_NAME,
                                           "SendT2Event");
        if (!msg)
        {
            EVENT_ERROR("Failed to create D-Bus method call message\n");
            status = -1;
        }
        else
        {
            if (!dbus_message_append_args(msg,
                                         DBUS_TYPE_STRING, &markerName,
                                         DBUS_TYPE_STRING, &data,
                                         DBUS_TYPE_INVALID))
            {
                EVENT_ERROR("Failed to append D-Bus method call arguments\n");
                dbus_message_unref(msg);
                status = -1;
            }
            else
            {
                if (!dbus_connection_send((DBusConnection*)bus_handle, msg, NULL))
                {
                    EVENT_ERROR("Failed to send D-Bus method call\n");
                    status = -1;
                }
                else
                {
                    // Don't flush - let the event loop thread handle it
                    // Flushing here blocks until socket is written, causing delays
                    EVENT_DEBUG("Event sent successfully: %s = %s\n", markerName, data);
                    status = 0;
                }
                dbus_message_unref(msg);
            }
        }
    }
    else
    {
        EVENT_ERROR("D-Bus not enabled\n");
        status = -1;
    }
    
    EVENT_DEBUG("%s --out with status %d\n", __FUNCTION__, status);
    return status;
}

static int report_or_cache_data(char* telemetry_data, const char* markerName)
{
    int ret = 0;
    
    // Check if initialization is complete
    if(!isInitialized)
    {
        EVENT_ERROR("T2 not initialized - call t2_init() first\n");
        free(telemetry_data);
        return -1;
    }
    
    // If T2 is not ready, cache (for now just skip)
    if(!isT2Ready)
    {
        EVENT_DEBUG("T2 not ready yet - would cache event\n");
        free(telemetry_data);
        return -1;
    }

    // T2 is ready - send event
    EVENT_DEBUG("T2 is ready, sending event\n");
    ret = filtered_event_send(telemetry_data, markerName);
    free(telemetry_data);
    
    return ret;
}

/* ============== Public API ============== */

void t2_init(char *component)
{
    EVENT_DEBUG("t2_init called with component: %s\n", component);
    componentName = strdup(component);
    
    // Initialize D-Bus connection
    if(!bus_handle)
    {
        if(initMessageBus() != T2ERROR_SUCCESS)
        {
            EVENT_ERROR("initMessageBus failed\n");
            return;
        }
        isRFCT2Enable = true;
    }
    
    // Check if T2 is ready
    uint32_t t2ReadyStatus = 0;
    if(isDbusEnabled)
    {
        T2ERROR retVal = dbus_getGetOperationalStatus(T2_OPERATIONAL_STATUS, &t2ReadyStatus);
        EVENT_DEBUG("D-Bus: GetOperationalStatus returned: %d, status: 0x%08X\n", retVal, t2ReadyStatus);
        
        if(retVal == T2ERROR_SUCCESS && (t2ReadyStatus & T2_STATE_COMPONENT_READY))
        {
            isT2Ready = true;
            EVENT_DEBUG("T2 is READY\n");
        }
        else
        {
            EVENT_DEBUG("T2 is NOT ready yet\n");
        }
    }
    
    // Get marker list from server (only if T2 is ready)
    if(isT2Ready && isDbusEnabled)
    {
        EVENT_DEBUG("Fetching marker list for component: %s\n", componentName);
        doPopulateEventMarkerList();
    }
    
    // Subscribe to ProfileUpdate signals for dynamic updates
    if(isDbusEnabled)
    {
        char rule[512];
        DBusError error;
        dbus_error_init(&error);
        
        snprintf(rule, sizeof(rule), 
                 "type='signal',path='%s',interface='%s',member='ProfileUpdate'",
                 T2_DBUS_OBJECT_PATH, T2_DBUS_EVENT_INTERFACE_NAME);
        
        EVENT_DEBUG("D-Bus: Subscribing to ProfileUpdate signals\n");
        EVENT_DEBUG("D-Bus: Match rule: %s\n", rule);
        
        dbus_bus_add_match((DBusConnection*)bus_handle, rule, &error);
        
        if (dbus_error_is_set(&error))
        {
            EVENT_ERROR("Unable to subscribe to ProfileUpdate signal: %s\n", error.message);
            dbus_error_free(&error);
        }
        else
        {
            EVENT_DEBUG("D-Bus: ProfileUpdate subscription successful\n");
            dbus_connection_add_filter((DBusConnection*)bus_handle, dbusEventReceiveHandler, NULL, NULL);
            EVENT_DEBUG("D-Bus: Event handler registered\n");
            
            // Start D-Bus event loop thread
            if (!dbus_event_thread_running)
            {
                dbus_event_thread_running = true;
                if (pthread_create(&dbus_event_thread, NULL, dbus_event_loop_thread, bus_handle) != 0)
                {
                    EVENT_ERROR("Failed to create D-Bus event loop thread\n");
                    dbus_event_thread_running = false;
                }
                else
                {
                    EVENT_DEBUG("D-Bus: Event loop thread started\n");
                }
            }
        }
    }
    
    // Mark initialization as complete
    isInitialized = true;
    EVENT_DEBUG("t2_init completed successfully - isT2Ready: %d\n", isT2Ready);
}

void t2_uninit(void)
{
    EVENT_DEBUG("t2_uninit called\n");
    
    if(componentName)
    {
        free(componentName);
        componentName = NULL;
    }

    if(isDbusEnabled)
    {
        dBusInterface_Uninit();
    }
    
    if(eventMarkerMap)
    {
        hash_map_destroy(eventMarkerMap, free);
        eventMarkerMap = NULL;
    }
    
    // Reset all flags
    isInitialized = false;
    isT2Ready = false;
    isRFCT2Enable = false;
}

T2ERROR t2_event_s(const char* marker, const char* value)
{
    int ret;
    T2ERROR retStatus = T2ERROR_FAILURE;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    char* strvalue = NULL;
    
    if(componentName == NULL)
    {
        EVENT_ERROR("Component name is NULL\n");
        return T2ERROR_FAILURE;
    }
    
    if (NULL == marker || NULL == value)
    {
        EVENT_ERROR("marker or value is NULL\n");
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_INVALID_ARGS;
    }

    EVENT_DEBUG("marker = %s : value = %s\n", marker, value);
    
    if (0 == strlen(value) || strcmp(value, "0") == 0)
    {
        EVENT_DEBUG("Empty or zero value, skipping\n");
        return T2ERROR_SUCCESS;
    }
    
    strvalue = strdup(value);
    if(strvalue[strlen(strvalue) - 1] == '\n')
    {
        strvalue[strlen(strvalue) - 1] = '\0';
    }
    
    ret = report_or_cache_data(strvalue, marker);
    
    if(ret != -1)
    {
        retStatus = T2ERROR_SUCCESS;
    }
    
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return retStatus;
}

T2ERROR t2_event_f(const char* marker, double value)
{
    int ret;
    T2ERROR retStatus = T2ERROR_FAILURE;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);

    if(componentName == NULL)
    {
        EVENT_ERROR("Component name is NULL\n");
        return T2ERROR_FAILURE;
    }

    if (NULL == marker)
    {
        EVENT_ERROR("marker is NULL\n");
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_INVALID_ARGS;
    }

    EVENT_DEBUG("marker = %s : value = %f\n", marker, value);
    
    char *buffer = (char*)malloc(MAX_DATA_LEN * sizeof(char));
    if (NULL != buffer)
    {
        snprintf(buffer, MAX_DATA_LEN, "%f", value);
        ret = report_or_cache_data(buffer, marker);
        if(ret != -1)
        {
            retStatus = T2ERROR_SUCCESS;
        }
    }
    else
    {
        EVENT_ERROR("Failed to allocate buffer\n");
    }
    
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return retStatus;
}

T2ERROR t2_event_d(const char* marker, int value)
{
    int ret;
    T2ERROR retStatus = T2ERROR_FAILURE;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);

    if(componentName == NULL)
    {
        EVENT_ERROR("Component name is NULL\n");
        return T2ERROR_FAILURE;
    }

    if (NULL == marker)
    {
        EVENT_ERROR("marker is NULL\n");
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_INVALID_ARGS;
    }

    EVENT_DEBUG("marker = %s : value = %d\n", marker, value);

    if (value == 0)
    {
        EVENT_DEBUG("Value is 0, skipping per requirement\n");
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_SUCCESS;
    }

    char *buffer = (char*)malloc(MAX_DATA_LEN * sizeof(char));
    if (NULL != buffer)
    {
        snprintf(buffer, MAX_DATA_LEN, "%d", value);
        ret = report_or_cache_data(buffer, marker);
        if(ret != -1)
        {
            retStatus = T2ERROR_SUCCESS;
        }
    }
    else
    {
        EVENT_ERROR("Failed to allocate buffer\n");
    }
    
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return retStatus;
}

/* ============== Main Test Function ============== */

int main(int argc, char *argv[])
{
    (void)argc; // To fix compiler warning
    (void)argv; // To fix compiler warning
    
    printf("===========================================\n");
    printf("D-Bus Test Client Starting\n");
    printf("Component: %s\n", COMP_NAME);
    printf("===========================================\n");
    
    t2_init(COMP_NAME);
    
    printf("\nSending test events...\n");
    
    printf("1. Sending: c1Test1_split = test_value_1\n");
    t2_event_s("c1Test1_split", "test_value_1");
    
    printf("2. Sending: c1Test2_bootup = test_value_2\n");
    t2_event_s("c1Test2_bootup", "test_value_2");
    
    printf("3. Sending: c1Test2_bootup = test_value_3\n");
    t2_event_s("c1Test2_bootup", "test_value_3");
    
    printf("\nTest events sent. Waiting for ProfileUpdate signals...\n");
    printf("Press Ctrl+C to exit\n");
    
    // Keep running to receive ProfileUpdate signals
    while(1)
    {
        sleep(5);
    }
    
    t2_uninit();
    
    printf("\n===========================================\n");
    printf("D-Bus Test Client Shutdown\n");
    printf("===========================================\n");
    
    return 0;
}
