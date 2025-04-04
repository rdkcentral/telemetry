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
#include "telemetry2_0.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <curl/curl.h>
#ifdef __GNUC__
#ifndef _BUILD_ANDROID
#ifdef __GLIBC__
#include <execinfo.h>
#endif
#endif
#endif
#include "t2log_wrapper.h"
#include "syslog.h"
#include "reportprofiles.h"
#include "xconfclient.h"
#include "scheduler.h"
#include "t2eventreceiver.h"

#ifdef INCLUDE_BREAKPAD
#ifndef ENABLE_RDKC_SUPPORT
#include "breakpad_wrapper.h"
#else
#include "breakpadwrap.h"
#endif
#endif

#define MAX_PARAMETERNAME_LEN    512
/*Define signals properly to make sure they don't get overide anywhere*/
#define LOG_UPLOAD 10
#define EXEC_RELOAD 12
#define LOG_UPLOAD_ONDEMAND 29

sigset_t blocking_signal;

static bool isDebugEnabled = true;
static int initcomplete = 0;
static pid_t DAEMONPID; //static varible store the Main Pid

T2ERROR initTelemetry()
{
    T2ERROR ret = T2ERROR_FAILURE;
    T2Debug("%s ++in\n", __FUNCTION__);

    if(T2ERROR_SUCCESS == initReportProfiles())
    {
#ifndef DEVICE_EXTENDER
        if(T2ERROR_SUCCESS == initXConfClient())
        {
            ret = T2ERROR_SUCCESS;
            generateDcaReport(true, false);
            T2Debug("%s --out\n", __FUNCTION__);
        }
        else
        {
            T2Error("Failed to initializeXConfClient\n");
        }
#endif
#if defined(DEVICE_EXTENDER)
        ret = T2ERROR_SUCCESS;
#endif
    }
    else
    {
        T2Error("Failed to initialize ReportProfiles\n");
    }

    initcomplete = 1;

    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}


static void terminate()
{
    if(remove("/tmp/.t2ReadyToReceiveEvents") != 0)
    {
        printf("removing the file /tmp/.t2ReadyToReceiveEvents failed!\n");
    }

    if(remove("/tmp/telemetry_initialized_bootup") != 0)
    {
        printf("removing the file /tmp/telemetry_initialized_bootup failed!\n");
    }

    if(remove(T2_CONFIG_READY) != 0)
    {
        printf("removing the file T2_CONFIG_READY failed!\n");
    }
    if (initcomplete)
    {
#ifndef DEVICE_EXTENDER
        uninitXConfClient();
#endif
        ReportProfiles_uninit();
    }

}
static void _print_stack_backtrace(void)
{
#ifdef __GNUC__
#ifndef _BUILD_ANDROID
#ifdef __GLIBC__
    void* tracePtrs[100];
    char** funcNames = NULL;
    int i, count = 0;

    count = backtrace( tracePtrs, 100 );
    backtrace_symbols_fd( tracePtrs, count, 2 );

    funcNames = backtrace_symbols( tracePtrs, count );

    if ( funcNames )
    {
        // Print the stack trace
        for( i = 0; i < count; i++ )
        {
            printf("%s\n", funcNames[i] );
        }

        // Free the string pointers
        free( funcNames );
    }
#endif
#endif
#endif
}

void sig_handler(int sig, siginfo_t* info, void* uc)
{
    if(DAEMONPID == getpid())
    {
        int fd;
        const char *path = "/tmp/telemetry_logupload";
        if ( sig == SIGINT )
        {
            T2Info(("SIGINT received!\n"));
#ifndef DEVICE_EXTENDER
            uninitXConfClient();
#endif
            ReportProfiles_uninit();
            exit(0);
        }
        else if ( sig == SIGUSR1 || sig == LOG_UPLOAD )
        {
            T2Info(("LOG_UPLOAD received!\n"));
            set_logdemand(false);
            ReportProfiles_Interrupt();
        }
        else if (sig == LOG_UPLOAD_ONDEMAND || sig == SIGIO)
        {
            T2Info(("LOG_UPLOAD_ONDEMAND received!\n"));
            set_logdemand(true);
            ReportProfiles_Interrupt();
        }
        else if(sig == SIGUSR2 || sig == EXEC_RELOAD)
        {
            T2Info(("EXEC_RELOAD received!\n"));
            fd = open(path, O_RDONLY | O_CREAT, 0400);
            if(fd  == -1)
            {
                T2Warning("Failed to open the file\n");
            }
            else
            {
                T2Debug("File is created\n");
                close(fd);
            }
#ifndef DEVICE_EXTENDER
            stopXConfClient();
            if(T2ERROR_SUCCESS == startXConfClient())
            {
                T2Info("XCONF config reload - SUCCESS \n");
            }
            else
            {
                T2Info("XCONF config reload - IN PROGRESS ... Ignore current reload request \n");
            }
#endif

        }
        else if ( sig == SIGTERM || sig == SIGKILL )
        {
            terminate();
            exit(0);
        }
        else
        {
            /* get stack trace first */
            _print_stack_backtrace();
            terminate();
            T2Warning("[%s][%u] Signal number: %d\n", __FUNCTION__, __LINE__, info->si_signo);
            T2Warning("[%s][%u] Signal error: %d\n", __FUNCTION__, __LINE__, info->si_errno);
            T2Warning("[%s][%u] Signal code: %d\n", __FUNCTION__, __LINE__, info->si_code);
            T2Warning("[%s][%u] Signal value: %d\n", __FUNCTION__, __LINE__, info->si_value.sival_int);
            (void)uc;
            exit(0);
        }
    }
    else
    {
        // This logic is added to terminate child imediately if we get terminate signals eg:SIGTERM / SIGKILL ...
        if(!(sig == SIGUSR1 || sig == LOG_UPLOAD || sig == LOG_UPLOAD_ONDEMAND || sig == SIGIO || sig == SIGCHLD || sig == SIGPIPE || sig == SIGUSR2 || sig == EXEC_RELOAD || sig == SIGALRM ))
        {
            exit(0);
        }
    }
}

static void t2DaemonMainModeInit( )
{

    /**
     * Signal handling is being used as way to handle log uploads . Double check whether we get minidump events for crashes
     */
#ifdef INCLUDE_BREAKPAD
#ifndef ENABLE_RDKC_SUPPORT
    breakpad_ExceptionHandler();
#else
    BreakPadWrapExceptionHandler eh;
    eh = newBreakPadWrapExceptionHandler();
#endif
#endif
    /**
    * Create a Signal Mask for signals that need to be blocked while using fork
    */
    struct sigaction act;
    memset (&act, 0, sizeof(act));
    act.sa_sigaction = sig_handler;
    act.sa_flags = SA_ONSTACK | SA_SIGINFO ;

    sigaddset(&blocking_signal, SIGUSR2);
    sigaddset(&blocking_signal, SIGUSR1);
    sigaddset(&blocking_signal, LOG_UPLOAD);
    sigaddset(&blocking_signal, EXEC_RELOAD);
    sigaddset(&blocking_signal, LOG_UPLOAD_ONDEMAND);
    sigaddset(&blocking_signal, SIGIO);

    DAEMONPID = getpid(); // save the pid of the deamon
    T2Debug("Telemetry 2.0 Process PID %d\n", (int)DAEMONPID); //Debug line

    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(LOG_UPLOAD, &act, NULL);
    sigaction(EXEC_RELOAD, &act, NULL);
    sigaction(LOG_UPLOAD_ONDEMAND, &act, NULL);
    sigaction(SIGIO, &act, NULL);

    if(T2ERROR_SUCCESS != initTelemetry())
    {
        T2Error("Failed to initialize Telemetry.. exiting the process\n");
        exit(0);
    }


    T2Info("Telemetry 2.0 Component Init Success\n");

    while(1)
    {
        sleep(30);
    }
    T2Info("Telemetry 2.0 Process Terminated\n");
}


static int checkAnotherTelemetryInstance (void)
{
    int fd;

    fd = open("/var/run/telemetry2_0.lock", O_CREAT | O_RDWR, 0666);

    if (fd == -1)
    {
        T2Error("Failed to open lock file\n");
        return 0;
    }

    if (flock(fd, LOCK_EX | LOCK_NB) != 0)
    {
        T2Error("Failed to acquire lock file\n");
        close(fd);
        return 1;
    }

    /* OK to proceed (lock will be released and file descriptor will be closed on exit) */

    return 0;
}

int main()
{
    pid_t process_id = 0;
    pid_t sid = 0;
    LOGInit();

    /* Abort if another instance of telemetry2_0 is already running */
    if (checkAnotherTelemetryInstance())
    {
        return 1;
    }

    T2Info("Starting Telemetry 2.0 Process\n");

    // Create child process
    process_id = fork();
    if (process_id < 0)
    {
        T2Error("fork failed!\n");
        return 1;
    }
    else if (process_id > 0)
    {
        return 0;
    }

    //unmask the file mode
    umask(0);

    //set new session
    sid = setsid();
    if (sid < 0)
    {
        T2Error("setsid failed!\n");
        return 1;
    }

    // Change the current working directory to root.
    if (chdir("/") < 0)
    {
        T2Error("chdir failed!\n");
        return 1;
    }

    if (isDebugEnabled != true)
    {
        // Close stdin. stdout and stderr
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }


    T2Info("Initializing Telemetry 2.0 Component\n");
    t2DaemonMainModeInit();
    return 0;
}
