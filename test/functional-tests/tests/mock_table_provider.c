/*
 * Mock rbus table provider for L2 testing of dataModelTable feature.
 * Registers Device.WiFi.AccessPoint.{1,2,3}.{SSID,Status,Enable} as
 * indexed table parameters accessible via rbus.
 *
 * Build: gcc -o mock_table_provider mock_table_provider.c \
 *        -I/usr/local/include -I/usr/local/include/rbus \
 *        -L/usr/local/lib -lrbus -lrbuscore -lrtMessage -lmsgpackc
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rbus.h>

#define NUM_ROWS 3
#define NUM_PARAMS_PER_ROW 3
#define TOTAL_PARAMS (NUM_ROWS * NUM_PARAMS_PER_ROW)

static rbusHandle_t handle;

/* Base table path - uses custom namespace to avoid conflicts with tr69hostif */
#define TABLE_BASE "Device.X_T2TEST_Table.AccessPoint."

/* Table data */
static const char *ssid_values[NUM_ROWS] = {"HomeNetwork", "GuestNetwork", "IoT_Network"};
static const char *status_values[NUM_ROWS] = {"Enabled", "Enabled", "Disabled"};
static const char *enable_values[NUM_ROWS] = {"true", "true", "false"};

/* Parameter names - Device.X_T2TEST_Table.AccessPoint.{1,2,3}.{SSID,Status,Enable} */
static char paramNames[TOTAL_PARAMS][128];

static void buildParamNames(void)
{
    for (int row = 0; row < NUM_ROWS; row++)
    {
        snprintf(paramNames[row * NUM_PARAMS_PER_ROW + 0], 128, TABLE_BASE "%d.SSID", row + 1);
        snprintf(paramNames[row * NUM_PARAMS_PER_ROW + 1], 128, TABLE_BASE "%d.Status", row + 1);
        snprintf(paramNames[row * NUM_PARAMS_PER_ROW + 2], 128, TABLE_BASE "%d.Enable", row + 1);
    }
}

static const char* getValueForParam(const char *name)
{
    for (int row = 0; row < NUM_ROWS; row++)
    {
        if (strcmp(name, paramNames[row * NUM_PARAMS_PER_ROW + 0]) == 0)
        {
            return ssid_values[row];
        }
        if (strcmp(name, paramNames[row * NUM_PARAMS_PER_ROW + 1]) == 0)
        {
            return status_values[row];
        }
        if (strcmp(name, paramNames[row * NUM_PARAMS_PER_ROW + 2]) == 0)
        {
            return enable_values[row];
        }
    }
    return NULL;
}

static rbusError_t getHandler(rbusHandle_t h, rbusProperty_t prop, rbusGetHandlerOptions_t *opts)
{
    (void)h;
    (void)opts;
    const char *name = rbusProperty_GetName(prop);
    const char *val = getValueForParam(name);
    if (val)
    {
        rbusValue_t value;
        rbusValue_Init(&value);
        rbusValue_SetString(value, val);
        rbusProperty_SetValue(prop, value);
        rbusValue_Release(value);
        return RBUS_ERROR_SUCCESS;
    }
    return RBUS_ERROR_INVALID_INPUT;
}

static rbusError_t tableGetHandler(rbusHandle_t h, rbusProperty_t prop, rbusGetHandlerOptions_t *opts)
{
    (void)h;
    (void)opts;
    const char *name = rbusProperty_GetName(prop);

    /* Handle wildcard query - when someone queries Device.X_T2TEST_Table.AccessPoint. */
    if (strcmp(name, TABLE_BASE) == 0)
    {
        rbusProperty_t current = prop;
        int first = 1;
        for (int i = 0; i < TOTAL_PARAMS; i++)
        {
            rbusValue_t val;
            rbusValue_Init(&val);
            const char *paramVal = getValueForParam(paramNames[i]);
            rbusValue_SetString(val, paramVal ? paramVal : "");
            if (first)
            {
                rbusProperty_SetName(current, paramNames[i]);
                rbusProperty_SetValue(current, val);
                first = 0;
            }
            else
            {
                rbusProperty_t next;
                rbusProperty_Init(&next, paramNames[i], val);
                rbusProperty_Append(current, next);
                rbusProperty_Release(next);
                current = next;
            }
            rbusValue_Release(val);
        }
        return RBUS_ERROR_SUCCESS;
    }

    /* Otherwise, try individual param lookup */
    return getHandler(h, prop, opts);
}

static rbusError_t setHandler(rbusHandle_t h, rbusProperty_t prop, rbusSetHandlerOptions_t *opts)
{
    (void)h;
    (void)prop;
    (void)opts;
    return RBUS_ERROR_SUCCESS;
}

static rbusDataElement_t dataElements[TOTAL_PARAMS + 1]; /* +1 for table element */

static void exitHandler(int sig)
{
    printf("mock_table_provider: caught signal %d, exiting\n", sig);
    rbus_unregDataElements(handle, TOTAL_PARAMS + 1, dataElements);
    rbus_close(handle);
    exit(0);
}

int main(void)
{
    rbusError_t rc;
    buildParamNames();

    printf("mock_table_provider: starting...\n");

    rc = rbus_open(&handle, "mock_table_provider");
    if (rc != RBUS_ERROR_SUCCESS)
    {
        printf("mock_table_provider: rbus_open failed: %d\n", rc);
        return 1;
    }

    /* Register individual property elements */
    for (int i = 0; i < TOTAL_PARAMS; i++)
    {
        dataElements[i].name = paramNames[i];
        dataElements[i].type = RBUS_ELEMENT_TYPE_PROPERTY;
        dataElements[i].cbTable.getHandler = getHandler;
        dataElements[i].cbTable.setHandler = setHandler;
        dataElements[i].cbTable.tableAddRowHandler = NULL;
        dataElements[i].cbTable.tableRemoveRowHandler = NULL;
        dataElements[i].cbTable.eventSubHandler = NULL;
        dataElements[i].cbTable.methodHandler = NULL;
    }

    /* Register table-level element for wildcard queries */
    dataElements[TOTAL_PARAMS].name = TABLE_BASE;
    dataElements[TOTAL_PARAMS].type = RBUS_ELEMENT_TYPE_PROPERTY;
    dataElements[TOTAL_PARAMS].cbTable.getHandler = tableGetHandler;
    dataElements[TOTAL_PARAMS].cbTable.setHandler = NULL;
    dataElements[TOTAL_PARAMS].cbTable.tableAddRowHandler = NULL;
    dataElements[TOTAL_PARAMS].cbTable.tableRemoveRowHandler = NULL;
    dataElements[TOTAL_PARAMS].cbTable.eventSubHandler = NULL;
    dataElements[TOTAL_PARAMS].cbTable.methodHandler = NULL;

    rc = rbus_regDataElements(handle, TOTAL_PARAMS + 1, dataElements);
    if (rc != RBUS_ERROR_SUCCESS)
    {
        printf("mock_table_provider: rbus_regDataElements failed: %d\n", rc);
        rbus_close(handle);
        return 1;
    }

    printf("mock_table_provider: registered %d elements, running...\n", TOTAL_PARAMS + 1);

    signal(SIGINT, exitHandler);
    signal(SIGTERM, exitHandler);

    while (1)
    {
        sleep(5);
    }

    return 0;
}
