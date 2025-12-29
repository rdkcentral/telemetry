/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <pthread.h>
#include <unistd.h>

#include "t2collection.h"
#include "t2common.h"
#include "busInterface.h"
#include "rbusInterface.h"
#include "telemetry2_0.h"
#include "t2log_wrapper.h"
#include "profile.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#if defined(PRIVACYMODES_CONTROL)
#include "rdkservices_privacyutils.h"
#endif

#define buffLen 1024
#define maxParamLen 128

#define NUM_PROFILE_ELEMENTS 7

#define RBUS_METHOD_TIMEOUT 10
#define MAX_REPORT_TIMEOUT 50

#define T2_TCP_PORT 12345  // Port for telemetry daemon
#define MAX_TCP_CLIENTS 50
#define TCP_BACKLOG 10
#define MESSAGE_DELIMITER "<#=#>" 

static rbusHandle_t t2bus_handle;

static TelemetryEventCallback eventCallBack;
static T2EventMarkerListCallback getMarkerListCallBack;
static dataModelCallBack dmProcessingCallBack;
static dataModelMsgPckCallBack dmMsgPckProcessingCallBack = NULL;
static dataModelSavedJsonCallBack dmSavedJsonProcessingCallBack;
static dataModelSavedMsgPackCallBack dmSavedMsgPackProcessingCallBack;
static hash_map_t *compTr181ParamMap = NULL;
static profilememCallBack profilememUsedCallBack;
static dataModelReportOnDemandCallBack reportOnDemandCallBack;
static triggerReportOnCondtionCallBack reportOnConditionCallBack;
static xconfPrivacyModesDoNotShareCallBack privacyModesDoNotShareCallBack;
static ReportProfilesDeleteDNDCallBack mprofilesDeleteCallBack;
#if defined(PRIVACYMODES_CONTROL)
static char* privacyModeVal = NULL;
#endif
static uint32_t t2ReadyStatus = T2_STATE_NOT_READY;
static char* reportProfileVal = NULL ;
static char* tmpReportProfileVal = NULL ;
static char* reportProfilemsgPckVal = NULL ;
#ifdef DCMAGENT
static bool dcmEventStatus = false;
#endif
uint32_t t2MemUsage = 0;

static pthread_mutex_t asyncMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t compParamMap = PTHREAD_MUTEX_INITIALIZER;
static rbusMethodAsyncHandle_t onDemandReportCallBackHandler = NULL ;

typedef struct MethodData
{
    rbusMethodAsyncHandle_t asyncHandle;
} MethodData;

typedef enum {
    T2_REQ_SUBSCRIBE = 1,
    T2_REQ_PROFILE_DATA = 2,
    T2_REQ_MARKER_LIST = 3,
    T2_REQ_DAEMON_STATUS = 4,
    T2_MSG_EVENT_DATA = 5
} T2RequestType;

typedef struct {
    uint32_t request_type;    // T2RequestType
    uint32_t data_length;     // Length of data following this header
    uint32_t client_id;       // Unique client identifier
    uint32_t last_known_version; // For versioning/sync purposes
} T2RequestHeader;

typedef struct {
    uint32_t response_status; // 0=success, 1=failure, 2=invalid_request, 3=no_data
    uint32_t data_length;     // Length of response data
    uint32_t sequence_id;     // Matches request sequence
    uint32_t reserved;        // For future use
} T2ResponseHeader;

// Response status codes
typedef enum {
    T2_RESP_SUCCESS = 0,
    T2_RESP_FAILURE = 1,
    T2_RESP_INVALID_REQUEST = 2,
    T2_RESP_NO_DATA = 3
} T2ResponseStatus;

typedef struct {
    int server_fd;
    struct sockaddr_in server_addr;
    bool server_running;
    pthread_t server_thread;
    pthread_mutex_t clients_mutex;
    
    struct {
        int client_fd;
        struct sockaddr_in client_addr;
        bool active;
        bool subscribed_to_profile_updates;
        bool subscribed_to_marker_updates;
        uint32_t client_id;
        char component_name[256];
        time_t connect_time;
    } clients[MAX_TCP_CLIENTS];
} T2TcpServer;

static T2TcpServer g_tcp_server = {0};

T2ERROR T2RbusConsumer(TriggerCondition *triggerCondition);
static void t2_handle_marker_list_request(int client_index, T2RequestHeader* req_header);
static void t2_handle_event_data(int client_index, T2RequestHeader* req_header);

bool isRbusInitialized( )
{

    return t2bus_handle != NULL ? true : false;
}

void logHandler(
    rbusLogLevel level,
    const char* file,
    int line,
    int threadId,
    char* message)
{
    (void) threadId; // To avoid compiler warnings, we cant remove this argument as this is requirment from rbus
    switch (level)
    {
    case RBUS_LOG_FATAL:
        T2Error("%s:%d %s\n", file, line, message);
        break;
    case RBUS_LOG_ERROR:
        T2Error("%s:%d %s\n", file, line, message);
        break;
    case RBUS_LOG_WARN:
        T2Warning("%s:%d %s\n", file, line, message);
        break;
    case RBUS_LOG_INFO:
        T2Info("%s:%d %s\n", file, line, message);
        break;
    case RBUS_LOG_DEBUG:
        T2Debug("%s:%d %s\n", file, line, message);
        break;
    }
    return;
}


#if 1
static void t2_unix_socket_server_uninit()
{
    if (g_tcp_server.server_running) {
        T2Info("Stopping TCP server...\n");
        g_tcp_server.server_running = false;
        
        // Wait for server thread to exit
        pthread_join(g_tcp_server.server_thread, NULL);
        
        // Close all client connections
        pthread_mutex_lock(&g_tcp_server.clients_mutex);
        for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
            if (g_tcp_server.clients[i].active) {
                            char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &g_tcp_server.clients[i].client_addr.sin_addr, 
                          client_ip, INET_ADDRSTRLEN);
                
                T2Info("Closing client connection %d (%s:%d) for component %s\n", 
                       i, client_ip, ntohs(g_tcp_server.clients[i].client_addr.sin_port), g_tcp_server.clients[i].component_name);
                
                close(g_tcp_server.clients[i].client_fd);
                g_tcp_server.clients[i].active = false;
            }
        }
        pthread_mutex_unlock(&g_tcp_server.clients_mutex);
        
        // Cleanup
        close(g_tcp_server.server_fd);
        pthread_mutex_destroy(&g_tcp_server.clients_mutex);
        
        T2Info("TCP server uninitialized successfully\n");
    }
    else
    {
        T2Info("TCP server was not running\n");
    }
}
#endif

// Initialize TCP server
static T2ERROR t2_init_tcp_server()
{
    T2Info("%s ++in\n", __FUNCTION__);
    
    // Create TCP socket
    g_tcp_server.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_tcp_server.server_fd < 0) {
        T2Error("Failed to create TCP socket: %s\n", strerror(errno));
        return T2ERROR_FAILURE;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(g_tcp_server.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        T2Error("Failed to set SO_REUSEADDR: %s\n", strerror(errno));
        close(g_tcp_server.server_fd);
        return T2ERROR_FAILURE;
    }
    
    // Setup server address
    memset(&g_tcp_server.server_addr, 0, sizeof(g_tcp_server.server_addr));
    g_tcp_server.server_addr.sin_family = AF_INET;
    g_tcp_server.server_addr.sin_addr.s_addr = INADDR_ANY;  //0.0.0.0=>To communicate with any client includ ing container
    g_tcp_server.server_addr.sin_port = htons(T2_TCP_PORT);
    
    // Bind socket
    if (bind(g_tcp_server.server_fd, (struct sockaddr*)&g_tcp_server.server_addr, 
             sizeof(g_tcp_server.server_addr)) < 0) {
        T2Error("Failed to bind TCP socket to %s:%d: %s\n", 
                "INADDR_ANY", T2_TCP_PORT, strerror(errno));
        close(g_tcp_server.server_fd);
        return T2ERROR_FAILURE;
    }
    
    // Listen for connections
    if (listen(g_tcp_server.server_fd, TCP_BACKLOG) < 0) {
        T2Error("Failed to listen on TCP socket: %s\n", strerror(errno));
        close(g_tcp_server.server_fd);
        return T2ERROR_FAILURE;
    }
    
    // Initialize mutex
    if (pthread_mutex_init(&g_tcp_server.clients_mutex, NULL) != 0) {
        T2Error("Failed to initialize TCP clients mutex\n");
        close(g_tcp_server.server_fd);
        return T2ERROR_FAILURE;
    }
    
    T2Info("TCP server initialized on %s:%d\n", "INADDR_ANY", T2_TCP_PORT);
    T2Info("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

static void t2_cleanup_tcp_client(int client_index)
{
    T2Info("TODO: Handle Client disconnection for client %d", client_index);
}

// Handle new TCP connection
static void t2_handle_new_tcp_connection(int client_fd, struct sockaddr_in* client_addr)
{
    // Set socket timeouts
    struct timeval timeout = {.tv_sec = 30, .tv_usec = 0};
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    // Read subscription request
    T2RequestHeader sub_header;
    ssize_t received = recv(client_fd, &sub_header, sizeof(sub_header), MSG_WAITALL);
    
    if (received != sizeof(sub_header)) {
        T2Info("Failed to receive subscription header from TCP client\n");
        close(client_fd);
        return;
    }
    
    if (sub_header.request_type != T2_REQ_SUBSCRIBE) {
        T2Info("Invalid request type from TCP client: %u\n", sub_header.request_type);
        close(client_fd);
        return;
    }
    
    // Read component name if present
    char component_name[256] = "default";
    if (sub_header.data_length > 0) {
        ssize_t comp_received = recv(client_fd, component_name, 
                                     min(sub_header.data_length, sizeof(component_name) - 1), 
                                     MSG_WAITALL);
        if (comp_received > 0) {
            component_name[comp_received] = '\0';
        }
    }
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, INET_ADDRSTRLEN);
    
    T2Info("TCP subscription from client_id: %u, component: %s, IP: %s\n", 
           sub_header.client_id, component_name, client_ip);
    
    // Find slot for new subscriber
    pthread_mutex_lock(&g_tcp_server.clients_mutex);
    
    bool slot_found = false;
    for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
        if (!g_tcp_server.clients[i].active) {
            g_tcp_server.clients[i].client_fd = client_fd;
            g_tcp_server.clients[i].client_addr = *client_addr;
            g_tcp_server.clients[i].active = true;
            g_tcp_server.clients[i].client_id = sub_header.client_id;
            g_tcp_server.clients[i].subscribed_to_profile_updates = true;
            g_tcp_server.clients[i].subscribed_to_marker_updates = true;
            g_tcp_server.clients[i].connect_time = time(NULL);
            strncpy(g_tcp_server.clients[i].component_name, component_name, 
                    sizeof(g_tcp_server.clients[i].component_name) - 1);
            
            T2Info("TCP client subscribed in slot %d from %s\n", i, client_ip);
            slot_found = true;
            break;
        }
    }
    
    if (!slot_found) {
        T2Error("No available TCP subscription slots\n");
        close(client_fd);
    }
    
    pthread_mutex_unlock(&g_tcp_server.clients_mutex);
}

static void t2_handle_tcp_client_message(int client_index)
{
    T2Info("TODO: Handle client message for client %s", g_tcp_server.clients[client_index].component_name);

    pthread_mutex_lock(&g_tcp_server.clients_mutex);
    
    if (!g_tcp_server.clients[client_index].active) {
        pthread_mutex_unlock(&g_tcp_server.clients_mutex);
        return;
    }
    
    int client_fd = g_tcp_server.clients[client_index].client_fd;
    char* component_name = g_tcp_server.clients[client_index].component_name;
    
    pthread_mutex_unlock(&g_tcp_server.clients_mutex);
    
    // Read request header
    T2RequestHeader req_header;
    ssize_t received = recv(client_fd, &req_header, sizeof(req_header), MSG_WAITALL);
    
    if (received != sizeof(req_header)) {
        T2Error("Failed to receive request header from client %s\n", component_name);
        t2_cleanup_tcp_client(client_index);
        return;
    }
    
    T2Info("Received request type: %u from client %d (%s)\n", 
           req_header.request_type, client_index, component_name);
    
    switch (req_header.request_type) {
        case T2_REQ_MARKER_LIST:
            t2_handle_marker_list_request(client_index, &req_header);
            break;
            
        case T2_MSG_EVENT_DATA:
            t2_handle_event_data(client_index, &req_header);
            break;

        default:
            T2Error("Unknown request type: %u from client %d\n", 
                    req_header.request_type, client_index);
            break;
    }    
}

// New function to handle marker list requests
static void t2_handle_marker_list_request(int client_index, T2RequestHeader* req_header)
{
    T2Debug("%s ++in for client %d\n", __FUNCTION__, client_index);
    
    pthread_mutex_lock(&g_tcp_server.clients_mutex);
    
    if (!g_tcp_server.clients[client_index].active) {
        pthread_mutex_unlock(&g_tcp_server.clients_mutex);
        return;
    }
    
    int client_fd = g_tcp_server.clients[client_index].client_fd;
    char* component_name = g_tcp_server.clients[client_index].component_name;
    
    pthread_mutex_unlock(&g_tcp_server.clients_mutex);
    
    // Read component name from request data (if any)
    char query_component[256] = {0};
    if (req_header->data_length > 0) {
        ssize_t comp_received = recv(client_fd, query_component, 
                                     min(req_header->data_length, sizeof(query_component) - 1), 
                                     MSG_WAITALL);
        if (comp_received > 0) {
            query_component[comp_received] = '\0';
        }
    } else {
        // Use registered component name
        strncpy(query_component, component_name, sizeof(query_component) - 1);
    }
    
    T2Info("Processing marker list request for component: %s\n", query_component);
    
    // Get marker list using existing callback
    Vector* eventMarkerList = NULL;
    getMarkerListCallBack(query_component, (void**)&eventMarkerList);
    
    // Prepare response data
    char marker_response[2048] = {0}; // Adjust size as needed
    int marker_count = 0;
    
    if (eventMarkerList && Vector_Size(eventMarkerList) > 0) {
        marker_count = Vector_Size(eventMarkerList);
        
        for (int i = 0; i < marker_count; i++) {
            char* marker_name = (char*)Vector_At(eventMarkerList, i);
            if (marker_name) {
                if (i > 0) {
                    strcat(marker_response, ",");
                }
                strncat(marker_response, marker_name, sizeof(marker_response) - strlen(marker_response) - 1);
            }
        }
        
        Vector_Destroy(eventMarkerList, free);
        
        T2Info("Found %d markers for component %s: %s\n", 
               marker_count, query_component, marker_response);
            } else {
                T2Info("No markers found for component: %s\n", query_component);
                strcpy(marker_response, ""); // Empty response
            }
            
            // Send response header
            T2ResponseHeader resp_header = {
                .response_status = 0, // Success
                .data_length = strlen(marker_response),
                .sequence_id = req_header->client_id,
                .reserved = 0
            };
            ssize_t sent = send(client_fd, &resp_header, sizeof(resp_header), MSG_NOSIGNAL);
            if (sent != sizeof(resp_header)) {
                T2Error("Failed to send marker response header to client %d\n", client_index);
                t2_cleanup_tcp_client(client_index);
                return;
            }
            
            // Send response data
            if (resp_header.data_length > 0) {
                sent = send(client_fd, marker_response, resp_header.data_length, MSG_NOSIGNAL);
                if (sent != (ssize_t)resp_header.data_length) {
                    T2Error("Failed to send marker response data to client %d\n", client_index);
                    t2_cleanup_tcp_client(client_index);
                    return;
                }
            }
    T2Info("Successfully sent marker list response to client %d (%s)\n", 
           client_index, component_name);
    
    T2Debug("%s --out\n", __FUNCTION__);
}  

// Function to handle incoming event data
static void t2_handle_event_data(int client_index, T2RequestHeader* req_header)
{
    T2Debug("%s ++in for client %d\n", __FUNCTION__, client_index);
    
    pthread_mutex_lock(&g_tcp_server.clients_mutex);
    
    if (!g_tcp_server.clients[client_index].active) {
        pthread_mutex_unlock(&g_tcp_server.clients_mutex);
        return;
    }
    
    int client_fd = g_tcp_server.clients[client_index].client_fd;
    char* component_name = g_tcp_server.clients[client_index].component_name;
    
    pthread_mutex_unlock(&g_tcp_server.clients_mutex);
    
    // Read event data
    if (req_header->data_length > 0) {
        char* event_data = malloc(req_header->data_length + 1);
        if (!event_data) {
            T2Error("Failed to allocate memory for event data\n");
            return;
        }
        
        ssize_t received = recv(client_fd, event_data, req_header->data_length, MSG_WAITALL);
        if (received == (ssize_t)req_header->data_length) {
            event_data[req_header->data_length] = '\0';
            
            T2Info("Received event from %s: %s\n", component_name, event_data);
            
            // Parse and process event using existing callback
            // Format: "markerName<delimiter>eventValue"
            char* delimiter_pos = strstr(event_data, MESSAGE_DELIMITER);
            if (delimiter_pos) {
                *delimiter_pos = '\0';
                char* marker_name = event_data;
                char* event_value = delimiter_pos + strlen(MESSAGE_DELIMITER);
                
                T2Debug("Processing event: marker=%s, value=%s\n", marker_name, event_value);
                
                // Call existing event callback
                if (eventCallBack) {
                    eventCallBack(strdup(marker_name), strdup(event_value));
                }
            }
        } else {
            T2Error("Failed to receive complete event data from client %d\n", client_index);
        }
        
        free(event_data);
    }
    
    T2Debug("%s --out\n", __FUNCTION__);
}

// TCP server thread
static void* t2_tcp_server_thread(void* arg)
{
    (void)arg;
    
    T2Info("TCP server thread started on %s:%d\n", "INADDR_ANY", T2_TCP_PORT);
    
    struct pollfd poll_fds[MAX_TCP_CLIENTS + 1];
    int client_indices[MAX_TCP_CLIENTS + 1];
    int poll_count = 0;

    // Setup polling for server socket
    poll_fds[0].fd = g_tcp_server.server_fd;
    poll_fds[0].events = POLLIN;
    client_indices[0] = -1;
    poll_count = 1;

    while (g_tcp_server.server_running) {
        
        // Add existing clients to poll list - Not required for every loop. TODO: fix that.
        pthread_mutex_lock(&g_tcp_server.clients_mutex);
        for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
            if (g_tcp_server.clients[i].active) {
                poll_fds[poll_count].fd = g_tcp_server.clients[i].client_fd;
                poll_fds[poll_count].events = POLLIN;
                client_indices[poll_count] = i;
                poll_count++;
            }
        }
        pthread_mutex_unlock(&g_tcp_server.clients_mutex);
        
        // Poll for activity
        int poll_result = poll(poll_fds, poll_count, 1000); // 1 second timeout
        if (poll_result <= 0) {
            continue;
        }
        
        // Handle new connections
        if (poll_fds[0].revents & POLLIN) {
            T2Info("New TCP client connection:\n");
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(g_tcp_server.server_fd, 
                                   (struct sockaddr*)&client_addr, &client_len);
            
            if (client_fd >= 0) {
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                
                T2Info("New TCP client connection from %s:%d (fd: %d)\n", 
                       client_ip, ntohs(client_addr.sin_port), client_fd);
                
                t2_handle_new_tcp_connection(client_fd, &client_addr);
            }
        }
        
        // Handle client messages
        for (int i = 1; i < poll_count; i++) {
            if (poll_fds[i].revents & POLLIN) {
                int client_index = client_indices[i];
                if (client_index >= 0) {
                    t2_handle_tcp_client_message(client_index);
                }
            }
            
            // Handle disconnections
            if (poll_fds[i].revents & (POLLHUP | POLLERR)) {
                int client_index = client_indices[i];
                if (client_index >= 0) {
                    t2_cleanup_tcp_client(client_index);
                }
            }
        }
    }
    
    T2Info("TCP server thread exiting\n");
    return NULL;
}

static T2ERROR rBusInterface_Init( )
{
    T2Debug("%s ++in\n", __FUNCTION__);

    int ret = RBUS_ERROR_SUCCESS;

    //rbus_setLogLevel(RBUS_LOG_DEBUG);
    rbus_registerLogHandler(logHandler);

    ret = rbus_open(&t2bus_handle, COMPONENT_NAME);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        T2Error("%s:%d, init failed with error code %d \n", __func__, __LINE__, ret);
        return T2ERROR_FAILURE;
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

static void rBusInterface_Uninit( )
{
    rbus_close(t2bus_handle);
}

T2ERROR getRbusParameterVal(const char* paramName, char **paramValue)
{
    T2Debug("%s ++in \n", __FUNCTION__);

    rbusError_t ret = RBUS_ERROR_SUCCESS;
    rbusValue_t paramValue_t;
    rbusValueType_t rbusValueType ;
    char *stringValue = NULL;
#if 0
    rbusSetOptions_t opts;
    opts.commit = true;
#endif

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        return T2ERROR_FAILURE;
    }

    ret = rbus_get(t2bus_handle, paramName, &paramValue_t);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        T2Error("Unable to get %s\n", paramName);
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

#if defined(ENABLE_RDKV_SUPPORT)
    // Workaround as video platforms doesn't have a TR param which gives Firmware name
    // Existing dashboards doesn't like version with file name exentsion
    // Workaround stays until initiative for unified new TR param for version name gets implemented across board
    if(0 == strncmp(paramName, "Device.DeviceInfo.X_COMCAST-COM_FirmwareFilename", maxParamLen ) || 0 == strncmp(paramName, "Device.DeviceInfo.X_RDKCENTRAL-COM_FirmwareFilename", maxParamLen ))
    {
        char* temp = NULL ;
        temp = strstr(stringValue, "-signed.bin");
        if(!temp)
        {
            temp = strstr(stringValue, ".bin");
        }
        if(temp)
        {
            *temp = '\0';
        }
    }
#endif

    T2Debug("%s = %s\n", paramName, stringValue);
    *paramValue = stringValue;
    rbusValue_Release(paramValue_t);
    T2Debug("%s --out \n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

Vector* getRbusProfileParamValues(Vector *paramList, int execcount)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    unsigned int i = 0;
    Vector *profileValueList = NULL;
    Vector_Create(&profileValueList);

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        Vector_Destroy(profileValueList, free);
        return NULL ;
    }
    char** paramNames = (char **) malloc(paramList->count * sizeof(char*));

    T2Debug("TR-181 Param count : %lu\n", (unsigned long)paramList->count);
    for( ; i < paramList->count; i++ )   // Loop through paramlist from profile
    {

        tr181ValStruct_t **paramValues = NULL;
        rbusProperty_t rbusPropertyValues = NULL;
        int paramValCount = 0;
        int iterate = 0;
        Param *parmTemp = Vector_At(paramList, i);
        profileValues* profVals = (profileValues *) malloc(sizeof(profileValues));
        if(parmTemp->skipFreq > 0 && (execcount % (parmTemp->skipFreq + 1) != 0))
        {
            T2Debug("Skipping parameter : %s as per skipFreq : %d\n", parmTemp->name, parmTemp->skipFreq);
            profVals->paramValues = NULL;
            Vector_PushBack(profileValueList, profVals);
            continue;
        }

        char *param = (char*)parmTemp->alias ;
        if(param != NULL)
        {
            paramNames[0] = strdup(param);
        }
        else
        {
            paramNames[0] = NULL;
        }
        T2Debug("Parameter Name is %s \n",  paramNames[0]);
        if(paramNames[0] != NULL)
        {
            T2Debug("Calling rbus_getExt for %s \n", paramNames[0]);
            if(RBUS_ERROR_SUCCESS != rbus_getExt(t2bus_handle, 1, (const char**)paramNames, &paramValCount, &rbusPropertyValues))
            {
                T2Error("Failed to retrieve param : %s\n", paramNames[0]);
                paramValCount = 0 ;
            }
            else
            {
                if(rbusPropertyValues == NULL || paramValCount == 0)
                {
                    T2Info("ParameterName : %s Retrieved value count : %d\n", paramNames[0], paramValCount);
                }
            }
        }
        else
        {
            paramValCount = 0;
        }
        profVals->paramValueCount = paramValCount;

        T2Debug("Received %d parameters for %s fetch \n", paramValCount, paramNames[0]);

        // Populate bus independent parameter value array
        if(paramValCount == 0)
        {
            paramValues = (tr181ValStruct_t**) malloc(sizeof(tr181ValStruct_t*));
            if(paramValues != NULL)
            {
                paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
                paramValues[0]->parameterName = strdup(param);
                paramValues[0]->parameterValue = strdup("NULL");
                // If parameter doesn't exist in device we do populate with entry as NULL.
                // So count of populated data list has 1 entry and is not 0
                profVals->paramValueCount = 1;
            }
            if(rbusPropertyValues != NULL)
            {
                rbusProperty_Release(rbusPropertyValues);
            }
        }
        else
        {
            paramValues = (tr181ValStruct_t**) malloc(paramValCount * sizeof(tr181ValStruct_t*));
            if(paramValues != NULL)
            {
                rbusProperty_t nextProperty = rbusPropertyValues;
                for( iterate = 0; iterate < paramValCount; ++iterate )   // Loop through values obtained from query for individual param in list
                {
                    if(nextProperty)
                    {
                        char* stringValue = NULL;
                        rbusValue_t value = rbusProperty_GetValue(nextProperty);
                        paramValues[iterate] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
                        if(paramValues[iterate])
                        {
                            stringValue = (char*)rbusProperty_GetName(nextProperty);
                            paramValues[iterate]->parameterName = strdup(stringValue);

                            rbusValueType_t rbusValueType = rbusValue_GetType(value);
                            if(rbusValueType == RBUS_BOOLEAN)
                            {
                                if(rbusValue_GetBoolean(value))
                                {
                                    paramValues[iterate]->parameterValue = strdup("true");
                                }
                                else
                                {
                                    paramValues[iterate]->parameterValue = strdup("false");
                                }
                            }
                            else
                            {
                                paramValues[iterate]->parameterValue = rbusValue_ToString(value, NULL, 0);
                            }

#if defined(ENABLE_RDKV_SUPPORT)
                            // Workaround as video platforms doesn't have a TR param which gives Firmware name
                            // Existing dashboards doesn't like version with file name exentsion
                            // Workaround stays until initiative for unified new TR param for version name gets implemented across board
                            if(0 == strncmp(stringValue, "Device.DeviceInfo.X_RDKCENTRAL-COM_FirmwareFilename", maxParamLen ) || 0 == strncmp(stringValue, "Device.DeviceInfo.X_COMCAST-COM_FirmwareFilename", maxParamLen ))
                            {
                                char* temp = NULL;
                                temp = strstr(paramValues[iterate]->parameterValue, "-signed.bin");
                                if(!temp)
                                {
                                    temp = strstr(paramValues[iterate]->parameterValue, ".bin");
                                }
                                if(temp)
                                {
                                    *temp = '\0';
                                }
                            }
#endif
                        }
                    }
                    nextProperty = rbusProperty_GetNext(nextProperty);
                }
                rbusProperty_Release(rbusPropertyValues);
            }
        }
        free(paramNames[0]);

        profVals->paramValues = paramValues;
        // End of populating bus independent parameter value array
        Vector_PushBack(profileValueList, profVals);
    } // End of looping through tr181 parameter list from profile
    if(paramNames)
    {
        free(paramNames);
    }

    T2Debug("%s --Out\n", __FUNCTION__);
    return profileValueList;
}

rbusError_t eventSubHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char* eventName, rbusFilter_t* filter, int interval, bool* autoPublish)
{
    (void) handle;
    (void) filter;
    (void) interval;
    (void) autoPublish;
    T2Debug("%s ++in\n", __FUNCTION__);
    T2Info("eventSubHandler called:\n \taction=%s \teventName=%s\n", action == RBUS_EVENT_ACTION_SUBSCRIBE ? "subscribe" : "unsubscribe", eventName);
    T2Debug("%s --out\n", __FUNCTION__);
    return RBUS_ERROR_SUCCESS;
}

/**
 * Data set handler for event receiving datamodel
 * Data being set will be an rbusProperty object with -
 *     eventName as property name
 *     eventValue as property value
 * This eliminates avoids the need for sending data with fixed delimiters
 * comapred to CCSP way of eventing .
 */
rbusError_t t2PropertyDataSetHandler(rbusHandle_t handle, rbusProperty_t prop, rbusSetHandlerOptions_t* opts)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    (void) handle;
    (void) opts;

    char const* paramName = rbusProperty_GetName(prop);
    if((strncmp(paramName, T2_EVENT_PARAM, maxParamLen) != 0) && (strncmp(paramName, T2_REPORT_PROFILE_PARAM, maxParamLen) != 0)
            && (strncmp(paramName, T2_REPORT_PROFILE_PARAM_MSG_PCK, maxParamLen) != 0)
            && (strncmp(paramName, T2_TEMP_REPORT_PROFILE_PARAM, maxParamLen) != 0) && (strncmp(paramName, T2_TOTAL_MEM_USAGE, maxParamLen) != 0) && (strncmp(paramName, PRIVACYMODES_RFC, maxParamLen) != 0))
    {
        T2Debug("Unexpected parameter = %s \n", paramName);
        T2Debug("%s --out\n", __FUNCTION__);
        return RBUS_ERROR_ELEMENT_DOES_NOT_EXIST;
    }

    T2Debug("Parameter name is %s \n", paramName);
    rbusValueType_t type_t;
    rbusValue_t paramValue_t = rbusProperty_GetValue(prop);
    if(paramValue_t)
    {
        type_t = rbusValue_GetType(paramValue_t);
    }
    else
    {
        return RBUS_ERROR_INVALID_INPUT;
        T2Debug("%s --out\n", __FUNCTION__);
    }
    if(strncmp(paramName, T2_EVENT_PARAM, maxParamLen) == 0)
    {
        if(type_t == RBUS_PROPERTY)
        {
            T2Debug("Received property type as value \n");
            rbusProperty_t objProperty = rbusValue_GetProperty(paramValue_t);
            char *eventName = (char*)rbusProperty_GetName(objProperty);
            if(eventName)
            {
                T2Debug("Event name is %s \n", eventName);
                rbusValue_t value = rbusProperty_GetValue(objProperty);
                type_t = rbusValue_GetType(value);
                if(type_t == RBUS_STRING)
                {
                    char* eventValue = rbusValue_ToString(value, NULL, 0);
                    if(eventValue)
                    {
                        T2Debug("Event value is %s \n", eventValue);
                        eventCallBack((char*) strdup(eventName), (char*) strdup(eventValue) );
                        free(eventValue);
                    }
                }
                else
                {
                    T2Debug("Unexpected value type for property %s \n", eventName);
                }
            }
        }
    }
    else if(strncmp(paramName, T2_REPORT_PROFILE_PARAM, maxParamLen) == 0)
    {
        T2Debug("Inside datamodel handler \n");
        if(type_t == RBUS_STRING)
        {
            char* data = rbusValue_ToString(paramValue_t, NULL, 0);
            if(data)
            {
                T2Debug("Call datamodel function  with data %s \n", data);
                if(T2ERROR_SUCCESS != dmProcessingCallBack(data, T2_RP))
                {
                    free(data);
                    return RBUS_ERROR_INVALID_INPUT;
                }

                if (reportProfileVal)
                {
                    free(reportProfileVal);
                    reportProfileVal = NULL;
                }
                if(reportProfilemsgPckVal)
                {
                    free(reportProfilemsgPckVal);
                    reportProfilemsgPckVal = NULL;
                }
                reportProfileVal = strdup(data);
                free(data);
            }
        }
        else
        {
            T2Debug("Unexpected value type for property %s \n", paramName);
        }

    }
    else if(strncmp(paramName, T2_REPORT_PROFILE_PARAM_MSG_PCK, maxParamLen) == 0)
    {
        T2Debug("Inside datamodel handler for message pack \n");
        if(dmMsgPckProcessingCallBack == NULL)
        {
            T2Debug("%s --out\n", __FUNCTION__);
            return RBUS_ERROR_ELEMENT_DOES_NOT_EXIST;
        }

        if(type_t == RBUS_STRING)
        {
            char* data = rbusValue_ToString(paramValue_t, NULL, 0);
            guchar *webConfigString = NULL;
            gsize decodedDataLen = 0;
            if(data)
            {
                T2Debug("Call datamodel function  with data %s \n", data);
                webConfigString = g_base64_decode(data, &decodedDataLen);
                if(NULL == webConfigString ||  0 == decodedDataLen )
                {
                    T2Error("Invalid base64 input string. Ignore processing input configuration.\n");
                    if(webConfigString != NULL)
                    {
                        g_free(webConfigString);
                        webConfigString = NULL;
                    }
                    free(data);
                    return RBUS_ERROR_INVALID_INPUT;
                }

                if(T2ERROR_SUCCESS != dmMsgPckProcessingCallBack((char *)webConfigString, decodedDataLen))
                {
                    free(data);
                    T2Info("RBUS_ERROR_INVALID_INPUT freeing the webconfigString\n");
                    if(webConfigString != NULL)
                    {
                        g_free(webConfigString);
                        webConfigString = NULL;
                    }
                    return RBUS_ERROR_INVALID_INPUT;
                }

                if(reportProfilemsgPckVal)
                {
                    free(reportProfilemsgPckVal);
                    reportProfilemsgPckVal = NULL;
                }
                if(reportProfileVal)
                {
                    free(reportProfileVal);
                    reportProfileVal = NULL;
                }
                reportProfilemsgPckVal = strdup(data);
                free(data);
            }
        }
        else
        {
            T2Debug("Unexpected value type for property %s \n", paramName);
        }

    }
    else if(strncmp(paramName, T2_TEMP_REPORT_PROFILE_PARAM, maxParamLen) == 0)
    {
        T2Debug("Inside datamodel handler for Short-lived profile \n");
        if(type_t == RBUS_STRING)
        {
            char* data = rbusValue_ToString(paramValue_t, NULL, 0);
            if(data)
            {
                T2Debug("Call datamodel function  with data %s \n", data);
                if(T2ERROR_SUCCESS != dmProcessingCallBack(data, T2_TEMP_RP))
                {
                    free(data);
                    return RBUS_ERROR_INVALID_INPUT;
                }
                if (tmpReportProfileVal)
                {
                    free(tmpReportProfileVal);
                    tmpReportProfileVal = NULL;
                }
                tmpReportProfileVal = strdup(data);
                free(data);
            }
        }
        else
        {
            T2Debug("Unexpected value type for property %s \n", paramName);
        }
    }
#if defined(PRIVACYMODES_CONTROL)
    else if(strncmp(paramName, PRIVACYMODES_RFC, maxParamLen) == 0)
    {
        if(type_t == RBUS_STRING)
        {
            T2Debug("Inside datamodel handler for privacymodes profile \n");
            char* data = rbusValue_ToString(paramValue_t, NULL, 0);
            if(privacyModeVal != NULL)
            {
                free(privacyModeVal);
                privacyModeVal = NULL;
            }
            if((strcmp(data, "SHARE") != 0) && (strcmp(data, "DO_NOT_SHARE") != 0))
            {
                T2Info("Unexpected privacy Mode value %s\n", data);
                free(data);
                return RBUS_ERROR_INVALID_INPUT;
            }
            privacyModeVal = strdup(data);
            free(data);
            T2Debug("PrivacyMode data is %s\n", privacyModeVal);
            if(T2ERROR_SUCCESS != setPrivacyMode(privacyModeVal))
            {
                return RBUS_ERROR_INVALID_INPUT;
            }
            if(strcmp(privacyModeVal, "DO_NOT_SHARE") == 0)
            {
                if(mprofilesDeleteCallBack()  != T2ERROR_SUCCESS)
                {
                    return RBUS_ERROR_INVALID_INPUT;
                }
            }
            if(privacyModesDoNotShareCallBack() != T2ERROR_SUCCESS)
            {
                return RBUS_ERROR_INVALID_INPUT;
            }
        }
        else
        {
            T2Debug("Unexpected value type for property %s \n", paramName);
        }
    }
#endif
    T2Debug("%s --out\n", __FUNCTION__);
    return RBUS_ERROR_SUCCESS;
}

/**
 * Common data get handler for all parameters owned by t2.0
 * Implements :
 * 1] Telemetry.ReportProfiles.<Component Name>.EventMarkerList
 *    Returns list of events associated a component that needs to be notified to
 *    t2.0 in the form of single row multi instance rbus object
 */
rbusError_t t2PropertyDataGetHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t* opts)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    (void) handle;
    (void) opts;
    char const* propertyName;
    char* componentName = NULL;
    Vector* eventMarkerListForComponent = NULL;
    int length = 0;

    propertyName = strdup(rbusProperty_GetName(property));
    if(propertyName)
    {
        T2Debug("Property Name is %s \n", propertyName);
    }
    else
    {
        T2Info("Unable to handle get request for property \n");
        T2Debug("%s --out\n", __FUNCTION__);
        return RBUS_ERROR_INVALID_INPUT;
    }

    if(strncmp(propertyName, T2_REPORT_PROFILE_PARAM, maxParamLen) == 0)
    {
        rbusValue_t value;
        rbusValue_Init(&value);
        if(reportProfileVal)
        {
            rbusValue_SetString(value, reportProfileVal);
        }
        else if(!reportProfilemsgPckVal)
        {
            T2Info("Check the persistant folder for Active Profiles\n");
            char* temp = NULL;
            (*dmSavedJsonProcessingCallBack)(&temp);
            if (temp != NULL)
            {
                T2Info("Profiles from persistant folder %s \n", temp);
                rbusValue_SetString(value, temp);
                free(temp);
            }
            else
            {
                rbusValue_SetString(value, "");
            }
        }
        else
        {
            rbusValue_SetString(value, "");
        }
        rbusProperty_SetValue(property, value);
        rbusValue_Release(value);
    }
    else if(strncmp(propertyName, T2_REPORT_PROFILE_PARAM_MSG_PCK, maxParamLen) == 0)
    {
        rbusValue_t value;
        rbusValue_Init(&value);
        if(reportProfilemsgPckVal)
        {
            rbusValue_SetString(value, reportProfilemsgPckVal);
        }
        else if (!reportProfileVal)
        {
            char* temp = NULL;
            int size;
            char* text;
            size = (*dmSavedMsgPackProcessingCallBack)(&temp);
            if (temp != NULL && size > 0)
            {
                text = g_base64_encode ((const guchar *) temp, (gsize) size);
                T2Info("Profiles from persistant folder profile.msgpack \n");
                rbusValue_SetString(value, text);
                free(text);
                free(temp);
            }
            else
            {
                rbusValue_SetString(value, "");
            }
        }
        else
        {
            rbusValue_SetString(value, "");
        }
        rbusProperty_SetValue(property, value);
        rbusValue_Release(value);

    }
    else if(strncmp(propertyName, T2_TEMP_REPORT_PROFILE_PARAM, maxParamLen) == 0)
    {
        rbusValue_t value;
        rbusValue_Init(&value);
        if(tmpReportProfileVal)
        {
            rbusValue_SetString(value, tmpReportProfileVal);
        }
        else
        {
            rbusValue_SetString(value, "");
        }
        rbusProperty_SetValue(property, value);
        rbusValue_Release(value);
    }
    else if(strncmp(propertyName, T2_OPERATIONAL_STATUS, maxParamLen) == 0)
    {
        rbusValue_t value;
        rbusValue_Init(&value);

        rbusValue_SetUInt32(value, t2ReadyStatus);
        rbusProperty_SetValue(property, value);
        rbusValue_Release(value);
    }
    else if(strncmp(propertyName, T2_TOTAL_MEM_USAGE, maxParamLen) == 0)
    {
        rbusValue_t value;
        rbusValue_Init(&value);
        profilememUsedCallBack(&t2MemUsage);
        rbusValue_SetUInt32(value, t2MemUsage);
        rbusProperty_SetValue(property, value);
        rbusValue_Release(value);
    }
#if defined(PRIVACYMODES_CONTROL)
    else if(strncmp(propertyName, PRIVACYMODES_RFC, maxParamLen) == 0)
    {
        rbusValue_t value;
        rbusValue_Init(&value);
        if(privacyModeVal != NULL)
        {
            rbusValue_SetString(value, privacyModeVal);
        }
        else
        {
            char *data = NULL;
            getPrivacyMode(&data);
            if(data != NULL)
            {
                T2Debug("Privacy mode fetched  from the persistent folder is %s\n", data);
                rbusValue_SetString(value, data);
            }
        }
        rbusProperty_SetValue(property, value);
        rbusValue_Release(value);
    }
#endif
    else
    {
        // START : Extract component name requesting for event marker list
        pthread_mutex_lock(&compParamMap);
        if(compTr181ParamMap != NULL)
        {
            componentName = (char*) hash_map_get(compTr181ParamMap, propertyName);
        }
        pthread_mutex_unlock(&compParamMap); // This needs rework

        if(componentName)
        {
            T2Debug("Component name = %s \n", componentName);
        }
        else
        {
            T2Error("Component name is empty \n");
            free((char*)propertyName);
            propertyName = NULL;  //CID 158138: Resource leak
            return RBUS_ERROR_DESTINATION_RESPONSE_FAILURE;
        }

        // END : Extract component name requesting for event marker list

        rbusValue_t value;
        rbusObject_t object;
        rbusProperty_t propertyList = NULL;

        getMarkerListCallBack(componentName, (void**)&eventMarkerListForComponent);
        length = Vector_Size(eventMarkerListForComponent);

        // START : create LL of rbusProperty_t object

        rbusValue_t fixedVal;
        rbusValue_Init(&fixedVal);
        rbusValue_SetString(fixedVal, "eventName");

        rbusProperty_t prevProperty = NULL;
        int i = 0;
        if(length == 0)    // rbus doesn't like NULL objects and seems to crash. Setting empty values instead
        {
            rbusProperty_Init(&propertyList, "", fixedVal);
        }
        else
        {
            for( i = 0; i < length; ++i )
            {
                char* markerName = (char *) Vector_At(eventMarkerListForComponent, i);
                if(markerName)
                {
                    if(i == 0)
                    {
                        rbusProperty_Init(&propertyList, markerName, fixedVal);
                        prevProperty = propertyList;
                    }
                    else
                    {
                        rbusProperty_t currentProperty = NULL;
                        rbusProperty_Init(&currentProperty, markerName, fixedVal);
                        if(prevProperty)
                        {
                            rbusProperty_SetNext(prevProperty, currentProperty);
                        }
                        prevProperty = currentProperty;
                        rbusProperty_Release(currentProperty);
                    }
                    T2Debug("%d Updated rbusProperty with name = %s \n", i, markerName);
                }
            }
            Vector_Destroy(eventMarkerListForComponent, free);
        }
        // END : create LL of rbusProperty_t object

        rbusObject_Init(&object, "eventList");
        rbusObject_SetProperties(object, propertyList);
        rbusValue_Init(&value);
        rbusValue_SetObject(value, object);
        rbusProperty_SetValue(property, value);

        rbusValue_Release(fixedVal);
        rbusValue_Release(value);
        rbusProperty_Release(propertyList);
        rbusObject_Release(object);
    }

    if(propertyName)
    {
        free((char*)propertyName);
        propertyName = NULL;
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return RBUS_ERROR_SUCCESS;
}


void publishReportUploadStatus(char* status)
{

    rbusObject_t outParams;
    rbusValue_t value;
    rbusError_t err;

    T2Debug("++in %s \n", __FUNCTION__);
    if(!status)
    {
        T2Error("%s received NULL argument.\n", __FUNCTION__);
        T2Debug("--OUT %s\n", __FUNCTION__);
        return ;
    }

    if(!onDemandReportCallBackHandler)
    {
        T2Error("%s onDemandReportCallBackHandler is NULL.\n", __FUNCTION__);
        T2Debug("--OUT %s\n", __FUNCTION__);
        return ;
    }


    rbusObject_Init(&outParams, NULL);
    rbusValue_Init(&value);
    rbusValue_SetString(value, status);
    rbusObject_SetValue(outParams, "UPLOAD_STATUS", value);
    rbusValue_Release(value);  // FIX: Release value immediately after SetValue to prevent double-free

    T2Info("Sending response as %s from %s \n", status,  __FUNCTION__ );
    err = rbusMethod_SendAsyncResponse(onDemandReportCallBackHandler, RBUS_ERROR_SUCCESS, outParams);
    if(err != RBUS_ERROR_SUCCESS)
    {
        T2Error("%s rbusMethod_SendAsyncResponse failed err:%d\n", __FUNCTION__, err);
    }
    else
    {
        T2Info("%s rbusMethod_SendAsyncResponse sent successfully \n", __FUNCTION__);
    }
    onDemandReportCallBackHandler = NULL; // just a safety cleanup
    rbusObject_Release(outParams);  // FIX: Only release outParams, value is already released above
    pthread_mutex_unlock(&asyncMutex);  // FIX: Unlock mutex after cleanup to prevent race conditions

    T2Debug("--OUT %s\n", __FUNCTION__);

}

static rbusError_t dcmOnDemandMethodHandler(rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusObject_t outParams,
        rbusMethodAsyncHandle_t asyncCallBackHandle)
{
    (void) handle;
    (void) inParams;
    (void) outParams;

    T2Debug("++IN %s\n", __FUNCTION__);

    /* Trigger a report generation asynchronously */
    if(!strncmp(methodName, T2_ON_DEMAND_REPORT, maxParamLen))
    {
        if(!pthread_mutex_trylock(&asyncMutex))
        {
            T2Info("Lock is acquired for asyncMutex\n");
            pthread_t rpOnDemandTh;
            pthread_attr_t rpOnDemandAttr;
            pthread_attr_init(&rpOnDemandAttr);
            pthread_attr_setdetachstate(&rpOnDemandAttr, PTHREAD_CREATE_DETACHED);
            // Calls within reportOnDemandCallBack are thread safe, thread synchronization is not required
            if(pthread_create(&rpOnDemandTh, &rpOnDemandAttr, reportOnDemandCallBack, ON_DEMAND_ACTION_UPLOAD) != 0)
            {
                T2Error("Failed to create thread for report on demand.\n");
            }
            pthread_attr_destroy(&rpOnDemandAttr);

            /* If a callback handler is included use it to report the status of report sending status */
            if(NULL != asyncCallBackHandle)
            {
                onDemandReportCallBackHandler = asyncCallBackHandle ;
            }
        }
        else
        {
            T2Error("Failed to lock as previous upload request is already in progress\n");
            if(NULL != asyncCallBackHandle)
            {
                rbusObject_t outputParams;
                rbusValue_t value;
                rbusError_t err;

                rbusObject_Init(&outputParams, NULL);
                rbusValue_Init(&value);
                rbusValue_SetString(value, "PREVIOUS_UPLOAD_IN_PROGRESS_REQUEST_FAILED");
                rbusObject_SetValue(outputParams, "UPLOAD_STATUS", value);

                T2Info("Sending response as PREVIOUS_UPLOAD_IN_PROGRESS_REQUEST_FAILED for %s from %s \n", methodName, __FUNCTION__);
                err = rbusMethod_SendAsyncResponse(asyncCallBackHandle, RBUS_ERROR_SUCCESS, outputParams);
                if(err != RBUS_ERROR_SUCCESS)
                {
                    T2Error("%s rbusMethod_SendAsyncResponse failed err:%d\n", __FUNCTION__, err);
                }
                else
                {
                    T2Info("%s rbusMethod_SendAsyncResponse sent successfully \n", __FUNCTION__);
                }

                rbusValue_Release(value);
                rbusObject_Release(outputParams);
            }
        }
    }
    else if(!strncmp(methodName, T2_ABORT_ON_DEMAND_REPORT, maxParamLen))
    {

        T2Info("%s Aborting previous ondemand report upload %s \n", __FUNCTION__, methodName);

        pthread_t rpOnDemandTh;
        pthread_attr_t rpOnDemandAttr;
        pthread_attr_init(&rpOnDemandAttr);
        pthread_attr_setdetachstate(&rpOnDemandAttr, PTHREAD_CREATE_DETACHED);
        // Calls within reportOnDemandCallBack are thread safe, thread synchronization is not required
        if(pthread_create(&rpOnDemandTh, &rpOnDemandAttr, reportOnDemandCallBack, ON_DEMAND_ACTION_ABORT) != 0)
        {
            T2Error("Failed to create thread for report on demand.\n");
        }

        pthread_attr_destroy(&rpOnDemandAttr);
        if(NULL != asyncCallBackHandle)
        {
            rbusObject_t outputParams;
            rbusValue_t value;
            rbusError_t err;

            rbusObject_Init(&outputParams, NULL);
            rbusValue_Init(&value);
            rbusValue_SetString(value, "SUCCESS");
            rbusObject_SetValue(outputParams, "ABORT_STATUS", value);

            T2Info("Sending response as SUCCESS for %s from %s \n", methodName, __FUNCTION__);
            err = rbusMethod_SendAsyncResponse(asyncCallBackHandle, RBUS_ERROR_SUCCESS, outputParams);
            if(err != RBUS_ERROR_SUCCESS)
            {
                T2Error("%s rbusMethod_SendAsyncResponse failed err:%d\n", __FUNCTION__, err);
            }
            else
            {
                T2Info("%s rbusMethod_SendAsyncResponse sent successfully \n", __FUNCTION__);
            }

            rbusValue_Release(value);
            rbusObject_Release(outputParams);
        }

    }
    else
    {
        T2Info("%s Undefined method %s \n", __FUNCTION__, methodName);
    }


    T2Debug("--OUT %s\n", __FUNCTION__);
    return RBUS_ERROR_ASYNC_RESPONSE ;
}

#ifdef DCMAGENT
static void rbusReloadConf(rbusHandle_t handle,
                           rbusEvent_t const* event,
                           rbusEventSubscription_t* subscription)
{
    (void) handle; //To avoid compiler Warning
    T2Info("%s ++in\n", __FUNCTION__);
    if(event == NULL)
    {
        T2Error("Rbus Event handle is null\n");
        return;
    }

    if(subscription == NULL)
    {
        T2Error("Rbus Event subscription is null\n");
        return;
    }

    T2Info("Recieved eventName: %s, Event type: %d, Event Name: %s\n", subscription->eventName, event->type, event->name);

    dcmEventStatus = true;
    T2Info("%s --out\n", __FUNCTION__);
}

int getRbusDCMEventStatus()
{
    return dcmEventStatus;
}

T2ERROR registerRbusDCMEventListener()
{
    T2Debug("%s ++in\n", __FUNCTION__);

    T2ERROR status = T2ERROR_SUCCESS;
    rbusError_t ret = RBUS_ERROR_SUCCESS;

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        return T2ERROR_FAILURE;
    }

    /**
     * Register data elements with rbus for EVENTS and Profile Updates.
     */
    rbusDataElement_t dataElements[2] =
    {
        {T2_EVENT_DCM_SETCONF, RBUS_ELEMENT_TYPE_EVENT, {NULL, NULL, NULL, NULL, (rbusEventSubHandler_t)eventSubHandler, NULL}},
        {T2_EVENT_DCM_PROCCONF, RBUS_ELEMENT_TYPE_EVENT, {NULL, NULL, NULL, NULL, (rbusEventSubHandler_t)eventSubHandler, NULL}}
    };

    ret = rbus_regDataElements(t2bus_handle, 2, dataElements);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        T2Error("Failed to register DCM events with rbus. Error code : %d\n", ret);
        status = T2ERROR_FAILURE ;
    }

    T2Debug("Subscribing to %s\n", T2_DCM_RELOAD_EVENT);
    /* Subscribe for reload config event */
    ret = rbusEvent_Subscribe(t2bus_handle,
                              T2_DCM_RELOAD_EVENT,
                              rbusReloadConf,
                              NULL,
                              0);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        T2Error("Failed to subscribe DCM reload event with rbus. Error code : %d\n", ret);
        status = T2ERROR_FAILURE ;
    }

    T2Debug("%s --out\n", __FUNCTION__);

    return status;
}

T2ERROR publishEventsDCMSetConf(char *confPath)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    if(confPath == NULL)
    {
        T2Error("Configuration path is NULL \n");
        return T2ERROR_FAILURE;
    }

    rbusEvent_t  event = {0};
    rbusObject_t data;
    rbusValue_t  value;

    T2ERROR status = T2ERROR_SUCCESS;
    rbusError_t ret = RBUS_ERROR_SUCCESS;

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        T2Error("Failed to initialize rbus \n");
        return T2ERROR_FAILURE;
    }

    T2Debug("Publishing event for configuration path to DCM component \n");
    rbusValue_Init(&value);
    rbusValue_SetString(value, confPath);

    rbusObject_Init(&data, NULL);
    rbusObject_SetValue(data, T2_DCM_SET_CONFIG, value);

    event.name = T2_EVENT_DCM_SETCONF;
    event.data = data;
    event.type = RBUS_EVENT_GENERAL;

    ret = rbusEvent_Publish(t2bus_handle, &event);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        T2Error("rbusEvent_Publish %s failed: %d\n", T2_EVENT_DCM_SETCONF, ret);
        status = T2ERROR_FAILURE;
    }

    rbusValue_Release(value);
    if(data != NULL)
    {
        T2Debug("Releasing the rbusobject \n");
        rbusObject_Release(data);
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return status;
}


T2ERROR publishEventsDCMProcConf()
{
    T2Debug("%s ++in\n", __FUNCTION__);

    rbusEvent_t  event = {0};
    rbusObject_t data;
    rbusValue_t  value;

    T2ERROR status  = T2ERROR_SUCCESS;
    rbusError_t ret = RBUS_ERROR_SUCCESS;

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        T2Error("Failed to initialize rbus \n");
        return T2ERROR_FAILURE;
    }

    T2Debug("Publishing event for configuration path to DCM component \n");
    rbusValue_Init(&value);
    rbusValue_SetString(value, "start");

    rbusObject_Init(&data, NULL);
    rbusObject_SetValue(data, T2_DCM_START_CONFIG, value);

    event.name = T2_EVENT_DCM_PROCCONF;
    event.data = data;
    event.type = RBUS_EVENT_GENERAL;

    ret = rbusEvent_Publish(t2bus_handle, &event);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        T2Error("rbusEvent_Publish %s failed: %d\n", T2_EVENT_DCM_PROCCONF, ret);
        status = T2ERROR_FAILURE;
    }

    rbusValue_Release(value);
    if(data != NULL)
    {
        T2Debug("Releasing the rbusobject \n");
        rbusObject_Release(data);
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return status;
}
#endif


T2ERROR registerRbusT2EventListener(TelemetryEventCallback eventCB)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    T2ERROR status = T2ERROR_SUCCESS;
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        return T2ERROR_FAILURE;
    }

    /**
     * Register data elements with rbus for EVENTS and Profile Updates.
     */
    rbusDataElement_t dataElements[3] =
    {
        {T2_EVENT_PARAM, RBUS_ELEMENT_TYPE_PROPERTY, {NULL, t2PropertyDataSetHandler, NULL, NULL, NULL, NULL}},
        {T2_PROFILE_UPDATED_NOTIFY, RBUS_ELEMENT_TYPE_EVENT, {NULL, NULL, NULL, NULL, (rbusEventSubHandler_t)eventSubHandler, NULL}},
        {T2_OPERATIONAL_STATUS, RBUS_ELEMENT_TYPE_PROPERTY, {t2PropertyDataGetHandler, NULL, NULL, NULL, NULL, NULL}}
    };
    ret = rbus_regDataElements(t2bus_handle, 3, dataElements);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        T2Error("Failed to register T2 data elements with rbus. Error code : %d\n", ret);
        status = T2ERROR_FAILURE ;
    }
    eventCallBack = eventCB ;

    T2Info("%s initialize Unix socket server\n", __FUNCTION__);
    if (t2_init_tcp_server() != T2ERROR_SUCCESS) {
        T2Info("Failed to initialize Unix socket server\n");
    }
    T2Info("%s initialized Unix socket server\n", __FUNCTION__);
    T2Info("%s starting TCP server thread\n", __FUNCTION__);
    g_tcp_server.server_running = true;
    
    if (pthread_create(&g_tcp_server.server_thread, NULL, t2_tcp_server_thread, NULL) != 0) {
        T2Error("Failed to create TCP server thread: %s\n", strerror(errno));
        g_tcp_server.server_running = false;
        close(g_tcp_server.server_fd);
        pthread_mutex_destroy(&g_tcp_server.clients_mutex);
        return T2ERROR_FAILURE;
    }
    
    T2Info("%s TCP server thread started successfully\n", __FUNCTION__);

    T2Debug("%s --out\n", __FUNCTION__);
    return status;
}

void setT2EventReceiveState(int T2_STATE)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    t2ReadyStatus |= T2_STATE;

    T2Debug("%s ++out\n", __FUNCTION__);
}

T2ERROR unregisterRbusT2EventListener()
{
    rbusEvent_Unsubscribe(t2bus_handle, T2_EVENT_PARAM);
    rBusInterface_Uninit();
    return T2ERROR_SUCCESS;
}

/**
 * Register data elements for COMPONENT marker list over bus for common lib event filtering.
 * Data element over bus will be of the format :Telemetry.ReportProfiles.<componentName>.EventMarkerList
 */
T2ERROR regDEforCompEventList(const char* componentName, T2EventMarkerListCallback callBackHandler)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    char deNameSpace[125] = { '\0' };
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    T2ERROR status = T2ERROR_SUCCESS;

    if(!componentName)
    {
        return status;
    }

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        T2Error("%s Failed in getting bus handles \n", __FUNCTION__);
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    pthread_mutex_lock(&compParamMap);

    if(compTr181ParamMap == NULL)
    {
        compTr181ParamMap = hash_map_create();
    }
    else
    {
        char *existingProperty = (char*) hash_map_get(compTr181ParamMap, componentName);
        if (existingProperty != NULL)
        {
            T2Warning("The paramenter exist with compName : %s, the registered paramenter is %s, skipping registration to avoid duplicate registration... \n", componentName, existingProperty );
            pthread_mutex_unlock(&compParamMap);
            return status;
        }
    }
    pthread_mutex_unlock(&compParamMap);
    snprintf(deNameSpace, 124, "%s%s%s", T2_ROOT_PARAMETER, componentName, T2_EVENT_LIST_PARAM_SUFFIX);

    rbusDataElement_t dataElements[1] =
    {
        { deNameSpace, RBUS_ELEMENT_TYPE_PROPERTY, { t2PropertyDataGetHandler, NULL, NULL, NULL, NULL, NULL } }
    };
    ret = rbus_regDataElements(t2bus_handle, 1, dataElements);
    if(ret == RBUS_ERROR_SUCCESS)
    {
        T2Debug("Registered data element %s with bus \n ", deNameSpace);
        pthread_mutex_lock(&compParamMap);
        hash_map_put(compTr181ParamMap, (void*) strdup(deNameSpace), (void*) strdup(componentName), free);
        pthread_mutex_unlock(&compParamMap);
        T2Debug("Save dataelement mapping, %s with component name %s \n ", deNameSpace, componentName);
    }
    else
    {
        T2Error("Failed in registering data element %s \n", deNameSpace);
        status = T2ERROR_FAILURE;
    }

    if(!getMarkerListCallBack)
    {
        getMarkerListCallBack = callBackHandler;
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return status;
}

void freeComponentEventList(void *data)
{
    hash_element_t *componentEventList = (hash_element_t*) data;
    if(componentEventList)
    {
        if(componentEventList->data)
        {
            free(componentEventList->data);
            componentEventList->data = NULL;
        }

        if(componentEventList->key)
        {
            free(componentEventList->key);
            componentEventList->data = NULL;
        }
        free(componentEventList);
        componentEventList = NULL;
    }
}

/**
 * Unregister data elements for COMPONENT marker list over bus
 * Data element over bus will be of the format :Telemetry.ReportProfiles.<componentName>.EventMarkerList
 */
void unregisterDEforCompEventList()
{

    int count = 0;
    int i = 0;
    T2Debug("%s ++in\n", __FUNCTION__);

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        T2Error("%s Failed in getting bus handles \n", __FUNCTION__);
        T2Debug("%s --out\n", __FUNCTION__);
        return ;
    }

    pthread_mutex_lock(&compParamMap);

    if(!compTr181ParamMap)
    {
        T2Info("No data elements present to unregister\n");
        T2Debug("%s --out\n", __FUNCTION__);
        pthread_mutex_unlock(&compParamMap);
        return;
    }

    count = hash_map_count(compTr181ParamMap);
    pthread_mutex_unlock(&compParamMap);
    T2Debug("compTr181ParamMap has %d components registered \n", count);
    if(count > 0)
    {
        rbusDataElement_t dataElements[count];
        rbusCallbackTable_t cbTable = { t2PropertyDataGetHandler, NULL, NULL, NULL, NULL, NULL };
        for( i = 0; i < count; ++i )
        {
            pthread_mutex_lock(&compParamMap);
            char *dataElementName = hash_map_lookupKey(compTr181ParamMap, i);
            pthread_mutex_unlock(&compParamMap);
            if(dataElementName)
            {
                T2Debug("Adding %s to unregister list \n", dataElementName);
                dataElements[i].name = dataElementName;
                dataElements[i].type = RBUS_ELEMENT_TYPE_PROPERTY;
                dataElements[i].cbTable = cbTable;
            }
            else
            {
                dataElements[i].name = NULL;
            }
        }
        if(RBUS_ERROR_SUCCESS != rbus_unregDataElements(t2bus_handle, count, dataElements))
        {
            T2Error("Failed to unregister to dataelements");
        }
    }
    T2Debug("Freeing compTr181ParamMap \n");
    pthread_mutex_lock(&compParamMap);
    hash_map_destroy(compTr181ParamMap, freeComponentEventList);
    compTr181ParamMap = NULL;
    pthread_mutex_unlock(&compParamMap);

    T2Debug("%s --out\n", __FUNCTION__);
}

/**
 * Register data elements for dataModel implementation.
 * Data element over bus will be Device.X_RDKCENTRAL-COM_T2.ReportProfiles,
 *    Device.X_RDKCENTRAL-COM_T2.ReportProfilesMsgPack
 */
T2ERROR regDEforProfileDataModel(callBackHandlers* cbHandlers)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    char deNameSpace[125] = { '\0' };
    char deMsgPck[125] = { '\0' };
    char deTmpNameSpace[125] = { '\0' };
    char deTotalMemUsage[125] = { '\0' };
    char deReportonDemand[125] = { '\0' };
    char deAbortReportonDemand[125] = { '\0' };
    char dePrivacyModesnotshare[125] = { '\0' };

    rbusError_t ret = RBUS_ERROR_SUCCESS;
    T2ERROR status = T2ERROR_SUCCESS;

    if(!cbHandlers)
    {
        T2Error("Callback handlers are NULL \n");
        return T2ERROR_FAILURE;
    }

    if(cbHandlers->dmSavedJsonCallBack)
    {
        dmSavedJsonProcessingCallBack = cbHandlers->dmSavedJsonCallBack;
    }

    if(cbHandlers->dmSavedMsgPackCallBack)
    {
        dmSavedMsgPackProcessingCallBack = cbHandlers->dmSavedMsgPackCallBack;
    }

    if(cbHandlers->pmCallBack)
    {
        profilememUsedCallBack = cbHandlers->pmCallBack;
    }

    if(cbHandlers->reportonDemand)
    {
        reportOnDemandCallBack = cbHandlers->reportonDemand;
    }

    if (cbHandlers->dmCallBack)
    {
        dmProcessingCallBack = cbHandlers->dmCallBack ;
    }

    if(cbHandlers->dmMsgPckCallBackHandler)
    {
        dmMsgPckProcessingCallBack = cbHandlers->dmMsgPckCallBackHandler;
    }

    if(cbHandlers->privacyModesDoNotShare)
    {
        privacyModesDoNotShareCallBack = cbHandlers->privacyModesDoNotShare;
    }

    if(cbHandlers->mprofilesdeleteDoNotShare)
    {
        mprofilesDeleteCallBack = cbHandlers->mprofilesdeleteDoNotShare;
    }

    snprintf(deNameSpace, 124, "%s", T2_REPORT_PROFILE_PARAM);
    snprintf(deMsgPck, 124, "%s", T2_REPORT_PROFILE_PARAM_MSG_PCK);
    snprintf(deTmpNameSpace, 124, "%s", T2_TEMP_REPORT_PROFILE_PARAM);
    snprintf(deTotalMemUsage, 124, "%s", T2_TOTAL_MEM_USAGE);
    snprintf(deReportonDemand, 124, "%s", T2_ON_DEMAND_REPORT);
    snprintf(deAbortReportonDemand, 124, "%s", T2_ABORT_ON_DEMAND_REPORT);
    snprintf(dePrivacyModesnotshare, 124, "%s", PRIVACYMODES_RFC);

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        T2Error("%s Failed in getting bus handles \n", __FUNCTION__);
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    rbusDataElement_t dataElements[NUM_PROFILE_ELEMENTS] =
    {
        {deNameSpace, RBUS_ELEMENT_TYPE_PROPERTY, {t2PropertyDataGetHandler, t2PropertyDataSetHandler, NULL, NULL, NULL, NULL}},
        {deMsgPck, RBUS_ELEMENT_TYPE_PROPERTY, {t2PropertyDataGetHandler, t2PropertyDataSetHandler, NULL, NULL, NULL, NULL}},
        {deTmpNameSpace, RBUS_ELEMENT_TYPE_PROPERTY, {t2PropertyDataGetHandler, t2PropertyDataSetHandler, NULL, NULL, NULL, NULL}},
        {deTotalMemUsage, RBUS_ELEMENT_TYPE_PROPERTY, {t2PropertyDataGetHandler, NULL, NULL, NULL, NULL, NULL}},
        {deReportonDemand, RBUS_ELEMENT_TYPE_METHOD, {NULL, NULL, NULL, NULL, NULL, dcmOnDemandMethodHandler}},
        {deAbortReportonDemand, RBUS_ELEMENT_TYPE_METHOD, {NULL, NULL, NULL, NULL, NULL, dcmOnDemandMethodHandler}},
        {dePrivacyModesnotshare, RBUS_ELEMENT_TYPE_PROPERTY, {t2PropertyDataGetHandler, t2PropertyDataSetHandler, NULL, NULL, NULL, NULL}}
    };
    ret = rbus_regDataElements(t2bus_handle, NUM_PROFILE_ELEMENTS, dataElements);
    if(ret == RBUS_ERROR_SUCCESS)
    {
        T2Debug("Registered data element %s with bus \n ", deNameSpace);
    }
    else
    {
        T2Error("Failed in registering data element %s \n", deNameSpace);
        status = T2ERROR_FAILURE;
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return status ;
}


T2ERROR publishEventsProfileUpdates()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    rbusEvent_t event;
    rbusObject_t data;
    rbusValue_t value;
    T2ERROR status = T2ERROR_SUCCESS;
    rbusError_t ret = RBUS_ERROR_SUCCESS;

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        return T2ERROR_FAILURE;
    }

    T2Debug("Publishing event for t2profile update notification to components \n");
    rbusValue_Init(&value);
    rbusValue_SetString(value, "t2ProfileUpdated");
    rbusObject_Init(&data, NULL);
    rbusObject_SetValue(data, "t2ProfileUpdated", value);

    event.name = T2_PROFILE_UPDATED_NOTIFY;
    event.data = data;
    event.type = RBUS_EVENT_GENERAL;
    ret = rbusEvent_Publish(t2bus_handle, &event);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        T2Debug("provider: rbusEvent_Publish Event1 failed: %d\n", ret);
        status = T2ERROR_FAILURE;
    }

    rbusValue_Release(value);
    if(data != NULL)
    {
        T2Debug("Releasing the rbusobject \n");
        rbusObject_Release(data);
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return status;
}

void registerConditionalReportCallBack(triggerReportOnCondtionCallBack triggerConditionCallback)
{
    if(!reportOnConditionCallBack)
    {
        reportOnConditionCallBack = triggerConditionCallback ;
    }
}

void reportEventHandler(
    rbusHandle_t handle,
    rbusEvent_t const* event,
    rbusEventSubscription_t* subscription)
{
    (void)handle;
    (void)subscription; // To avoid compiler warning
    T2Debug("in Function %s \n", __FUNCTION__);
    T2Debug("Call the callback for the prop %s\n", event->name ? event->name : "NONAME");
    const char* eventName = event->name;

    rbusValue_t newValue = rbusObject_GetValue(event->data, "value");
    const char* eventValue = NULL;
    if(newValue)
    {
        eventValue = rbusValue_ToString(newValue, NULL, 0);
    }

    eventCallBack(eventName ? (char*) strdup(eventName) : NULL, eventValue ? (char*) strdup(eventValue) : (char*) strdup("NOVALUE"));
    if(eventValue != NULL)
    {
        free((char*)eventValue);
        eventValue = NULL;
    }
    T2Debug("exit Function %s \n", __FUNCTION__);
}

void triggerCondtionReceiveHandler(
    rbusHandle_t handle,
    rbusEvent_t const* event,
    rbusEventSubscription_t* subscription)
{
    (void)handle;
    (void)subscription;
    rbusValue_t newValue = rbusObject_GetValue(event->data, "value");
    rbusValue_t oldValue = rbusObject_GetValue(event->data, "oldValue");
    rbusValue_t filter = rbusObject_GetValue(event->data, "filter");
    const char* eventName = event->name;
    char* eventValue = rbusValue_ToString(newValue, NULL, 0);

    if( NULL == eventName)
    {
        if(eventValue != NULL)
        {
            free(eventValue);
            eventValue = NULL;
        }
        T2Warning("%s : eventName is published as null, ignoring trigger condition \n ", __FUNCTION__);
        return ;
    }

    if( NULL == eventValue)
    {
        T2Warning("%s : eventValue is published as null, ignoring trigger condition \n ", __FUNCTION__);
        return ;
    }

    T2Debug("Consumer receiver event for param %s\n and the value %s\n", event->name, eventValue);

    if(newValue)
    {
        char* newVal = rbusValue_ToString(newValue, NULL, 0);
        if(newVal)
        {
            T2Info("  New Value: %s \n", newVal);
            free(newVal);
        }
    }
    if(oldValue)
    {
        char* oldVal = rbusValue_ToString(oldValue, NULL, 0);
        if(oldVal)
        {
            T2Info("  Old Value: %s \n", oldVal );
            free(oldVal);
        }
    }

    if(reportOnConditionCallBack)
    {
        if(filter)
        {
            T2Debug("Filter event\n");
            if(rbusValue_GetBoolean(filter) == 1)
            {
                reportOnConditionCallBack(eventName, eventValue);
            }
        }
        else
        {
            T2Debug("ValueChange event\n");
            reportOnConditionCallBack(eventName, eventValue);
        }
    }

    free(eventValue);
    eventValue = NULL ;

}

T2ERROR rbusT2ConsumerReg(Vector *triggerConditionList)
{
    size_t j;
    int ret = T2ERROR_SUCCESS;
    int status = T2ERROR_SUCCESS;
    T2Debug("--in %s \n", __FUNCTION__);

    for( j = 0; j < triggerConditionList->count; j++ )
    {
        TriggerCondition *triggerCondition = ((TriggerCondition *) Vector_At(triggerConditionList, j));
        if(triggerCondition)
        {
            T2Debug("Adding to register consumer list \n");
            status = T2RbusConsumer(triggerCondition);
            if(status == T2ERROR_FAILURE)
            {
                ret = T2ERROR_FAILURE;
            }
            T2Debug("T2RbusConsumer return = %d\n", ret);
        }
    }
    return ret;
}

T2ERROR rbusT2ConsumerUnReg(Vector *triggerConditionList)
{
    int rc;
    size_t j;
    //char user_data[32] = {0};
    T2Debug("%s ++in\n", __FUNCTION__);
    t2_unix_socket_server_uninit();

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        T2Error("%s Failed in getting bus handles \n", __FUNCTION__);
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    for( j = 0; j < triggerConditionList->count; j++ )
    {
        TriggerCondition *triggerCondition = ((TriggerCondition *) Vector_At(triggerConditionList, j));
        T2Debug("Adding %s to unregister list \n", triggerCondition->reference);
        rbusFilter_RelationOperator_t filterOperator = RBUS_FILTER_OPERATOR_EQUAL;
        rbusFilter_t filter;
        rbusValue_t filterValue;
        rbusEventSubscription_t subscription = {triggerCondition->reference, NULL, 0, 0, (void *)triggerCondtionReceiveHandler, NULL, NULL, NULL, false};

        if(strcmp(triggerCondition->oprator, "lt") == 0)
        {
            filterOperator = RBUS_FILTER_OPERATOR_LESS_THAN;
        }
        if(strcmp(triggerCondition->oprator, "gt") == 0)
        {
            filterOperator = RBUS_FILTER_OPERATOR_GREATER_THAN;
        }
        if(strcmp(triggerCondition->oprator, "eq") == 0)
        {
            filterOperator = RBUS_FILTER_OPERATOR_EQUAL;
        }
        if(strcmp(triggerCondition->oprator, "any") == 0)
        {
            rc = rbusEvent_Unsubscribe(
                     t2bus_handle,
                     triggerCondition->reference);
            if (rc != RBUS_ERROR_SUCCESS)
            {
                T2Debug("%s UnSubscribe failed\n", __FUNCTION__);
            }
        }
        else
        {
            rbusValue_Init(&filterValue);
            rbusValue_SetInt32(filterValue, triggerCondition->threshold);
            rbusFilter_InitRelation(&filter, filterOperator, filterValue);
            subscription.filter = filter;
            rc = rbusEvent_UnsubscribeEx(t2bus_handle, &subscription, 1);
            if (rc != RBUS_ERROR_SUCCESS)
            {
                T2Debug("%s UnSubscribeEx failed\n", __FUNCTION__);
            }
        }
    }
    return T2ERROR_SUCCESS;
}

T2ERROR T2RbusConsumer(TriggerCondition *triggerCondition)
{
    int rc = RBUS_ERROR_SUCCESS;
    int ret = T2ERROR_SUCCESS;
    char user_data[32] = {0};
    //char componentName[] = "t2consumer";
    T2Debug("--in %s\n", __FUNCTION__);
    if(triggerCondition->isSubscribed == true)
    {
        T2Debug("%s already subscribed\n", triggerCondition->reference);
        return T2ERROR_SUCCESS;
    }

    rbusFilter_RelationOperator_t filterOperator = RBUS_FILTER_OPERATOR_EQUAL; // Initialized with a default value
    rbusFilter_t filter;
    rbusValue_t filterValue;
    rbusEventSubscription_t subscription = {triggerCondition->reference, NULL, 0, 0, triggerCondtionReceiveHandler, NULL, NULL, NULL, false};

    if(strcmp(triggerCondition->oprator, "lt") == 0)
    {
        filterOperator = RBUS_FILTER_OPERATOR_LESS_THAN;
    }
    if(strcmp(triggerCondition->oprator, "gt") == 0)
    {
        filterOperator = RBUS_FILTER_OPERATOR_GREATER_THAN;
    }
    if(strcmp(triggerCondition->oprator, "eq") == 0)
    {
        filterOperator = RBUS_FILTER_OPERATOR_EQUAL;
    }

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        T2Debug("Consumer: rbus_open failed: %d\n", rc);
        return T2ERROR_FAILURE;
    }
    snprintf(user_data, sizeof(user_data), "Not used");
    if(strcmp(triggerCondition->oprator, "any") == 0)
    {
        T2Debug("filterOperator %s , threshold %d \n", triggerCondition->oprator, triggerCondition->threshold);
        rc = rbusEvent_Subscribe(
                 t2bus_handle,
                 triggerCondition->reference,
                 triggerCondtionReceiveHandler,
                 user_data,
                 0);
        if (rc != RBUS_ERROR_SUCCESS)
        {
            T2Error(" %s Subscribe failed\n", __FUNCTION__);
            ret = T2ERROR_FAILURE;
        }
        else
        {
            triggerCondition->isSubscribed = true;
        }
    }
    else
    {
        T2Debug("Ex filterOperator %s ( %d ) , threshold %d \n", triggerCondition->oprator, filterOperator, triggerCondition->threshold);
        rbusValue_Init(&filterValue);
        rbusValue_SetInt32(filterValue, triggerCondition->threshold);
        rbusFilter_InitRelation(&filter, filterOperator, filterValue);
        subscription.filter = filter;
        rc = rbusEvent_SubscribeEx(t2bus_handle, &subscription, 1, 0);
        if (rc != RBUS_ERROR_SUCCESS)
        {
            T2Error(" %s SubscribeEx failed\n", __FUNCTION__);
            ret = T2ERROR_FAILURE;
        }
        else
        {
            triggerCondition->isSubscribed = true;
        }
        rbusValue_Release(filterValue);
        rbusFilter_Release(filter);
    }
    return ret;
}

T2ERROR T2RbusReportEventConsumer(char* reference, bool subscription)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    int rc = RBUS_ERROR_SUCCESS;
    if (!subscription)
    {
        rc = rbusEvent_Unsubscribe(
                 t2bus_handle,
                 reference);
        if (rc != RBUS_ERROR_SUCCESS)
        {
            T2Debug("--in %s there is an issue in unsubscribe \n", __FUNCTION__);
        }
        T2Debug("%s --out\n", __FUNCTION__);
        return rc;
    }
    else
    {
        int ret = T2ERROR_SUCCESS;
        char user_data[32] = {0};
        //char componentName[] = "t2consumer";
        T2Debug("--in %s\n", __FUNCTION__);
        if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
        {
            T2Debug("Consumer: rbus_open failed\n");
            T2Debug("%s --out\n", __FUNCTION__);
            return T2ERROR_FAILURE;
        }
        snprintf(user_data, sizeof(user_data), "Not used");
        rc = rbusEvent_Subscribe(
                 t2bus_handle,
                 reference,
                 reportEventHandler,
                 user_data,
                 0);
        if (rc != RBUS_ERROR_SUCCESS)
        {
            T2Error(" %s Subscribe failed\n", __FUNCTION__);
            ret = T2ERROR_FAILURE;
        }
        T2Debug("%s --out\n", __FUNCTION__);
        return ret;
    }
}

rbusError_t t2TriggerConditionGetHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t* opts)
{
    char const* name = rbusProperty_GetName(property);
    (void)handle;
    (void)opts;

    T2Debug("Provider: Called get handler for [%s] \n", name);
    static int32_t mydata = 0;
    rbusValue_t value;
    rbusValue_Init(&value);
    rbusValue_SetInt32(value, mydata);
    rbusProperty_SetValue(property, value);
    rbusValue_Release(value);
    return RBUS_ERROR_SUCCESS;
}


T2ERROR rbusMethodCaller(char *methodName, rbusObject_t* inputParams, char* payload, rbusMethodCallBackPtr rbusMethodCallBack )
{
    T2Debug("%s ++in\n", __FUNCTION__);

    T2ERROR ret = T2ERROR_FAILURE;
    int rc = RBUS_ERROR_BUS_ERROR;
    T2Info("methodName = %s payload = %s \n", methodName, payload);

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        T2Error("%s Failed in getting bus handles \n", __FUNCTION__);
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
    rc = rbusMethod_InvokeAsync(t2bus_handle, methodName, *inputParams, rbusMethodCallBack, RBUS_METHOD_TIMEOUT);
    if (rc == RBUS_ERROR_SUCCESS)
    {
        ret = T2ERROR_SUCCESS ;
    }
    else
    {
        T2Error("rbusMethod_InvokeAsync invocation from %s failed with error code %d \n", __FUNCTION__, rc);
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}

bool rbusCheckMethodExists(const char* rbusMethodName)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    rbusError_t rc = RBUS_ERROR_BUS_ERROR;
    rbusObject_t inParams, outParams;
    rbusValue_t value;

    T2Info("methodName = %s \n", rbusMethodName);

    if(!t2bus_handle && T2ERROR_SUCCESS != rBusInterface_Init())
    {
        T2Error("%s Failed in getting bus handles \n", __FUNCTION__);
        T2Debug("%s --out\n", __FUNCTION__);
        return false;
    }
    rbusObject_Init(&inParams, NULL);
    rbusValue_Init(&value);
    rbusValue_SetString(value, "");
    rbusObject_SetValue(inParams, "check", value);
    rc = rbusMethod_Invoke(t2bus_handle, rbusMethodName, inParams, &outParams);
    rbusValue_Release(value);
    rbusObject_Release(inParams);

    T2Debug("%s --out\n", __FUNCTION__);
    if (rc != RBUS_ERROR_SUCCESS)
    {
        T2Debug("rbusMethod_Invoke called: %s with return code \n  error = %s \n", rbusMethodName, rbusError_ToString(rc));
        T2Info("Rbus method %s doesn't exists \n", rbusMethodName );
        return false ;
    }
    rbusObject_Release(outParams);
    return true ;
}
