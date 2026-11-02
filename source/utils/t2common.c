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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <strings.h>
#include "t2common.h"
#include "t2log_wrapper.h"
#include "dcautil.h"

bool whoami_support = false;

int openSecureFileNoFollow(const char *path, int flags, mode_t mode)
{
    int fd = -1;

    if (path == NULL || *path == '\0')
    {
        return -1;
    }

    fd = open(path, flags | O_NOFOLLOW, mode);
    if (fd < 0)
    {
        T2Error("Unable to safely open %s: %s\n", path, strerror(errno));
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) || st.st_uid != geteuid())
    {
        T2Error("Refusing to use unsafe file: %s\n", path);
        close(fd);
        return -1;
    }

    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
    {
        T2Error("Unable to set 0600 permissions on %s: %s\n", path, strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

int openSecureTempFile(const char *path, bool truncateExisting)
{
    int flags = O_WRONLY | O_CREAT;
    if (truncateExisting)
    {
        flags |= O_TRUNC;
    }

    int fd = openSecureFileNoFollow(path, flags, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        return -1;
    }

    if (truncateExisting && ftruncate(fd, 0) != 0)
    {
        T2Error("Unable to truncate %s: %s\n", path, strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

bool createSecureTempFlag(const char *path)
{
    if (path == NULL || *path == '\0')
    {
        return false;
    }

    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        if (errno == EEXIST)
        {
            T2Debug("Existing secure flag already present: %s\n", path);
            return true;
        }

        T2Error("Unable to create secure flag %s: %s\n", path, strerror(errno));
        return false;
    }

    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
    {
        T2Error("Unable to set 0600 permissions on %s: %s\n", path, strerror(errno));
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

static bool isLocalNumericAddress(const char *host)
{
    if (host == NULL || *host == '\0')
    {
        return false;
    }

    struct in_addr addr4;
    if (inet_pton(AF_INET, host, &addr4) == 1)
    {
        uint32_t ip = ntohl(addr4.s_addr);
        return ((ip & 0xFF000000u) == 0x7F000000u) || (ip == 0x00000000u);
    }

    if (inet_aton(host, &addr4) == 1)
    {
        uint32_t ip = ntohl(addr4.s_addr);
        return ((ip & 0xFF000000u) == 0x7F000000u) || (ip == 0x00000000u);
    }

    if (strchr(host, ':') == NULL)
    {
        errno = 0;
        char *end = NULL;
        unsigned long long numeric = strtoull(host, &end, 0);
        if (errno == 0 && end != host && *end == '\0' && numeric <= 0xFFFFFFFFULL)
        {
            struct in_addr parsed;
            parsed.s_addr = htonl((uint32_t)numeric);
            uint32_t ip = ntohl(parsed.s_addr);
            return ((ip & 0xFF000000u) == 0x7F000000u) || (ip == 0x00000000u);
        }
    }

    struct in6_addr addr6;
    if (inet_pton(AF_INET6, host, &addr6) == 1)
    {
        if (IN6_IS_ADDR_LOOPBACK(&addr6) || IN6_IS_ADDR_UNSPECIFIED(&addr6))
        {
            return true;
        }

        if (IN6_IS_ADDR_V4MAPPED(&addr6))
        {
            struct in_addr mapped_v4;
            memcpy(&mapped_v4, &addr6.s6_addr[12], sizeof(mapped_v4));
            uint32_t mapped_ip = ntohl(mapped_v4.s_addr);
            return ((mapped_ip & 0xFF000000u) == 0x7F000000u) || (mapped_ip == 0x00000000u);
        }
    }

    return false;
}

bool isValidHttpsUploadUrl(const char *url)
{
    if (url == NULL || *url == '\0')
    {
        return false;
    }

    char url_copy[2048];
    size_t url_len = strlen(url);
    if (url_len == 0 || url_len >= sizeof(url_copy))
    {
        return false;
    }

    snprintf(url_copy, sizeof(url_copy), "%s", url);
    char *scheme_end = strstr(url_copy, "://");
    if (scheme_end == NULL)
    {
        return false;
    }

    *scheme_end = '\0';
    if (strcasecmp(url_copy, "https") != 0)
    {
        return false;
    }

    char *authority = scheme_end + 3;
    if (*authority == '\0')
    {
        return false;
    }

    if (strchr(authority, '@') != NULL)
    {
        return false;
    }

    char *host_end = strpbrk(authority, "/?#");
    if (host_end != NULL)
    {
        *host_end = '\0';
    }

    char host[256];
    memset(host, 0, sizeof(host));
    if (authority[0] == '[')
    {
        char *close_bracket = strchr(authority, ']');
        if (close_bracket == NULL)
        {
            return false;
        }
        size_t host_len = (size_t)(close_bracket - authority - 1);
        if (host_len == 0 || host_len >= sizeof(host))
        {
            return false;
        }
        memcpy(host, authority + 1, host_len);
        host[host_len] = '\0';
    }
    else
    {
        char *port_sep = strrchr(authority, ':');
        if (port_sep != NULL)
        {
            char *first_colon = strchr(authority, ':');
            if (first_colon != NULL && first_colon != port_sep)
            {
                return false;
            }

            size_t host_len = (size_t)(port_sep - authority);
            if (host_len == 0 || host_len >= sizeof(host))
            {
                return false;
            }
            memcpy(host, authority, host_len);
            host[host_len] = '\0';

            char *port_value = port_sep + 1;
            if (*port_value == '\0')
            {
                return false;
            }
            for (char *p = port_value; *p != '\0'; ++p)
            {
                if (*p < '0' || *p > '9')
                {
                    return false;
                }
            }
        }
        else
        {
            size_t hostname_len = strlen(authority);
            if (hostname_len == 0 || hostname_len >= sizeof(host))
            {
                return false;
            }
            snprintf(host, sizeof(host), "%s", authority);
        }
    }

    if (host[0] == '\0')
    {
        return false;
    }

    if (strchr(host, '%') != NULL || isLocalNumericAddress(host))
    {
        return false;
    }

    char lower_host[256];
    size_t host_len = strlen(host);
    if (host_len >= sizeof(lower_host))
    {
        return false;
    }
    for (size_t i = 0; i < host_len; ++i)
    {
        lower_host[i] = (char)tolower((unsigned char)host[i]);
    }
    lower_host[host_len] = '\0';

    if (strcmp(lower_host, "localhost") == 0 ||
            strcmp(lower_host, "localhost.localdomain") == 0 ||
            (host_len >= 10 && strncmp(lower_host, "localhost.", 10) == 0) ||
            (host_len >= 6 && strcmp(lower_host + host_len - 6, ".local") == 0) ||
            (host_len >= 9 && strcmp(lower_host + host_len - 9, ".internal") == 0))
    {
        return false;
    }

    return true;
}

void freeParam(void *data)
{
    if(data != NULL)
    {
        Param *param = (Param *)data;
        if(param->name)
        {
            free(param->name);
        }
        if(param->alias)
        {
            free((char*)param->alias);
        }
        if(param->paramType)
        {
            free(param->paramType);
        }
        if(param->regexParam)
        {
            free(param->regexParam);
        }
        free(param);
    }
}

#ifdef ENABLE_DYNAMIC_TABLE_SUPPORT
void freeDataModelParam(void *data)
{
    if (data)
    {
        DataModelParam *param = (DataModelParam *)data;
        if (param->reference)
        {
            free(param->reference);
        }
        if (param->name)
        {
            free(param->name);
        }
        free(param);
    }
}

void freeDataModelTable(void *data)
{
    if (data)
    {
        DataModelTable *table = (DataModelTable *)data;
        if (table->reference)
        {
            free(table->reference);
        }
        if (table->index)
        {
            free(table->index);
        }
        if (table->paramList)
        {
            Vector_Destroy(table->paramList, freeDataModelParam);
        }
        free(table);
    }
}
#endif

void freeStaticParam(void *data)
{
    if(data != NULL)
    {
        StaticParam *sparam = (StaticParam *)data;
        if(sparam->name)
        {
            free(sparam->name);
        }
        if (sparam->paramType)
        {
            free(sparam->paramType);
        }
        if (sparam->value)
        {
            free(sparam->value);
        }

        free(sparam);
    }
}

void freeEMarker(void *data)
{
    if(data != NULL)
    {
        EventMarker *eMarker = (EventMarker *)data;
        if(eMarker->alias)
        {
            free(eMarker->alias);
        }
        if(eMarker->compName)
        {
            free(eMarker->compName);
        }
        if(eMarker->markerName)
        {
            free(eMarker->markerName);
        }
        if(eMarker->paramType)
        {
            free(eMarker->paramType);
        }
        if(eMarker->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
        {
            if(eMarker->markerName_CT != NULL)
            {
                free(eMarker->markerName_CT);
            }
            if(eMarker->timestamp != NULL)
            {
                free(eMarker->timestamp);
            }
        }
        if(eMarker->regexParam != NULL)
        {
            free(eMarker->regexParam);
        }
        if(eMarker->mType == MTYPE_ABSOLUTE && eMarker->u.markerValue)
        {
            free(eMarker->u.markerValue);
        }
        if(eMarker->mType == MTYPE_ACCUMULATE && eMarker->u.accumulatedValues)
        {
            Vector_Destroy(eMarker->u.accumulatedValues, freeAccumulatedParam);
            if(eMarker->accumulatedTimestamp != NULL && eMarker->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
            {
                Vector_Destroy(eMarker->accumulatedTimestamp, freeAccumulatedParam);
            }
        }
        free(eMarker);
    }
}

void freeAccumulatedParam(void* data)
{
    if(data != NULL)
    {
        char* dataStr = data;
        free(dataStr);
    }
}

void freeGMarker(void *data)
{
    if(data != NULL)
    {
        GrepMarker *gMarker = (GrepMarker *)data;
        if(gMarker->logFile)
        {
            free(gMarker->logFile);
        }
        if(gMarker->markerName)
        {
            free(gMarker->markerName);
        }
        if(gMarker->searchString)
        {
            free(gMarker->searchString);
        }
        if(gMarker->paramType)
        {
            free(gMarker->paramType);
        }
        if(gMarker->mType == MTYPE_ABSOLUTE && gMarker->u.markerValue)
        {
            free(gMarker->u.markerValue);
        }
        if(gMarker->regexParam != NULL)
        {
            free(gMarker->regexParam);
        }
        free(gMarker);
    }
}

/**
 * Comparator eventually called by qsort
 */
int compareLogFileNames(const void *g1, const void *g2)
{
    GrepMarker** p1 = (GrepMarker**) g1 ;
    GrepMarker** p2 = (GrepMarker**) g2 ;

    if ( NULL != p1 && NULL != p2)
    {
        return strcmp((*p1)->logFile, (*p2)->logFile);
    }
    else
    {
        T2Error("compareLogFileNames : either p1 or p2 is NULL \n");
        return -1 ;
    }
}

void freeTriggerCondition(void *data)
{
    if(data != NULL)
    {
        TriggerCondition *tCondition = (TriggerCondition *)data;
        if(tCondition->type)
        {
            free(tCondition->type);
        }
        if(tCondition->oprator)
        {
            free(tCondition->oprator);
        }
        if(tCondition->reference)
        {
            free(tCondition->reference);
        }
        free(tCondition);
    }
}

void freeGResult(void *data)
{
    if(data != NULL)
    {
        GrepResult *grepResult = (GrepResult *) data;
        if(grepResult->markerName)
        {
            free((char*)grepResult->markerName);
        }

        if(grepResult->markerValue)
        {
            free((char*)grepResult->markerValue);
        }

        free(grepResult);
        grepResult = NULL;
    }
}
/* Description: Getting device property by reading device.property file
 * @param: dev_prop_name : Device property name to get from file
 * @param: out_data : pointer to hold the device property get from file
 * @param: buff_size : Buffer size of the out_data.
 * @return bool: Success: true  and Fail : false
 * */
bool getDevicePropertyData(const char *dev_prop_name, char *out_data, unsigned int buff_size)
{
    bool ret = false;
    FILE *fp;
    char tbuff[MAX_DEVICE_PROP_BUFF_SIZE];
    char *tmp;
    int index;
    if (out_data == NULL || dev_prop_name == NULL)
    {
        T2Error("%s : parameter is NULL\n", __FUNCTION__);
        return ret;
    }
    if (buff_size == 0 || buff_size > MAX_DEVICE_PROP_BUFF_SIZE)
    {
        T2Error("%s : buff size not in the range. size should be < %d\n", __FUNCTION__, MAX_DEVICE_PROP_BUFF_SIZE);
        return ret;
    }
    T2Debug("%s : Trying device property data for %s and buf size=%u\n", __FUNCTION__, dev_prop_name, buff_size);
    fp = fopen(DEVICE_PROPERTIES_FILE, "r");
    if(fp == NULL)
    {
        T2Error("%s : device.property File not found\n", __FUNCTION__);
        return ret;
    }
    while((fgets(tbuff, sizeof(tbuff), fp) != NULL))
    {
        if(strstr(tbuff, dev_prop_name))
        {
            index = strcspn(tbuff, "\n");
            if (index > 0)
            {
                tbuff[index] = '\0';
            }
            tmp = strchr(tbuff, '=');
            if(tmp != NULL)
            {
                snprintf(out_data, buff_size, "%s", tmp + 1);
                T2Debug("%s : %s=%s\n", __FUNCTION__, dev_prop_name, out_data);
                ret = true;
                break;
            }
            else
            {
                T2Error("%s : strchr failed. '=' not found. str=%s\n", __FUNCTION__, tbuff);
            }
        }
    }
    fclose(fp);
    return ret;
}

void initWhoamiSupport(void)
{
    char buf[8] = {0};
    if (getDevicePropertyData("WHOAMI_SUPPORT", buf, sizeof(buf)) && strcmp(buf, "true") == 0)
    {
        whoami_support = true;
        T2Info("WhoAmI feature is enabled\n");
    }
    else
    {
        whoami_support = false;
        T2Info("WhoAmI feature is disabled\n");
    }
}

bool isWhoAmiEnabled(void)
{
    return whoami_support;
}

#ifdef ENABLE_DYNAMIC_TABLE_SUPPORT
// Function to check if configured parameter matches actual RBUS parameter
bool matchesParameter(const char* pattern, const char* paramName)
{
    if (!pattern || !paramName)
    {
        return false;
    }

    const char* lastPatternSegment = strrchr(pattern, '.');
    const char* lastParamSegment = strrchr(paramName, '.');

    if (lastPatternSegment && lastParamSegment)
    {
        if (strncmp(lastPatternSegment + 1, lastParamSegment + 1, strlen(lastPatternSegment + 1)) != 0)
        {
            return false;
        }
    }

    while (*pattern && *paramName)
    {
        if (*pattern == '*')
        {
            pattern++;
            if (*pattern == '\0')
            {
                return true;
            }
            while (*paramName && *paramName != *pattern)
            {
                paramName++;
            }
        }
        else
        {
            if (*pattern != *paramName)
            {
                return false;
            }
            pattern++;
            paramName++;
        }
    }

    return (*pattern == '\0' && *paramName == '\0');
}
#endif

int sanitize_string(const char *str)
{
    const char *src = str;

    while (*src)
    {
        if (!(isalnum((unsigned char) * src) || *src == '.' || *src == '-' || *src == '_'))
        {
            T2Error("Invalid search string configuration. Rejecting parameter due to invalid characters\n");
            return -1;
        }
        src++;
    }
    return 0;
}
