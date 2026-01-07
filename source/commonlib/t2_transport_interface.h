#ifndef T2_TRANSPORT_INTERFACE_H
#define T2_TRANSPORT_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include "telemetry2_0.h"

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


// Transport mode enumeration
typedef enum
{
    T2_TRANSPORT_MODE_RBUS = 0,
    T2_TRANSPORT_MODE_UNIX_SOCKET = 1,
    T2_TRANSPORT_MODE_MESSAGE_QUEUE = 2
} T2TransportMode;


// Function declarations for mode selection
void t2_set_transport_mode(T2TransportMode mode);
T2TransportMode t2_get_transport_mode(void);
void t2_set_transport_mode_from_env(void);
const char* t2_get_transport_mode_name(void);
int send_event_data(const char* data, const char *markerName);
void rBusInterface_Uninit( );
int report_or_cache_data(char* telemetry_data, const char* markerName);

// Communication subsystem functions
T2ERROR t2_communication_init(char *component);
void t2_communication_cleanup(void);

// Event sending function (unified for all transport modes)
int t2_send_event_data(const char* data, const char *markerName);

// Marker query functions
T2ERROR t2_query_event_markers(void);
void t2_parse_and_store_markers(const char* marker_data);

// Compatibility functions for backward compatibility
void t2_set_communication_mode(bool use_rbus);
bool t2_get_communication_mode(void);

#endif // T2_TRANSPORT_INTERFACE_H