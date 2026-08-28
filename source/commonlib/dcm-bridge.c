#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "telemetry_busmessage_sender.h"

#define LINE_SIZE 8192
#define HEADER_SIZE 256

static int has_suffix(const char *text, const char *suffix)
{
    size_t text_length;
    size_t suffix_length;

    if (text == NULL || suffix == NULL) {
        return 0;
    }

    text_length = strlen(text);
    suffix_length = strlen(suffix);

    return text_length >= suffix_length &&
           strcmp(text + text_length - suffix_length, suffix) == 0;
}

static int parse_marker_line(const char *line,
                             char *header,
                             size_t header_size,
                             char *message,
                             size_t message_size)
{
    const char *header_start;
    const char *header_end;
    const char *message_start;
    size_t header_length;
    size_t message_length;

    header_start = strstr(line, "header=");
    message_start = strstr(line, " msg=");

    if (header_start == NULL || message_start == NULL) {
        return -1;
    }

    header_start += strlen("header=");
    header_end = strchr(header_start, ' ');

    if (header_end == NULL || header_end >= message_start) {
        return -1;
    }

    header_length = (size_t)(header_end - header_start);

    if (header_length == 0 || header_length >= header_size) {
        return -1;
    }

    memcpy(header, header_start, header_length);
    header[header_length] = '\0';

    message_start += strlen(" msg=");
    message_length = strlen(message_start);

    while (message_length > 0 &&
           (message_start[message_length - 1] == '\n' ||
            message_start[message_length - 1] == '\r')) {
        message_length--;
    }

    if (message_length == 0 || message_length >= message_size) {
        return -1;
    }

    memcpy(message, message_start, message_length);
    message[message_length] = '\0';

    return 0;
}

static void send_marker_to_t2(const char *header, const char *message)
{
    T2ERROR status;

    if (has_suffix(header, "_split") ||
        has_suffix(header, "_accum")) {
        status = t2_event_s(header, message);
    } else {
        status = t2_event_d(header, 1);
    }

    if (status != T2ERROR_SUCCESS) {
        fprintf(stderr,
                "Failed to send marker to T2: header=%s status=%d\n",
                header,
                status);
    }
}

int main(void)
{
    char line[LINE_SIZE];
    char header[HEADER_SIZE];
    char message[LINE_SIZE];

    t2_init("dcm-bridge");

    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (parse_marker_line(line,
                             header,
                             sizeof(header),
                             message,
                             sizeof(message)) != 0) {
            fprintf(stderr, "Invalid marker line: %s", line);
            continue;
        }

        send_marker_to_t2(header, message);
    }

    t2_uninit();
    return EXIT_SUCCESS;
}
