/*
 * D-Bus Test Server for Telemetry T2
 * 
 * Compilation command:
 * gcc -o dbus_test_server dbus_test_server.c -ldbus-1 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -lpthread
 * 
 * Alternative for different architectures:
 * gcc -o dbus_test_server dbus_test_server.c $(pkg-config --cflags --libs dbus-1) -lpthread
 * 
 * Usage:
 * ./dbus_test_server
 * 
 * To send ProfileUpdate signal, type 's' and press Enter in the running server
 * Logs are written to: dbus_test_server.log
 * 
 * Copyright 2026
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <dbus/dbus.h>

/* D-Bus Configuration - Must match client */
#define T2_DBUS_SERVICE_NAME        "telemetry.t2"
#define T2_DBUS_OBJECT_PATH         "/telemetry/t2"
#define T2_DBUS_INTERFACE_NAME      "telemetry.t2.interface"
#define T2_DBUS_EVENT_INTERFACE_NAME "telemetry.t2.event.interface"

/* Server Configuration */
#define LOG_FILE                    "dbus_test_server.log"
#define T2_OPERATIONAL_STATUS       "Telemetry.OperationalStatus"
#define T2_STATE_COMPONENT_READY    0x01

/* Marker Lists for Each Client */
static const char* CLIENT_ONE_MARKERS[] = {
    "c1Test1_split",
    "c1Test2_bootup",
    "c1Test3_network",
    "c1Test4_wifi",
    "c1Test5_system"
};

static const char* CLIENT_TWO_MARKERS[] = {
    "c2Test1_memory",
    "c2Test2_cpu",
    "c2Test3_disk",
    "c2Test4_process"
};

static const char* CLIENT_THREE_MARKERS[] = {
    "c3Test1_error",
    "c3Test2_warning",
    "c3Test3_info",
    "c3Test4_debug",
    "c3Test5_trace",
    "c3Test6_critical"
};

#define CLIENT_ONE_MARKER_COUNT     (sizeof(CLIENT_ONE_MARKERS) / sizeof(CLIENT_ONE_MARKERS[0]))
#define CLIENT_TWO_MARKER_COUNT     (sizeof(CLIENT_TWO_MARKERS) / sizeof(CLIENT_TWO_MARKERS[0]))
#define CLIENT_THREE_MARKER_COUNT   (sizeof(CLIENT_THREE_MARKERS) / sizeof(CLIENT_THREE_MARKERS[0]))

/* Global Variables */
static DBusConnection *g_connection = NULL;
static bool g_server_ready = false;
static bool g_running = true;
static FILE *g_log_file = NULL;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Logging Function */
static void server_log(const char* level, const char* format, ...) {
    pthread_mutex_lock(&g_log_mutex);
    
    if (!g_log_file) {
        g_log_file = fopen(LOG_FILE, "a");
    }
    
    if (g_log_file) {
        time_t now;
        struct tm timeinfo;
        char timebuf[64];
        
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &timeinfo);
        
        fprintf(g_log_file, "[%s] [%s] ", timebuf, level);
        
        va_list args;
        va_start(args, format);
        vfprintf(g_log_file, format, args);
        va_end(args);
        
        fprintf(g_log_file, "\n");
        fflush(g_log_file);
    }
    
    pthread_mutex_unlock(&g_log_mutex);
}

#define LOG_DEBUG(fmt, ...)  server_log("DEBUG", fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)   server_log("INFO", fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)  server_log("ERROR", fmt, ##__VA_ARGS__)

/* Get Marker List for Component */
static char* get_marker_list_for_component(const char* component_name) {
    LOG_DEBUG("get_marker_list_for_component: component=%s", component_name);
    
    const char** markers = NULL;
    int count = 0;
    
    if (strcmp(component_name, "client_one") == 0) {
        markers = CLIENT_ONE_MARKERS;
        count = CLIENT_ONE_MARKER_COUNT;
        LOG_DEBUG("Selected client_one with %d markers", count);
    } else if (strcmp(component_name, "client_two") == 0) {
        markers = CLIENT_TWO_MARKERS;
        count = CLIENT_TWO_MARKER_COUNT;
        LOG_DEBUG("Selected client_two with %d markers", count);
    } else if (strcmp(component_name, "client_three") == 0) {
        markers = CLIENT_THREE_MARKERS;
        count = CLIENT_THREE_MARKER_COUNT;
        LOG_DEBUG("Selected client_three with %d markers", count);
    } else {
        LOG_INFO("Unknown component: %s, returning empty list", component_name);
        return strdup("");
    }
    
    /* Calculate required buffer size */
    size_t total_len = 0;
    for (int i = 0; i < count; i++) {
        total_len += strlen(markers[i]);
        if (i < count - 1) {
            total_len += 1; /* For comma */
        }
    }
    total_len += 1; /* For null terminator */
    
    /* Build comma-separated list */
    char* result = (char*)malloc(total_len);
    if (!result) {
        LOG_ERROR("Failed to allocate memory for marker list");
        return strdup("");
    }
    
    result[0] = '\0';
    for (int i = 0; i < count; i++) {
        strcat(result, markers[i]);
        if (i < count - 1) {
            strcat(result, ",");
        }
    }
    
    LOG_DEBUG("Returning marker list: %s", result);
    return result;
}

/* Handle GetOperationalStatus Method */
static DBusHandlerResult handle_get_operational_status(DBusConnection *connection, DBusMessage *message) {
    LOG_DEBUG("handle_get_operational_status: Received GetOperationalStatus method call");
    
    DBusError error;
    dbus_error_init(&error);
    
    const char* param_name = NULL;
    if (!dbus_message_get_args(message, &error,
                               DBUS_TYPE_STRING, &param_name,
                               DBUS_TYPE_INVALID)) {
        LOG_ERROR("Failed to parse GetOperationalStatus arguments: %s", error.message);
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    
    LOG_INFO("GetOperationalStatus called with param_name: %s", param_name);
    
    uint32_t value = 0;
    
    /* Check if requesting operational status */
    if (strcmp(param_name, T2_OPERATIONAL_STATUS) == 0) {
        if (g_server_ready) {
            value = T2_STATE_COMPONENT_READY;
            LOG_INFO("Returning operational status: READY (0x%08X)", value);
        } else {
            value = 0;
            LOG_INFO("Returning operational status: NOT READY (0x%08X)", value);
        }
    } else {
        LOG_INFO("Unknown parameter: %s, returning 0", param_name);
        value = 0;
    }
    
    /* Create reply */
    DBusMessage *reply = dbus_message_new_method_return(message);
    if (!reply) {
        LOG_ERROR("Failed to create reply message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    
    if (!dbus_message_append_args(reply,
                                  DBUS_TYPE_UINT32, &value,
                                  DBUS_TYPE_INVALID)) {
        LOG_ERROR("Failed to append reply arguments");
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    
    if (!dbus_connection_send(connection, reply, NULL)) {
        LOG_ERROR("Failed to send reply");
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    
    LOG_DEBUG("GetOperationalStatus: Reply sent successfully");
    dbus_message_unref(reply);
    dbus_connection_flush(connection);
    
    return DBUS_HANDLER_RESULT_HANDLED;
}

/* Handle SendT2Event Method */
static DBusHandlerResult handle_send_t2_event(DBusConnection *connection, DBusMessage *message) {
    LOG_DEBUG("handle_send_t2_event: Received SendT2Event method call");
    
    DBusError error;
    dbus_error_init(&error);
    
    const char* marker_name = NULL;
    const char* data = NULL;
    
    if (!dbus_message_get_args(message, &error,
                               DBUS_TYPE_STRING, &marker_name,
                               DBUS_TYPE_STRING, &data,
                               DBUS_TYPE_INVALID)) {
        LOG_ERROR("Failed to parse SendT2Event arguments: %s", error.message);
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    
    LOG_INFO("========================================");
    LOG_INFO("SendT2Event - EVENT RECEIVED");
    LOG_INFO("  Marker Name: %s", marker_name);
    LOG_INFO("  Data: %s", data);
    LOG_INFO("========================================");
    
    /* Print to console as well */
    printf("\n[EVENT] Marker: %s, Data: %s\n", marker_name, data);
    fflush(stdout);
    
    /* Create empty reply (method returns void) */
    DBusMessage *reply = dbus_message_new_method_return(message);
    if (!reply) {
        LOG_ERROR("Failed to create reply message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    
    if (!dbus_connection_send(connection, reply, NULL)) {
        LOG_ERROR("Failed to send reply");
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    
    LOG_DEBUG("SendT2Event: Reply sent successfully");
    dbus_message_unref(reply);
    dbus_connection_flush(connection);
    
    return DBUS_HANDLER_RESULT_HANDLED;
}

/* Handle GetMarkerList Method */
static DBusHandlerResult handle_get_marker_list(DBusConnection *connection, DBusMessage *message) {
    LOG_DEBUG("handle_get_marker_list: Received GetMarkerList method call");
    
    DBusError error;
    dbus_error_init(&error);
    
    const char* component_name = NULL;
    if (!dbus_message_get_args(message, &error,
                               DBUS_TYPE_STRING, &component_name,
                               DBUS_TYPE_INVALID)) {
        LOG_ERROR("Failed to parse GetMarkerList arguments: %s", error.message);
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    
    LOG_INFO("GetMarkerList called for component: %s", component_name);
    
    /* Get marker list for this component */
    char* marker_list = get_marker_list_for_component(component_name);
    
    /* Create reply */
    DBusMessage *reply = dbus_message_new_method_return(message);
    if (!reply) {
        LOG_ERROR("Failed to create reply message");
        free(marker_list);
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    
    if (!dbus_message_append_args(reply,
                                  DBUS_TYPE_STRING, &marker_list,
                                  DBUS_TYPE_INVALID)) {
        LOG_ERROR("Failed to append reply arguments");
        dbus_message_unref(reply);
        free(marker_list);
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    
    if (!dbus_connection_send(connection, reply, NULL)) {
        LOG_ERROR("Failed to send reply");
        dbus_message_unref(reply);
        free(marker_list);
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    
    LOG_DEBUG("GetMarkerList: Reply sent successfully");
    dbus_message_unref(reply);
    free(marker_list);
    dbus_connection_flush(connection);
    
    return DBUS_HANDLER_RESULT_HANDLED;
}

/* Message Handler */
static DBusHandlerResult message_handler(DBusConnection *connection, DBusMessage *message, void *user_data) {
    (void)user_data;
    
    const char* interface = dbus_message_get_interface(message);
    const char* member = dbus_message_get_member(message);
    const char* path = dbus_message_get_path(message);
    
    LOG_DEBUG("Received D-Bus message: interface=%s, member=%s, path=%s",
              interface ? interface : "NULL",
              member ? member : "NULL", 
              path ? path : "NULL");
    
    /* Check if message is for our interface */
    if (interface && strcmp(interface, T2_DBUS_INTERFACE_NAME) == 0)
    {
        if (dbus_message_is_method_call(message, T2_DBUS_INTERFACE_NAME, "GetOperationalStatus")) {
            return handle_get_operational_status(connection, message);
        }
        else if (dbus_message_is_method_call(message, T2_DBUS_INTERFACE_NAME, "SendT2Event")) {
            return handle_send_t2_event(connection, message);
        }
        else if (dbus_message_is_method_call(message, T2_DBUS_INTERFACE_NAME, "GetMarkerList")) {
            return handle_get_marker_list(connection, message);
        }
    }
    
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/* Send ProfileUpdate Signal */
static void send_profile_update_signal(void)
{
    LOG_INFO("send_profile_update_signal: Sending ProfileUpdate signal to all clients");
    
    if (!g_connection) {
        LOG_ERROR("Connection is NULL, cannot send signal");
        return;
    }
    
    DBusMessage *signal = dbus_message_new_signal(T2_DBUS_OBJECT_PATH,
                                                   T2_DBUS_EVENT_INTERFACE_NAME,
                                                   "ProfileUpdate");
    if (!signal) {
        LOG_ERROR("Failed to create ProfileUpdate signal");
        return;
    }
    
    /* Send signal - this queues the message */
    dbus_uint32_t serial = 0;
    if (!dbus_connection_send(g_connection, signal, &serial)) {
        LOG_ERROR("Failed to send ProfileUpdate signal - out of memory");
        dbus_message_unref(signal);
        return;
    }
    
    dbus_message_unref(signal);
    
    /* NOTE: We don't call dbus_connection_flush() here because:
     * 1. Signals are fire-and-forget (no reply expected)
     * 2. flush() blocks until all messages are sent, causing delays
     * 3. The main event loop handles actual transmission
     * This makes signal sending immediate regardless of subscriber count */
    
    LOG_INFO("ProfileUpdate signal queued successfully (serial=%u)", serial);
    printf("\n[SIGNAL] ProfileUpdate queued (serial=%u)\n", serial);
    fflush(stdout);
}

/* Input Handler Thread */
static void* input_handler_thread(void* arg) {
    (void)arg;
    
    LOG_INFO("Input handler thread started");
    printf("\nD-Bus Test Server Running\n");
    printf("Commands:\n");
    printf("  s - Send ProfileUpdate signal\n");
    printf("  q - Quit server\n");
    printf("\n");
    
    while (g_running) {
        char input[256];
        if (fgets(input, sizeof(input), stdin)) {
            if (input[0] == 's' || input[0] == 'S') {
                send_profile_update_signal();
            } else if (input[0] == 'q' || input[0] == 'Q') {
                LOG_INFO("Quit command received");
                g_running = false;
                break;
            }
        }
    }
    
    LOG_INFO("Input handler thread exiting");
    return NULL;
}

/* Signal Handler */
static void signal_handler(int signum) {
    LOG_INFO("Received signal %d, shutting down", signum);
    g_running = false;
}

/* Main Function */
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    DBusError error;
    int ret;
    pthread_t input_thread;
    
    /* Initialize logging */
    g_log_file = fopen(LOG_FILE, "w+");
    if (!g_log_file) {
        fprintf(stderr, "Failed to open log file: %s\n", LOG_FILE);
        return 1;
    }
    
    LOG_INFO("========================================");
    LOG_INFO("D-Bus Test Server Starting");
    LOG_INFO("Service: %s", T2_DBUS_SERVICE_NAME);
    LOG_INFO("Object: %s", T2_DBUS_OBJECT_PATH);
    LOG_INFO("Interface: %s", T2_DBUS_INTERFACE_NAME);
    LOG_INFO("========================================");
    
    /* Setup signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* Initialize D-Bus error */
    dbus_error_init(&error);
    
    /* Initialize D-Bus threading */
    if (!dbus_threads_init_default()) {
        LOG_ERROR("Failed to initialize D-Bus threading");
        return 1;
    }
    LOG_DEBUG("D-Bus threading initialized");
    
    /* Connect to system bus */
    g_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error)) {
        LOG_ERROR("Failed to connect to D-Bus: %s", error.message);
        dbus_error_free(&error);
        return 1;
    }
    LOG_INFO("Connected to D-Bus system bus");
    
    /* Request service name */
    ret = dbus_bus_request_name(g_connection, T2_DBUS_SERVICE_NAME,
                                DBUS_NAME_FLAG_REPLACE_EXISTING,
                                &error);
    if (dbus_error_is_set(&error)) {
        LOG_ERROR("Failed to request name: %s", error.message);
        dbus_error_free(&error);
        dbus_connection_unref(g_connection);
        return 1;
    }
    
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        LOG_ERROR("Not primary owner of the name (ret=%d)", ret);
        dbus_connection_unref(g_connection);
        return 1;
    }
    LOG_INFO("Acquired service name: %s", T2_DBUS_SERVICE_NAME);
    
    /* Register object path */
    DBusObjectPathVTable vtable = {
        .message_function = message_handler,
        .unregister_function = NULL
    };
    
    if (!dbus_connection_register_object_path(g_connection, T2_DBUS_OBJECT_PATH,
                                              &vtable, NULL)) {
        LOG_ERROR("Failed to register object path");
        dbus_connection_unref(g_connection);
        return 1;
    }
    LOG_INFO("Registered object path: %s", T2_DBUS_OBJECT_PATH);
    
    /* Server is now ready */
    g_server_ready = true;
    LOG_INFO("Server is READY to accept method calls");
    
    /* Start input handler thread */
    if (pthread_create(&input_thread, NULL, input_handler_thread, NULL) != 0) {
        LOG_ERROR("Failed to create input thread");
        g_running = false;
    } else {
        LOG_DEBUG("Input handler thread created");
    }
    
    /* Main event loop - REQUIRED to receive and process method calls from clients */
    LOG_INFO("Entering main event loop");
    while (g_running) {
        /* 
         * This call is CRITICAL - it does three things:
         * 1. Reads incoming D-Bus messages (method calls from clients)
         * 2. Writes outgoing D-Bus messages (replies, signals)
         * 3. Dispatches received messages to our message_handler
         * 
         * Without this, the server would NOT receive any method calls!
         * Timeout of 100ms allows responsive shutdown on g_running = false
         */
        dbus_connection_read_write_dispatch(g_connection, 100);
    }
    
    LOG_INFO("Exiting main event loop");
    
    /* Cleanup */
    if (input_thread) {
        pthread_join(input_thread, NULL);
        LOG_DEBUG("Input thread joined");
    }
    
    if (g_connection) {
        dbus_connection_unref(g_connection);
        LOG_DEBUG("D-Bus connection released");
    }
    
    if (g_log_file) {
        LOG_INFO("D-Bus Test Server Shutdown Complete");
        LOG_INFO("========================================");
        fclose(g_log_file);
    }
    
    printf("\nServer shutdown complete\n");
    return 0;
}
