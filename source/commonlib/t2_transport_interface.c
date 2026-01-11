#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <poll.h>
#include <fcntl.h>

#if defined(CCSP_SUPPORT_ENABLED)
#include <ccsp/ccsp_memory.h>
#include <ccsp/ccsp_custom.h>
#include <ccsp/ccsp_base_api.h>
#endif
#include <rbus/rbus.h>

#include "telemetry_busmessage_sender.h"
#include "t2_transport_interface.h"

#include "t2collection.h"
#include "vector.h"
#include "telemetry2_0.h"
#include "t2log_wrapper.h"
#include "telemetry_busmessage_internal.h"


#define MESSAGE_DELIMITER "<#=#>"
#define MAX_EVENT_CACHE 200
#define T2_COMPONENT_READY    "/tmp/.t2ReadyToReceiveEvents"
#define T2_SCRIPT_EVENT_COMPONENT "telemetry_client"
#define SENDER_LOG_FILE "/tmp/t2_sender_debug.log"
#define T2_TCP_PORT 12345
#define SERVER_IP "127.0.0.1"

static const char* CCSP_FIXED_COMP_ID = "com.cisco.spvtg.ccsp.t2commonlib" ;

static char *componentName = NULL;
static void *bus_handle = NULL;
static bool isRbusEnabled = false ;
static int count = 0;

static bool isRFCT2Enable = false ;
static bool isT2Ready = false;

static hash_map_t *eventMarkerMap = NULL;

static pthread_mutex_t markerListMutex ;
static pthread_mutex_t loggerMutex ;
//static pthread_mutex_t eventMutex ;
static pthread_mutex_t FileCacheMutex ;

static int g_tcp_client_fd = -1;
static pthread_mutex_t g_tcp_client_mutex = PTHREAD_MUTEX_INITIALIZER;

// Communication mode flag - true for RBUS (default), false for Unix socket
static bool g_use_rbus_communication = true;

typedef enum
{
    T2_REQ_SUBSCRIBE = 1,
    T2_REQ_PROFILE_DATA = 2,
    T2_REQ_MARKER_LIST = 3,
    T2_REQ_DAEMON_STATUS = 4,
    T2_MSG_EVENT_DATA = 5
} T2RequestType;

typedef struct
{
    uint32_t request_type;
    uint32_t data_length;
    uint32_t client_id;
    uint32_t last_known_version;
} T2RequestHeader;

// Response header for server responses
typedef struct
{
    uint32_t response_status; // 0=success, 1=failure
    uint32_t data_length;     // Length of response data
    uint32_t sequence_id;     // Matches request sequence
    uint32_t reserved;        // For future use
} T2ResponseHeader;

static pthread_mutex_t clientMarkerMutex = PTHREAD_MUTEX_INITIALIZER;


// Message Queue Configuration for Publish-Subscribe
#define T2_MQ_DAEMON_NAME "/t2_daemon_mq"           // For sending events to daemon
#define T2_MQ_BROADCAST_NAME "/t2_mq_"     // For receiving marker updates from daemon
#define T2_MQ_MAX_MESSAGES 50
#define T2_MQ_MAX_MSG_SIZE 4096
#define T2_MQ_PERMISSIONS 0666

// Simplified Message Types
typedef enum
{
    T2_MQ_MSG_MARKER_UPDATE = 1,    // Daemon broadcasts marker updates to all clients
    T2_MQ_MSG_EVENT_DATA = 2,       // Clients send events to daemon
    T2_MQ_MSG_SUBSCRIBE = 3,        // Client subscription (optional)
    T2_MQ_MSG_STATUS_REQUEST = 4
} T2MQMessageType;

// Simplified Message Header
typedef struct
{
    T2MQMessageType msg_type;
    uint32_t data_length;
    char component_name[128];       // Component this update is for (or "ALL" for global)
    uint64_t timestamp;
    uint32_t sequence_id;           // Incremental sequence for tracking updates
} T2MQMessageHeader;

// Global state for thread-free operation
static struct
{
    mqd_t daemon_mq;                // Queue to send events to daemon
    mqd_t broadcast_mq;             // Queue to receive marker updates from daemon
    char broadcast_queue_name[256]; // Store the queue name for cleanup
    bool initialized;
    uint32_t last_sequence_id;      // Track last processed marker update
    time_t last_check_time;         // Timestamp of last marker check
    
    // 🔥 NEW: Signal-based notification with existing message handling
    bool notification_registered;
    volatile sig_atomic_t marker_update_pending;
    bool first_check_after_registration;  // 🔥 NEW: Track first check for existing messages
} g_mq_state =
{
    .daemon_mq = -1,
    .broadcast_mq = -1,
    .initialized = false,
    .last_sequence_id = 0,
    .last_check_time = 0,
    .notification_registered = false,
    .marker_update_pending = 0,
    .first_check_after_registration = false  // 🔥 NEW
};

// Message queue mutex for thread safety
static pthread_mutex_t g_mq_mutex = PTHREAD_MUTEX_INITIALIZER;

static T2TransportMode g_transport_mode = T2_TRANSPORT_MODE_MESSAGE_QUEUE; // Default to RBUS

static pthread_mutex_t g_transport_mode_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_tcp_client_connected = false;

static T2ERROR t2_unix_client_init();
static void t2_unix_client_uninit();
static T2ERROR t2_mq_request_initial_markers(void);
static T2ERROR t2_unix_client_connect();
int send_event_via_message_queue(const char* data, const char *markerName);
static void t2_mq_client_uninit(void);
static T2ERROR initMessageBus( );

static void EVENT_DEBUG(char* format, ...)
{

    if(access(ENABLE_DEBUG_FLAG, F_OK) == -1)
    {
        return;
    }

    FILE *logHandle = NULL ;

    pthread_mutex_lock(&loggerMutex);
    logHandle = fopen(SENDER_LOG_FILE, "a+");
    if(logHandle)
    {
        time_t rawtime;
        struct tm* timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        static char timeBuffer[20] = { '\0' };
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        fprintf(logHandle, "%s : ", timeBuffer);
        va_list argList;
        va_start(argList, format);
        vfprintf(logHandle, format, argList);
        va_end(argList);
        fclose(logHandle);
    }
    pthread_mutex_unlock(&loggerMutex);

}


static bool initRFC( )
{
    bool status = true ;
    // Check for RFC and proceed - if true - else return now .
    if(!bus_handle)
    {
        if(initMessageBus() != 0)
        {
            EVENT_ERROR("initMessageBus failed\n");
            status = false ;
        }
        else
        {
            status = true;
        }
        isRFCT2Enable = true;
    }

    return status;
}


/**
 * Receives an rbus object as value which conatins a list of rbusPropertyObject
 * rbusProperty name will the eventName and value will be null
 */
static T2ERROR doPopulateEventMarkerList( )
{
    T2ERROR status = T2ERROR_SUCCESS;
    char deNameSpace[1][124] = {{ '\0' }};
    if(!isRbusEnabled)
    {
        return T2ERROR_SUCCESS;
    }

    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    rbusValue_t paramValue_t;

    if(!bus_handle && T2ERROR_SUCCESS != initMessageBus())
    {
        EVENT_ERROR("Unable to get message bus handles \n");
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    snprintf(deNameSpace[0], 124, "%s%s%s", T2_ROOT_PARAMETER, componentName, T2_EVENT_LIST_PARAM_SUFFIX);
    EVENT_DEBUG("rbus mode : Query marker list with data element = %s \n", deNameSpace[0]);

    pthread_mutex_lock(&markerListMutex);
    EVENT_DEBUG("Lock markerListMutex & Clean up eventMarkerMap \n");
    if(eventMarkerMap != NULL)
    {
        hash_map_destroy(eventMarkerMap, free);
        eventMarkerMap = NULL;
    }

    ret = rbus_get(bus_handle, deNameSpace[0], &paramValue_t);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        EVENT_ERROR("rbus mode : No event list configured in profiles %s and return value %d\n", deNameSpace[0], ret);
        pthread_mutex_unlock(&markerListMutex);
        EVENT_DEBUG("rbus mode : No event list configured in profiles %s and return value %d. Unlock markerListMutex\n", deNameSpace[0], ret);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_SUCCESS;
    }

    rbusValueType_t type_t = rbusValue_GetType(paramValue_t);
    if(type_t != RBUS_OBJECT)
    {
        EVENT_ERROR("rbus mode : Unexpected data object received for %s get query \n", deNameSpace[0]);
        rbusValue_Release(paramValue_t);
        pthread_mutex_unlock(&markerListMutex);
        EVENT_DEBUG("Unlock markerListMutex\n");
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    rbusObject_t objectValue = rbusValue_GetObject(paramValue_t);
    if(objectValue)
    {
        eventMarkerMap = hash_map_create();
        rbusProperty_t rbusPropertyList = rbusObject_GetProperties(objectValue);
        EVENT_DEBUG("\t rbus mode :  Update event map for component %s with below events : \n", componentName);
        while(NULL != rbusPropertyList)
        {
            const char* eventname = rbusProperty_GetName(rbusPropertyList);
            if(eventname && strlen(eventname) > 0)
            {
                EVENT_DEBUG("\t %s\n", eventname);
                hash_map_put(eventMarkerMap, (void*) strdup(eventname), (void*) strdup(eventname), free);
            }
            rbusPropertyList = rbusProperty_GetNext(rbusPropertyList);
        }
    }
    else
    {
        EVENT_ERROR("rbus mode : No configured event markers for %s \n", componentName);
    }
    EVENT_DEBUG("Unlock markerListMutex\n");
    pthread_mutex_unlock(&markerListMutex);
    rbusValue_Release(paramValue_t);
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return status;

}

static void rbusEventReceiveHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
{
    (void)handle;//To fix compiler warning.
    (void)subscription;//To fix compiler warning.
    const char* eventName = event->name;
    if(eventName)
    {
        if(0 == strcmp(eventName, T2_PROFILE_UPDATED_NOTIFY))
        {
            doPopulateEventMarkerList();
        }
    }
    else
    {
        EVENT_ERROR("eventName is null \n");
    }
}


void rBusInterface_Uninit( )
{
    rbus_close(bus_handle);
}

static T2ERROR getRbusParameterVal(const char* paramName, char **paramValue)
{

    rbusError_t ret = RBUS_ERROR_SUCCESS;
    rbusValue_t paramValue_t;
    rbusValueType_t rbusValueType ;
    char *stringValue = NULL;
#if 0
    rbusSetOptions_t opts;
    opts.commit = true;
#endif

    if(!bus_handle && T2ERROR_SUCCESS != initMessageBus())
    {
        return T2ERROR_FAILURE;
    }

    ret = rbus_get(bus_handle, paramName, &paramValue_t);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        EVENT_ERROR("Unable to get %s\n", paramName);
        return T2ERROR_FAILURE;
    }
    rbusValueType = rbusValue_GetType(paramValue_t);
    if(rbusValueType == RBUS_BOOLEAN)
    {
        if (rbusValue_GetBoolean(paramValue_t))
        {
            stringValue = strdup("true");
        }
        else
        {
            stringValue = strdup("false");
        }
    }
    else
    {
        stringValue = rbusValue_ToString(paramValue_t, NULL, 0);
    }
    *paramValue = stringValue;
    rbusValue_Release(paramValue_t);

    return T2ERROR_SUCCESS;
}

T2ERROR getParamValue(const char* paramName, char **paramValue)
{
    T2ERROR ret = T2ERROR_FAILURE ;
    if(isRbusEnabled)
    {
        ret = getRbusParameterVal(paramName, paramValue);
    }
#if defined(CCSP_SUPPORT_ENABLED)
    else
    {
        ret = getCCSPParamVal(paramName, paramValue);
    }
#endif

    return ret;
}

// Function to check if an event marker is valid for this component
static bool is_valid_event_marker(const char* markerName)
{
    if (!markerName || !eventMarkerMap)
    {
        return false;
    }

    pthread_mutex_lock(&clientMarkerMutex);
    char* found_marker = (char*)hash_map_get(eventMarkerMap, markerName);
    pthread_mutex_unlock(&clientMarkerMutex);

    return (found_marker != NULL);
}

static T2ERROR initMessageBus( )
{
    // EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    T2ERROR status = T2ERROR_SUCCESS;
    char* component_id = (char*)CCSP_FIXED_COMP_ID;
#if defined(CCSP_SUPPORT_ENABLED)
    char *pCfg = (char*)CCSP_MSG_BUS_CFG;
#endif

    if(RBUS_ENABLED == rbus_checkStatus())
    {
        // EVENT_DEBUG("%s:%d, T2:rbus is enabled\n", __func__, __LINE__);
        char commonLibName[124] = { '\0' };
        // Bus handles should be unique across the system
        if(componentName)
        {
            snprintf(commonLibName, 124, "%s%s", "t2_lib_", componentName);
        }
        else
        {
            snprintf(commonLibName, 124, "%s", component_id);
        }
        rbusError_t status_rbus =  rbus_open((rbusHandle_t*) &bus_handle, commonLibName);
        if(status_rbus != RBUS_ERROR_SUCCESS)
        {
            EVENT_ERROR("%s:%d, init using component name %s failed with error code %d \n", __func__, __LINE__, commonLibName, status);
            status = T2ERROR_FAILURE;
        }
        isRbusEnabled = true;
    }
#if defined(CCSP_SUPPORT_ENABLED)
    else
    {
        int ret = 0 ;
        ret = CCSP_Message_Bus_Init(component_id, pCfg, &bus_handle, (CCSP_MESSAGE_BUS_MALLOC)Ansc_AllocateMemory_Callback, Ansc_FreeMemory_Callback);
        if(ret == -1)
        {
            EVENT_ERROR("%s:%d, T2:initMessageBus failed\n", __func__, __LINE__);
            status = T2ERROR_FAILURE ;
        }
        else
        {
            status = T2ERROR_SUCCESS ;
        }
    }
#endif // CCSP_SUPPORT_ENABLED 
    // EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return status;
}

/**
 * Set transport communication mode
 * @param mode - Transport mode to use
 */
void t2_set_transport_mode(T2TransportMode mode)
{
    pthread_mutex_lock(&g_transport_mode_mutex);
    g_transport_mode = mode;
    pthread_mutex_unlock(&g_transport_mode_mutex);

    const char* mode_name = (mode == T2_TRANSPORT_MODE_RBUS) ? "RBUS" :
                            (mode == T2_TRANSPORT_MODE_UNIX_SOCKET) ? "Unix Socket" :
                            (mode == T2_TRANSPORT_MODE_MESSAGE_QUEUE) ? "Message Queue" : "Unknown";

    EVENT_DEBUG("Transport mode set to: %s\n", mode_name);
    printf("Transport mode set to: %s\n", mode_name);
}

/**
 * Get current transport communication mode
 * @return Current transport mode
 */
T2TransportMode t2_get_transport_mode(void)
{
    pthread_mutex_lock(&g_transport_mode_mutex);
    T2TransportMode mode = g_transport_mode;
    pthread_mutex_unlock(&g_transport_mode_mutex);
    return mode;
}

/**
 * Set transport mode from environment variable
 * Environment variable T2_TRANSPORT_MODE can be:
 * - "rbus" (default)
 * - "unix_socket"
 * - "message_queue" or "mq"
 */
void t2_set_transport_mode_from_env(void)
{
    const char* transport_env = getenv("T2_TRANSPORT_MODE");
    if (!transport_env)
    {
        printf("%s: T2_TRANSPORT_MODE not set, defaulting to RBUS\n", __FUNCTION__);
        t2_set_transport_mode(T2_TRANSPORT_MODE_MESSAGE_QUEUE); // Default
        return;
    }

    if (strcasecmp(transport_env, "unix_socket") == 0 ||
            strcasecmp(transport_env, "unix") == 0)
    {
        t2_set_transport_mode(T2_TRANSPORT_MODE_UNIX_SOCKET);
    }
    else if (strcasecmp(transport_env, "message_queue") == 0 ||
             strcasecmp(transport_env, "mq") == 0)
    {
        t2_set_transport_mode(T2_TRANSPORT_MODE_MESSAGE_QUEUE);
    }
    else if (strcasecmp(transport_env, "rbus") == 0)
    {
        t2_set_transport_mode(T2_TRANSPORT_MODE_RBUS);
    }
    else
    {
        EVENT_ERROR("Unknown transport mode: %s, defaulting to RBUS\n", transport_env);
        t2_set_transport_mode(T2_TRANSPORT_MODE_RBUS);
    }
}

/**
 * Get transport mode name as string
 */
const char* t2_get_transport_mode_name(void)
{
    T2TransportMode mode = t2_get_transport_mode();
    switch (mode)
    {
    case T2_TRANSPORT_MODE_RBUS:
        return "RBUS";
    case T2_TRANSPORT_MODE_UNIX_SOCKET:
        return "Unix Socket";
    case T2_TRANSPORT_MODE_MESSAGE_QUEUE:
        return "Message Queue";
    default:
        return "Unknown";
    }
}

// 🔥 NEW: Signal handler for marker update notifications
static void marker_update_signal_handler(int sig)
{
    if (sig == SIGUSR1) {
        g_mq_state.marker_update_pending = 1;
    }
}

// 🔥 ENHANCED: Register for queue notifications with first check flag
static T2ERROR t2_mq_register_notification(void)
{
    if (g_mq_state.broadcast_mq == -1 || g_mq_state.notification_registered) {
        return T2ERROR_SUCCESS;
    }
    
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    printf("%s ++in\n", __FUNCTION__);
    
    // Install signal handler
    struct sigaction sa;
    sa.sa_handler = marker_update_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        EVENT_ERROR("Failed to install signal handler: %s\n", strerror(errno));
        return T2ERROR_FAILURE;
    }
    
    // Register for notification when message arrives
    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGUSR1;
    sev.sigev_value.sival_ptr = NULL;
    
    if (mq_notify(g_mq_state.broadcast_mq, &sev) == -1) {
        EVENT_ERROR("Failed to register mq_notify: %s\n", strerror(errno));
        printf("Failed to register mq_notify: %s\n", strerror(errno));
        return T2ERROR_FAILURE;
    }
    
    g_mq_state.notification_registered = true;
    g_mq_state.first_check_after_registration = true;  // 🔥 NEW: Set flag for first check
    
    EVENT_DEBUG("Successfully registered for marker update notifications\n");
    printf("Successfully registered for marker update notifications\n");
    
    return T2ERROR_SUCCESS;
}

/**
 * Initialize POSIX message queue communication (THREAD-FREE)
 */
static T2ERROR t2_mq_client_init(void)
{
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    printf("%s ++in\n", __FUNCTION__);

    pthread_mutex_lock(&g_mq_mutex);

    if (g_mq_state.initialized)
    {
        pthread_mutex_unlock(&g_mq_mutex);
        return T2ERROR_SUCCESS;
    }
    
    char component_broadcast_queue[256];
    // Create component-specific broadcast queue name: "/t2_mq_wifi" or "/t2_mq_default"
    snprintf(component_broadcast_queue, sizeof(component_broadcast_queue),
             "%s%s", T2_MQ_BROADCAST_NAME, componentName ? componentName : "default");

    // Open daemon queue for sending events (non-blocking)
    g_mq_state.daemon_mq = mq_open(T2_MQ_DAEMON_NAME, O_WRONLY | O_NONBLOCK);
    if (g_mq_state.daemon_mq == -1)
    {
        EVENT_ERROR("Failed to open daemon message queue: %s\n", strerror(errno));
        printf("Failed to open daemon message queue: %s\n", strerror(errno));
    }
    else
    {
        EVENT_DEBUG("Successfully connected to daemon message queue\n");
        printf("Successfully connected to daemon message queue\n");
    }

    // Open broadcast queue for receiving marker updates (non-blocking, read-only)
    g_mq_state.broadcast_mq = mq_open(component_broadcast_queue, O_RDONLY | O_NONBLOCK);
    if (g_mq_state.broadcast_mq == -1)
    {
        EVENT_ERROR("Failed to open broadcast message queue: %s\n", strerror(errno));
        printf("Failed to open broadcast message queue: %s\n", strerror(errno));
    }
    else
    {
        EVENT_DEBUG("Successfully opened component broadcast queue: %s\n", component_broadcast_queue);
        printf("Successfully opened component broadcast queue: %s\n", component_broadcast_queue);
        
        // 🔥 NEW: Register for notifications instead of polling
        t2_mq_register_notification();
    }

    g_mq_state.initialized = true;
    g_mq_state.last_check_time = time(NULL);
    g_mq_state.marker_update_pending = 0;
    strncpy(g_mq_state.broadcast_queue_name, component_broadcast_queue,
            sizeof(g_mq_state.broadcast_queue_name) - 1);
    g_mq_state.broadcast_queue_name[sizeof(g_mq_state.broadcast_queue_name) - 1] = '\0';

    pthread_mutex_unlock(&g_mq_mutex);

    EVENT_DEBUG("Message queue client initialized with notifications\n");
    printf("Message queue client initialized with notifications\n");

    return T2ERROR_SUCCESS;
}

/**
 * Initialize communication subsystem based on selected mode
 */
/**
 * Initialize communication subsystem based on selected mode (ALL 3 MODES)
 */
T2ERROR t2_communication_init(char *component)
{
    T2TransportMode mode = t2_get_transport_mode();
    const char* mode_name = t2_get_transport_mode_name();
    componentName = strdup(component);

    EVENT_DEBUG("%s ++in (mode: %s)\n", __FUNCTION__, mode_name);
    printf("%s ++in (mode: %s)\n", __FUNCTION__, mode_name);

    T2ERROR status = T2ERROR_SUCCESS;

    switch (mode)
    {
    case T2_TRANSPORT_MODE_RBUS:
        // Initialize RBUS communication
        if (!bus_handle)
        {
            status = initMessageBus();
            if (status != T2ERROR_SUCCESS)
            {
                EVENT_ERROR("Failed to initialize RBUS communication\n");
                return status;
            }
        }
        EVENT_DEBUG("RBUS communication initialized successfully\n");
        break;

    case T2_TRANSPORT_MODE_UNIX_SOCKET:
        t2_set_communication_mode(false);
        // Initialize Unix socket communication
        status = t2_unix_client_init();
        if (status != T2ERROR_SUCCESS)
        {
            EVENT_ERROR("Failed to initialize Unix socket communication\n");
            return status;
        }
        EVENT_DEBUG("Unix socket communication initialized successfully\n");
        break;

    case T2_TRANSPORT_MODE_MESSAGE_QUEUE:
        // Initialize Message Queue communication
        status = t2_mq_client_init();
        if (status != T2ERROR_SUCCESS)
        {
            EVENT_ERROR("Failed to initialize Message Queue communication\n");
            return status;
        }

        // Request initial marker list from daemon for MQ mode
        t2_mq_request_initial_markers();
        EVENT_DEBUG("Message Queue communication initialized successfully\n");
        break;

    default:
        EVENT_ERROR("Unknown transport mode: %d\n", mode);
        return T2ERROR_FAILURE;
    }

    EVENT_DEBUG("%s --out with status %d\n", __FUNCTION__, status);
    return status;
}

/**
 * Communication abstraction functions
 */

/**
 * Set communication mode
 * @param use_rbus - true for RBUS communication, false for Unix socket communication
 */
void t2_set_communication_mode(bool use_rbus)
{
    g_use_rbus_communication = use_rbus;
    EVENT_DEBUG("Communication mode set to: %s\n", use_rbus ? "RBUS" : "Unix Socket");
    printf("Communication mode set to: %s\n", use_rbus ? "RBUS" : "Unix Socket");
}

/**
 * Get current communication mode
 * @return true if RBUS mode, false if Unix socket mode
 */
bool t2_get_communication_mode(void)
{
    return g_use_rbus_communication;
}

void *cacheEventToFile(void *arg)
{
    char *telemetry_data = (char *)arg;
    int fd;
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;
    FILE *fs = NULL;
    char path[100];
    pthread_detach(pthread_self());
    EVENT_ERROR("%s:%d, Caching the event to File\n", __func__, __LINE__);
    if(telemetry_data == NULL)
    {
        EVENT_ERROR("%s:%d, Data is NULL\n", __func__, __LINE__);
        return NULL;
    }
    pthread_mutex_lock(&FileCacheMutex);

    if ((fd = open(T2_CACHE_LOCK_FILE, O_RDWR | O_CREAT, 0666)) == -1)
    {
        EVENT_ERROR("%s:%d, T2:open failed\n", __func__, __LINE__);
        pthread_mutex_unlock(&FileCacheMutex);
        free(telemetry_data);
        return NULL;
    }

    if(fcntl(fd, F_SETLKW, &fl) == -1)  /* set the lock */
    {
        EVENT_ERROR("%s:%d, T2:fcntl failed\n", __func__, __LINE__);
        pthread_mutex_unlock(&FileCacheMutex);
        int ret = close(fd);
        if (ret != 0)
        {
            EVENT_ERROR("%s:%d, T2:close failed with error %d\n", __func__, __LINE__, ret);
        }
        free(telemetry_data);
        return NULL;
    }

    FILE *fp = fopen(T2_CACHE_FILE, "a");
    if (fp == NULL)
    {
        EVENT_ERROR("%s: File open error %s\n", __FUNCTION__, T2_CACHE_FILE);
        goto unlock;
    }
    fs = popen ("cat /tmp/t2_caching_file | wc -l", "r");
    if(fs != NULL)
    {
        fgets(path, 100, fs);
        count = atoi ( path );
        pclose(fs);
    }
    if(count < MAX_EVENT_CACHE)
    {
        fprintf(fp, "%s\n", telemetry_data);
    }
    else
    {
        EVENT_DEBUG("Reached Max cache limit of 200, Caching is not done\n");
    }
    fclose(fp);

unlock:

    fl.l_type = F_UNLCK;  /* set to unlock same region */
    if (fcntl(fd, F_SETLK, &fl) == -1)
    {
        EVENT_ERROR("fcntl failed \n");
    }
    int ret = close(fd);
    if (ret != 0)
    {
        EVENT_ERROR("%s:%d, T2:close failed with error %d\n", __func__, __LINE__, ret);
    }
    pthread_mutex_unlock(&FileCacheMutex);
    free(telemetry_data);
    return NULL;
}

/**
 * In rbus mode, should be using rbus subscribed param
 * from telemetry 2.0 instead of direct api for event sending
 */
int filtered_event_send(const char* data, const char *markerName)
{
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    int status = 0 ;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    printf("%s ++in\n", __FUNCTION__);

    if(!bus_handle)
    {
        EVENT_ERROR("bus_handle is null .. exiting !!! \n");
        return ret;
    }

    if(isRbusEnabled)
    {

        // Filter data from marker list
        if(componentName && (0 != strcmp(componentName, T2_SCRIPT_EVENT_COMPONENT)))   // Events from scripts needs to be sent without filtering
        {

            EVENT_DEBUG("%s markerListMutex lock & get list of marker for component %s \n", __FUNCTION__, componentName);
            pthread_mutex_lock(&markerListMutex);
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
                EVENT_DEBUG("%s eventMarkerMap for component %s is empty \n", __FUNCTION__, componentName );
            }
            EVENT_DEBUG("%s markerListMutex unlock\n", __FUNCTION__ );
            pthread_mutex_unlock(&markerListMutex);
            if(!isEventingEnabled)
            {
                EVENT_DEBUG("%s markerName %s not found in event list for component %s . Unlock markerListMutex . \n", __FUNCTION__, markerName, componentName);
                return status;
            }
        }
        // End of event filtering

        rbusProperty_t objProperty = NULL ;
        rbusValue_t objVal, value;
        rbusSetOptions_t options = {0};
        options.commit = true;

        rbusValue_Init(&objVal);
        rbusValue_SetString(objVal, data);
        rbusProperty_Init(&objProperty, markerName, objVal);

        rbusValue_Init(&value);
        rbusValue_SetProperty(value, objProperty);

        EVENT_DEBUG("rbus_set with param [%s] with %s and value [%s]\n", T2_EVENT_PARAM, markerName, data);
        EVENT_DEBUG("rbus_set with param [%s] with %s and value [%s]\n", T2_EVENT_PARAM, markerName, data);
        ret = rbus_set(bus_handle, T2_EVENT_PARAM, value, &options);
        if(ret != RBUS_ERROR_SUCCESS)
        {
            EVENT_ERROR("rbus_set Failed for [%s] with error [%d]\n", T2_EVENT_PARAM, ret);
            EVENT_DEBUG(" !!! Error !!! rbus_set Failed for [%s] with error [%d]\n", T2_EVENT_PARAM, ret);
            status = -1 ;
        }
        else
        {
            status = 0 ;
        }
        // Release all rbus data structures
        rbusValue_Release(value);
        rbusProperty_Release(objProperty);
        rbusValue_Release(objVal);

    }
#if defined(CCSP_SUPPORT_ENABLED)
    else
    {
        int eventDataLen = strlen(markerName) + strlen(data) + strlen(MESSAGE_DELIMITER) + 1;
        char* buffer = (char*) malloc(eventDataLen * sizeof(char));
        if(buffer)
        {
            snprintf(buffer, eventDataLen, "%s%s%s", markerName, MESSAGE_DELIMITER, data);
            ret = CcspBaseIf_SendTelemetryDataSignal(bus_handle, buffer);
            if(ret != CCSP_SUCCESS)
            {
                status = -1;
            }
            free(buffer);
        }
        else
        {
            EVENT_ERROR("Unable to allocate meory for event [%s]\n", markerName);
            status = -1 ;
        }
    }
#endif // CCSP_SUPPORT_ENABLED 

    EVENT_DEBUG("%s --out with status %d \n", __FUNCTION__, status);
    return status;
}

static bool isCachingRequired( )
{

    /**
     * Attempts to read from PAM before its ready creates deadlock .
     * PAM not ready is a definite case for caching the event and avoid bus traffic
     * */
#if defined(ENABLE_RDKB_SUPPORT)
    if (access( "/tmp/pam_initialized", F_OK ) != 0)
    {
        return true;
    }
#endif

    if(!initRFC())
    {
        EVENT_ERROR("initRFC failed - cache the events\n");
        return true;
    }

    // If feature is disabled by RFC, caching is always disabled
    if(!isRFCT2Enable)
    {
        return false ;
    }

    // Always check for t2 is ready to accept events. Shutdown target can bring down t2 process at runtime
    uint32_t t2ReadyStatus;
    rbusError_t retVal = RBUS_ERROR_SUCCESS;

    retVal = rbus_getUint(bus_handle, T2_OPERATIONAL_STATUS, &t2ReadyStatus);

    if(retVal != RBUS_ERROR_SUCCESS)
    {
        return true;
    }
    else
    {
        EVENT_DEBUG("value for  %s is : %d\n", T2_OPERATIONAL_STATUS, t2ReadyStatus);
        if((t2ReadyStatus & T2_STATE_COMPONENT_READY) == 0)
        {
            return true;
        }
    }

    if(!isRbusEnabled)
    {
        isT2Ready = true;
    }

    if(!isT2Ready)
    {
        if(componentName && (0 != strcmp(componentName, "telemetry_client")))
        {
            // From other binary applications in rbus mode if t2 daemon is yet to determine state of component specific config from cloud, enable cache
            if((t2ReadyStatus & T2_STATE_COMPONENT_READY) == 0)
            {
                return true;
            }
            else
            {
                rbusError_t ret = RBUS_ERROR_SUCCESS;
                doPopulateEventMarkerList();
                ret = rbusEvent_Subscribe(bus_handle, T2_PROFILE_UPDATED_NOTIFY, rbusEventReceiveHandler, "T2Event", 0);
                if(ret != RBUS_ERROR_SUCCESS)
                {
                    EVENT_ERROR("Unable to subscribe to event %s with rbus error code : %d\n", T2_PROFILE_UPDATED_NOTIFY, ret);
                    EVENT_DEBUG("Unable to subscribe to event %s with rbus error code : %d\n", T2_PROFILE_UPDATED_NOTIFY, ret);
                }
                isT2Ready = true;
            }
        }
        else
        {
            isT2Ready = true;
        }
    }

    return false;
}

int report_or_cache_data(char* telemetry_data, const char* markerName)
{
    printf("%s:%d, report_or_cache_data called\n", __func__, __LINE__);
    int ret = 0;


    // EVENT_DEBUG("T2: Sending event : %s\n", telemetry_data);
    printf("T2: Sending event : %s\n", telemetry_data);

    // Use the new unified communication function
    ret = send_event_data(telemetry_data, markerName);
    if(0 != ret)
    {
        EVENT_ERROR("%s:%d, T2:telemetry data send failed, status = %d \n", __func__, __LINE__, ret);
    }

    return ret;
}


/**
 * Send event data via RBUS communication
 */
static int send_event_via_rbus(const char* data, const char *markerName)
{
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    int status = 0;

    EVENT_DEBUG("%s ++in (RBUS mode)\n", __FUNCTION__);
    printf("%s ++in (RBUS mode)\n", __FUNCTION__);

    if(!bus_handle)
    {
        EVENT_ERROR("bus_handle is null .. exiting !!! \n");
        return -1;
    }

    if(isRbusEnabled)
    {
        // Filter data from marker list
        if(componentName && (0 != strcmp(componentName, T2_SCRIPT_EVENT_COMPONENT)))
        {
            EVENT_DEBUG("%s markerListMutex lock & get list of marker for component %s \n", __FUNCTION__, componentName);
            pthread_mutex_lock(&markerListMutex);
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
                EVENT_DEBUG("%s eventMarkerMap for component %s is empty \n", __FUNCTION__, componentName );
            }
            EVENT_DEBUG("%s markerListMutex unlock\n", __FUNCTION__ );
            pthread_mutex_unlock(&markerListMutex);
            if(!isEventingEnabled)
            {
                EVENT_DEBUG("%s markerName %s not found in event list for component %s . Unlock markerListMutex . \n", __FUNCTION__, markerName, componentName);
                return 0;
            }
        }

        rbusProperty_t objProperty = NULL ;
        rbusValue_t objVal, value;
        rbusSetOptions_t options = {0};
        options.commit = true;

        rbusValue_Init(&objVal);
        rbusValue_SetString(objVal, data);
        rbusProperty_Init(&objProperty, markerName, objVal);

        rbusValue_Init(&value);
        rbusValue_SetProperty(value, objProperty);

        EVENT_DEBUG("rbus_set with param [%s] with %s and value [%s]\n", T2_EVENT_PARAM, markerName, data);
        printf("rbus_set with param [%s] with %s and value [%s]\n", T2_EVENT_PARAM, markerName, data);
        ret = rbus_set(bus_handle, T2_EVENT_PARAM, value, &options);
        if(ret != RBUS_ERROR_SUCCESS)
        {
            EVENT_ERROR("rbus_set Failed for [%s] with error [%d]\n", T2_EVENT_PARAM, ret);
            EVENT_DEBUG(" !!! Error !!! rbus_set Failed for [%s] with error [%d]\n", T2_EVENT_PARAM, ret);
            status = -1;
        }
        else
        {
            status = 0;
        }

        // Release all rbus data structures
        rbusValue_Release(value);
        rbusProperty_Release(objProperty);
        rbusValue_Release(objVal);
    }
#if defined(CCSP_SUPPORT_ENABLED)
    else
    {
        int eventDataLen = strlen(markerName) + strlen(data) + strlen(MESSAGE_DELIMITER) + 1;
        char* buffer = (char*) malloc(eventDataLen * sizeof(char));
        if(buffer)
        {
            snprintf(buffer, eventDataLen, "%s%s%s", markerName, MESSAGE_DELIMITER, data);
            ret = CcspBaseIf_SendTelemetryDataSignal(bus_handle, buffer);
            if(ret != CCSP_SUCCESS)
            {
                status = -1;
            }
            free(buffer);
        }
        else
        {
            EVENT_ERROR("Unable to allocate memory for event [%s]\n", markerName);
            status = -1;
        }
    }
#endif

    EVENT_DEBUG("%s --out (RBUS mode) with status %d\n", __FUNCTION__, status);
    return status;
}

/**
 * Send event data via Unix socket communication
 */
static int send_event_via_unix_socket(const char* data, const char *markerName)
{
    int status = 0;

    EVENT_DEBUG("%s ++in (Unix Socket mode)\n", __FUNCTION__);
    printf("%s ++in (Unix Socket mode)\n", __FUNCTION__);

    if (g_tcp_client_fd < 0)
    {
        EVENT_DEBUG("TCP client not connected, attempting to connect\n");
        if (t2_unix_client_connect() != T2ERROR_SUCCESS)
        {
            EVENT_ERROR("Failed to connect to TCP server\n");
            return -1;
        }
    }

    // Validate marker against client event map
    if(!is_valid_event_marker(markerName))
    {
        EVENT_DEBUG("%s markerName %s not found in event list for component %s\n", __FUNCTION__, markerName, componentName);
        printf("%s markerName %s not found in event list for component %s\n", __FUNCTION__, markerName, componentName);
        return 0; // Not an error, just filtered out
    }

    // Create event message in format "markerName<#=#>eventValue"
    int eventDataLen = strlen(markerName) + strlen(data) + strlen(MESSAGE_DELIMITER) + 1;
    char* event_message = malloc(eventDataLen);
    if (!event_message)
    {
        EVENT_ERROR("Failed to allocate memory for event message\n");
        return -1;
    }

    snprintf(event_message, eventDataLen, "%s%s%s", markerName, MESSAGE_DELIMITER, data);

    pthread_mutex_lock(&g_tcp_client_mutex);

    // Create TCP request header
    T2RequestHeader req_header =
    {
        .request_type = T2_MSG_EVENT_DATA,
        .data_length = strlen(event_message),
        .client_id = (uint32_t)(getpid() ^ time(NULL)),
        .last_known_version = 0
    };

    // Send header
    ssize_t sent = send(g_tcp_client_fd, &req_header, sizeof(req_header), MSG_NOSIGNAL);
    if (sent == sizeof(req_header))
    {
        // Send event data
        sent = send(g_tcp_client_fd, event_message, strlen(event_message), MSG_NOSIGNAL);
        if (sent == (ssize_t)strlen(event_message))
        {
            EVENT_DEBUG("TCP event sent successfully: %s\n", event_message);
            printf("TCP event sent successfully: %s\n", event_message);
            status = 0;
        }
        else
        {
            EVENT_ERROR("Failed to send event data via TCP: %s\n", strerror(errno));
            printf("Failed to send event data via TCP: %s\n", strerror(errno));
            status = -1;
        }
    }
    else
    {
        EVENT_ERROR("Failed to send event header via TCP: %s\n", strerror(errno));
        printf("Failed to send event header via TCP: %s\n", strerror(errno));
        status = -1;
    }

    pthread_mutex_unlock(&g_tcp_client_mutex);
    free(event_message);

    EVENT_DEBUG("%s --out (Unix Socket mode) with status %d\n", __FUNCTION__, status);
    return status;
}

/**
 * Unified communication function that routes to appropriate communication method (ALL 3 MODES)
 */
int send_event_data(const char* data, const char *markerName)
{
    T2TransportMode mode = t2_get_transport_mode();
    const char* mode_name = t2_get_transport_mode_name();

    EVENT_DEBUG("%s ++in (mode: %s)\n", __FUNCTION__, mode_name);
    printf("%s ++in (mode: %s)\n", __FUNCTION__, mode_name);

    int status = 0;

    switch (mode)
    {

    case T2_TRANSPORT_MODE_RBUS:
        pthread_t tid;
        //pthread_mutex_lock(&eventMutex);

        if(isCachingRequired())
        {
            EVENT_DEBUG("Caching the event : %s \n", data);
            printf("Caching the event : %s \n", data);
            int eventDataLen = strlen(markerName) + strlen(data) + strlen(MESSAGE_DELIMITER) + 1;
            char* buffer = (char*) malloc(eventDataLen * sizeof(char));
            if(buffer)
            {
                // Caching format needs to be same for operation between rbus/dbus modes across reboots
                snprintf(buffer, eventDataLen, "%s%s%s", markerName, MESSAGE_DELIMITER, data);
                pthread_create(&tid, NULL, cacheEventToFile, (void *)buffer);
            }
            //pthread_mutex_unlock(&eventMutex);
            return T2ERROR_SUCCESS ;
        }
        //pthread_mutex_unlock(&eventMutex);
        status = send_event_via_rbus(data, markerName);
        break;

    case T2_TRANSPORT_MODE_UNIX_SOCKET:
        // For Unix socket mode, query event markers from daemon
        if (t2_query_event_markers() == T2ERROR_SUCCESS)
        {
            EVENT_ERROR("Successfully retrieved event markers for %s\n", componentName);
        }
        else
        {
            EVENT_ERROR("Failed to retrieve event markers for %s\n", componentName);
        }
        status = send_event_via_unix_socket(data, markerName);
        break;

    case T2_TRANSPORT_MODE_MESSAGE_QUEUE:
        status = send_event_via_message_queue(data, markerName);
        break;

    default:
        EVENT_ERROR("Unknown transport mode: %d\n", mode);
        status = -1;
        break;
    }

    EVENT_DEBUG("%s --out with status %d (mode: %s)\n", __FUNCTION__, status, mode_name);
    return status;
}

/**
 * Cleanup communication subsystem (ALL 3 MODES)
 */
void t2_communication_cleanup(void)
{
    T2TransportMode mode = t2_get_transport_mode();
    const char* mode_name = t2_get_transport_mode_name();

    EVENT_DEBUG("%s ++in (mode: %s)\n", __FUNCTION__, mode_name);

    switch (mode)
    {
    case T2_TRANSPORT_MODE_RBUS:
        // RBUS cleanup is handled in existing t2_uninit function
        EVENT_DEBUG("RBUS cleanup handled by t2_uninit\n");
        break;

    case T2_TRANSPORT_MODE_UNIX_SOCKET:
        t2_unix_client_uninit();
        EVENT_DEBUG("Unix socket communication cleaned up\n");
        break;

    case T2_TRANSPORT_MODE_MESSAGE_QUEUE:
        t2_mq_client_uninit();
        EVENT_DEBUG("Message Queue communication cleaned up\n");
        break;

    default:
        EVENT_ERROR("Unknown transport mode during cleanup: %d\n", mode);
        break;
    }

    EVENT_DEBUG("%s --out\n", __FUNCTION__);
}

/**
 * Cleanup message queue resources
 */
static void t2_mq_client_uninit(void)
{
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);

    pthread_mutex_lock(&g_mq_mutex);

    if (!g_mq_state.initialized)
    {
        pthread_mutex_unlock(&g_mq_mutex);
        return;
    }

    // Close message queues
    if (g_mq_state.daemon_mq != -1)
    {
        mq_close(g_mq_state.daemon_mq);
        g_mq_state.daemon_mq = -1;
    }

    if (g_mq_state.broadcast_mq != -1)
    {
        mq_close(g_mq_state.broadcast_mq);
        g_mq_state.broadcast_mq = -1;
    }

    g_mq_state.initialized = false;
    g_mq_state.last_sequence_id = 0;

    pthread_mutex_unlock(&g_mq_mutex);

    EVENT_DEBUG("Message queue client uninitialized\n");
}

/**
 * Check for marker updates from broadcast queue (NON-BLOCKING, NO THREADS)
 * This is called before every event send operation
 */
// 🔥 ENHANCED: Process pending updates with existing message check
static void t2_mq_process_pending_updates(void)
{
    // Check for pending updates OR first check after registration
    if (!g_mq_state.marker_update_pending && !g_mq_state.first_check_after_registration) {
        return;  // 🔥 FAST EXIT - but only if NOT first check
    }
    
    // Reset flags
    g_mq_state.marker_update_pending = 0;
    bool is_first_check = g_mq_state.first_check_after_registration;
    g_mq_state.first_check_after_registration = false;
    
    if (!g_mq_state.initialized || g_mq_state.broadcast_mq == -1) {
        return;
    }
    
    if (is_first_check) {
        EVENT_DEBUG("Processing existing messages in queue (first check after registration)\n");
        printf("Processing existing messages in queue (first check after registration)\n");
    } else {
        EVENT_DEBUG("Processing pending marker updates (signal-driven)\n");
        printf("Processing pending marker updates (signal-driven)\n");
    }
    
    char message[T2_MQ_MAX_MSG_SIZE];
    ssize_t msg_size;
    bool updates_processed = false;
    bool need_reregister = false;
    
    // Process ALL available messages (both existing and new)
    while ((msg_size = mq_receive(g_mq_state.broadcast_mq, message, T2_MQ_MAX_MSG_SIZE, NULL)) > 0)
    {
        T2MQMessageHeader* header = (T2MQMessageHeader*)message;
        
        EVENT_DEBUG("Received message type: %d from daemon\n", header->msg_type);
        printf("Received message type: %d from daemon\n", header->msg_type);
        
        if (header->msg_type == T2_MQ_MSG_MARKER_UPDATE &&
            header->sequence_id > g_mq_state.last_sequence_id)
        {
            bool is_for_us = (strcmp(header->component_name, "ALL") == 0) ||
                           (componentName && strcmp(header->component_name, componentName) == 0);
            
            if (is_for_us && header->data_length > 0)
            {
                char* marker_data = message + sizeof(T2MQMessageHeader);
                marker_data[header->data_length] = '\0';
                
                EVENT_DEBUG("Processing marker update: %s (seq: %u)\n", marker_data, header->sequence_id);
                printf("Processing marker update: %s (seq: %u)\n", marker_data, header->sequence_id);
                
                // Update local event marker map
                t2_parse_and_store_markers(marker_data);
                g_mq_state.last_sequence_id = header->sequence_id;
                updates_processed = true;
                need_reregister = true;  // Need to re-register after processing
            }
        }
    }
    
    // Re-register for next notification if we processed any messages
    // (mq_notify is one-shot and gets consumed when queue becomes non-empty)
    if (need_reregister && g_mq_state.broadcast_mq != -1) {
        struct sigevent sev;
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIGUSR1;
        sev.sigev_value.sival_ptr = NULL;
        
        if (mq_notify(g_mq_state.broadcast_mq, &sev) == -1) {
            EVENT_ERROR("Failed to re-register mq_notify: %s\n", strerror(errno));
            printf("Failed to re-register mq_notify: %s\n", strerror(errno));
        } else {
            EVENT_DEBUG("Re-registered mq_notify for future messages\n");
            printf("Re-registered mq_notify for future messages\n");
        }
    }
    
    if (errno != EAGAIN && errno != EWOULDBLOCK && msg_size == -1) {
        EVENT_ERROR("Error receiving marker update: %s\n", strerror(errno));
        printf("Error receiving marker update: %s\n", strerror(errno));
    } else if (updates_processed) {
        EVENT_DEBUG("Successfully processed marker updates\n");
        printf("Successfully processed marker updates\n");
    } else if (is_first_check) {
        EVENT_DEBUG("No existing messages found in queue\n");
        printf("No existing messages found in queue\n");
    }
}

/**
 * Send message to daemon via message queue (NON-BLOCKING)
 */
static T2ERROR t2_mq_send_to_daemon(T2MQMessageType msg_type, const char* data, uint32_t data_len)
{
    EVENT_DEBUG("%s ++in (msg_type: %d)\n", __FUNCTION__, msg_type);
    printf("%s ++in (msg_type: %d)\n", __FUNCTION__, msg_type);

    if (!g_mq_state.initialized)
    {
        EVENT_ERROR("Message queue not initialized\n");
        return T2ERROR_FAILURE;
    }

    // Try to open daemon queue if not already open
    if (g_mq_state.daemon_mq == -1)
    {
        g_mq_state.daemon_mq = mq_open(T2_MQ_DAEMON_NAME, O_WRONLY | O_NONBLOCK);
        if (g_mq_state.daemon_mq == -1)
        {
            EVENT_ERROR("Daemon message queue not available: %s\n", strerror(errno));
            return T2ERROR_FAILURE;
        }
    }

    // Prepare message
    char message[T2_MQ_MAX_MSG_SIZE];
    T2MQMessageHeader* header = (T2MQMessageHeader*)message;

    header->msg_type = msg_type;
    header->data_length = data_len;
    header->timestamp = (uint64_t)time(NULL);
    header->sequence_id = 0;  // Not used for client-to-daemon messages

    if (componentName)
    {
        strncpy(header->component_name, componentName, sizeof(header->component_name) - 1);
        header->component_name[sizeof(header->component_name) - 1] = '\0';
    }
    else
    {
        strcpy(header->component_name, "default");
    }

    // Copy data after header
    if (data && data_len > 0)
    {
        if (sizeof(T2MQMessageHeader) + data_len > T2_MQ_MAX_MSG_SIZE)
        {
            EVENT_ERROR("Message too large: %zu bytes\n", sizeof(T2MQMessageHeader) + data_len);
            return T2ERROR_FAILURE;
        }
        memcpy(message + sizeof(T2MQMessageHeader), data, data_len);
    }

    // Send message to daemon
    uint32_t total_size = sizeof(T2MQMessageHeader) + data_len;
    if (mq_send(g_mq_state.daemon_mq, message, total_size, 0) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            EVENT_ERROR("Daemon queue is full, message dropped\n");
            printf("Daemon queue is full, message dropped\n");
        }
        else
        {
            EVENT_ERROR("Failed to send message to daemon: %s\n", strerror(errno));
            printf("Failed to send message to daemon: %s\n", strerror(errno));

            // Try to reconnect to daemon queue
            mq_close(g_mq_state.daemon_mq);
            g_mq_state.daemon_mq = -1;
        }
        return T2ERROR_FAILURE;
    }

    EVENT_DEBUG("Successfully sent message to daemon (type: %d, size: %u)\n", msg_type, total_size);
    printf("Successfully sent message to daemon (type: %d, size: %u)\n", msg_type, total_size);

    return T2ERROR_SUCCESS;
}

/**
 * Send event data via message queue with automatic marker update check
 */
int send_event_via_message_queue(const char* data, const char *markerName)
{
    EVENT_DEBUG("%s ++in (Message Queue mode)\n", __FUNCTION__);
    printf("%s ++in (Message Queue mode)\n", __FUNCTION__);

    if (!g_mq_state.initialized)
    {
        EVENT_ERROR("Message queue not initialized\n");
        return -1;
    }

    // 🔥 REPLACE expensive polling with lightweight check
    t2_mq_process_pending_updates();  // Only processes if signal received

    // Validate marker against client event map
    if(!is_valid_event_marker(markerName))
    {
        EVENT_DEBUG("%s markerName %s not found in event list for component %s\n",
                   __FUNCTION__, markerName, componentName);
        printf("%s markerName %s not found in event list for component %s\n",
               __FUNCTION__, markerName, componentName);
        return 0; // Not an error, just filtered out
    }

    // Create event message in format "markerName<#=#>eventValue"
    int eventDataLen = strlen(markerName) + strlen(data) + strlen(MESSAGE_DELIMITER) + 1;
    char* event_message = malloc(eventDataLen);
    if (!event_message)
    {
        EVENT_ERROR("Failed to allocate memory for event message\n");
        return -1;
    }

    snprintf(event_message, eventDataLen, "%s%s%s", markerName, MESSAGE_DELIMITER, data);

    T2ERROR result = t2_mq_send_to_daemon(T2_MQ_MSG_EVENT_DATA, event_message, strlen(event_message));

    free(event_message);

    if (result == T2ERROR_SUCCESS)
    {
        EVENT_DEBUG("MQ event sent successfully: %s=%s\n", markerName, data);
        printf("MQ event sent successfully: %s=%s\n", markerName, data);
        return 0;
    }
    else
    {
        EVENT_ERROR("Failed to send event via message queue\n");
        printf("Failed to send event via message queue\n");
        return -1;
    }
}

/**
 * Request initial marker list from daemon (optional - for immediate startup)
 */
static T2ERROR t2_mq_request_initial_markers(void)
{
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    printf("%s ++in\n", __FUNCTION__);

    // Send subscription/marker request to daemon
    char request_data[256];
    snprintf(request_data, sizeof(request_data), "%s",
             componentName ? componentName : "default");

    T2ERROR result = t2_mq_send_to_daemon(T2_MQ_MSG_SUBSCRIBE, request_data, strlen(request_data));

    if (result == T2ERROR_SUCCESS)
    {
        EVENT_DEBUG("Initial marker request sent\n");
        printf("Initial marker request sent\n");

        // Give daemon a moment to respond, then check for updates
        sleep(1);
        t2_mq_process_pending_updates();
    }
    else
    {
        EVENT_ERROR("Failed to send initial marker request\n");
    }

    return result;
}


static T2ERROR t2_unix_client_connect()
{
    EVENT_ERROR("t2_unix_client_connect ++in\n");
    printf("t2_unix_client_connect ++in\n");

    pthread_mutex_lock(&g_tcp_client_mutex);

    if (g_tcp_client_fd >= 0)
    {
        pthread_mutex_unlock(&g_tcp_client_mutex);
        return T2ERROR_SUCCESS;
    }

    printf("Creating socket\n");
    g_tcp_client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_tcp_client_fd < 0)
    {
        pthread_mutex_unlock(&g_tcp_client_mutex);
        return T2ERROR_FAILURE;
    }

    struct timeval timeout = {.tv_sec = 10, .tv_usec = 0};
    setsockopt(g_tcp_client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(g_tcp_client_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    // Setup server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(T2_TCP_PORT);

    // Convert IP address
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        EVENT_ERROR("Invalid server IP address: %s\n", SERVER_IP);
        close(g_tcp_client_fd);
        g_tcp_client_fd = -1;
        pthread_mutex_unlock(&g_tcp_client_mutex);
        return T2ERROR_FAILURE;
    }

    printf("Connecting to %s:%d...\n", SERVER_IP, T2_TCP_PORT);

    if (connect(g_tcp_client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        EVENT_ERROR("Failed to connect to TCP server %s:%d: %s\n",
                    SERVER_IP, T2_TCP_PORT, strerror(errno));
        close(g_tcp_client_fd);
        g_tcp_client_fd = -1;
        pthread_mutex_unlock(&g_tcp_client_mutex);
        return T2ERROR_FAILURE;
    }

    EVENT_ERROR("TCP client connected to T2 daemon at %s:%d\n", SERVER_IP, T2_TCP_PORT);
    printf("TCP client connected to T2 daemon at %s:%d\n", SERVER_IP, T2_TCP_PORT);

    const char* component_to_send = componentName ? componentName : "default";

    // Create subscription request header
    T2RequestHeader sub_header =
    {
        .request_type = T2_REQ_SUBSCRIBE,           // ← Proper request type
        .data_length = strlen(component_to_send),    // Component name length
        .client_id = (uint32_t)(getpid() ^ time(NULL)), // Unique client ID
        .last_known_version = 0
    };

    // Send header first
    ssize_t sent = send(g_tcp_client_fd, &sub_header, sizeof(sub_header), MSG_NOSIGNAL);
    if (sent != sizeof(sub_header))
    {
        EVENT_ERROR("Failed to send subscription header\n");
        close(g_tcp_client_fd);
        g_tcp_client_fd = -1;
        pthread_mutex_unlock(&g_tcp_client_mutex);
        return T2ERROR_FAILURE;
    }
    EVENT_ERROR("Succesfully sent component name length\n");
    printf("Succesfully sent component name length\n");

    // Send component name
    if (sub_header.data_length > 0)
    {
        sent = send(g_tcp_client_fd, component_to_send, sub_header.data_length, MSG_NOSIGNAL);
        if (sent != (ssize_t)sub_header.data_length)
        {
            EVENT_ERROR("Failed to send component name\n");
            close(g_tcp_client_fd);
            g_tcp_client_fd = -1;
            pthread_mutex_unlock(&g_tcp_client_mutex);
            return T2ERROR_FAILURE;
        }
    }
    EVENT_ERROR("Succesfully sent component name\n");
    printf("Succesfully sent component name\n");

    pthread_mutex_unlock(&g_tcp_client_mutex);
    return T2ERROR_SUCCESS;
}

static T2ERROR t2_unix_client_init()
{
    if (g_tcp_client_connected)
    {
        return T2ERROR_SUCCESS;
    }
    EVENT_ERROR("t2_unix_client_init ++in\n");
    printf("t2_unix_client_init ++in\n");

    // Try connection (failure is OK, will retry in background)
    t2_unix_client_connect();

    g_tcp_client_connected = true;

    EVENT_DEBUG("T2 Unix client initialized\n");
    return T2ERROR_SUCCESS;
}

static void t2_unix_client_uninit()
{
    if (g_tcp_client_connected)
    {
        g_tcp_client_connected = false;

        pthread_mutex_lock(&g_tcp_client_mutex);
        if (g_tcp_client_fd >= 0)
        {
            close(g_tcp_client_fd);
            g_tcp_client_fd = -1;
        }
        pthread_mutex_unlock(&g_tcp_client_mutex);
    }
}

// Function to query event markers from daemon
T2ERROR t2_query_event_markers()
{
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    printf("%s ++in\n", __FUNCTION__);


    pthread_mutex_lock(&g_tcp_client_mutex);

    if (g_tcp_client_fd < 0)
    {
        if (t2_unix_client_connect() != T2ERROR_SUCCESS)
        {
            pthread_mutex_unlock(&g_tcp_client_mutex);
            return T2ERROR_FAILURE;
        }
    }

    const char* component_to_query = componentName ? componentName : "default";

    // Create marker query request header
    T2RequestHeader req_header =
    {
        .request_type = T2_REQ_MARKER_LIST,
        .data_length = strlen(component_to_query),
        .client_id = (uint32_t)(getpid() ^ time(NULL)),
        .last_known_version = 0
    };

    printf("Send request header\n");
    ssize_t sent = send(g_tcp_client_fd, &req_header, sizeof(req_header), MSG_NOSIGNAL);
    if (sent != sizeof(req_header))
    {
        EVENT_ERROR("Failed to send marker query header\n");
        close(g_tcp_client_fd);
        g_tcp_client_fd = -1;
        pthread_mutex_unlock(&g_tcp_client_mutex);
        return T2ERROR_FAILURE;
    }

    printf("Send component name\n");
    sent = send(g_tcp_client_fd, component_to_query, req_header.data_length, MSG_NOSIGNAL);
    if (sent != (ssize_t)req_header.data_length)
    {
        EVENT_ERROR("Failed to send component name for marker query\n");
        close(g_tcp_client_fd);
        g_tcp_client_fd = -1;
        pthread_mutex_unlock(&g_tcp_client_mutex);
        return T2ERROR_FAILURE;
    }

    EVENT_ERROR("Sent marker query for component: %s\n", component_to_query);
    printf("Sent marker query for component: %s\n", component_to_query);


    // Receive response header
    T2ResponseHeader resp_header;
    ssize_t received = recv(g_tcp_client_fd, &resp_header, sizeof(resp_header), MSG_WAITALL);
    if (received != sizeof(resp_header))
    {
        EVENT_ERROR("Failed to receive marker query response header\n");
        printf("Failed to receive marker query response header\n");

        close(g_tcp_client_fd);
        g_tcp_client_fd = -1;
        pthread_mutex_unlock(&g_tcp_client_mutex);
        return T2ERROR_FAILURE;
    }

    if (resp_header.response_status != 0)
    {
        EVENT_ERROR("Daemon returned error status: %u\n", resp_header.response_status);
        printf("Daemon returned error status: %u\n", resp_header.response_status);
        pthread_mutex_unlock(&g_tcp_client_mutex);
        return T2ERROR_FAILURE;
    }

    // Receive marker list data
    char* marker_data = NULL;
    if (resp_header.data_length > 0)
    {
        marker_data = malloc(resp_header.data_length + 1);
        if (!marker_data)
        {
            EVENT_ERROR("Failed to allocate memory for marker data\n");
            printf("Failed to allocate memory for marker data\n");
            pthread_mutex_unlock(&g_tcp_client_mutex);
            return T2ERROR_FAILURE;
        }

        received = recv(g_tcp_client_fd, marker_data, resp_header.data_length, MSG_WAITALL);
        if (received != (ssize_t)resp_header.data_length)
        {
            EVENT_ERROR("Failed to receive complete marker data\n");
            printf("Failed to receive complete marker data\n");

            free(marker_data);
            close(g_tcp_client_fd);
            g_tcp_client_fd = -1;
            pthread_mutex_unlock(&g_tcp_client_mutex);
            return T2ERROR_FAILURE;
        }

        marker_data[resp_header.data_length] = '\0';

        EVENT_ERROR("Received marker data: %s\n", marker_data);
        printf("Received marker data: %s\n", marker_data);

        // Parse and store markers in hash map
        t2_parse_and_store_markers(marker_data);

        free(marker_data);
    }
    else
    {
        EVENT_ERROR("No markers found for component: %s\n", component_to_query);
    }

    pthread_mutex_unlock(&g_tcp_client_mutex);

    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

// Function to parse marker data and store in hash map
void t2_parse_and_store_markers(const char* marker_data)
{
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    printf("%s ++in\n", __FUNCTION__);

    pthread_mutex_lock(&clientMarkerMutex);

    // Initialize hash map if not already done
    if (!eventMarkerMap)
    {
        eventMarkerMap = hash_map_create();
    }

    // Parse comma-separated marker list
    char* data_copy = strdup(marker_data);
    char* token = strtok(data_copy, ",");

    while (token != NULL)
    {
        // Remove leading/trailing whitespace
        while (*token == ' ' || *token == '\t')
        {
            token++;
        }
        char* end = token + strlen(token) - 1;
        while (end > token && (*end == ' ' || *end == '\t' || *end == '\n'))
        {
            *end = '\0';
            end--;
        }

        if (strlen(token) > 0)
        {
            // Store marker in hash map (key = marker name, value = marker name)
            hash_map_put(eventMarkerMap, strdup(token), strdup(token), free);
            EVENT_DEBUG("Added marker to client map: %s\n", token);
            printf("Added marker to client map: %s\n", token);

        }

        token = strtok(NULL, ",");
    }

    free(data_copy);
    pthread_mutex_unlock(&clientMarkerMutex);

    EVENT_DEBUG("%s --out\n", __FUNCTION__);
}
