####################################################################################
# If not stated otherwise in this file or this component's Licenses
# following copyright and licenses apply:
#
# Copyright 2024 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
####################################################################################


Feature: Telemetry Protocol Layer - HTTP and RBUS Method Report Transmission

  Background:
    Given the telemetry daemon is running
    And report profiles are configured with destination endpoints

  # ============================================================================
  # HTTP Protocol - Curl Interface
  # ============================================================================

  Scenario: Initialize HTTP protocol with curl
    Given HTTP protocol is selected for report transmission
    When sendReportOverHTTP is called
    Then curl library should be initialized
    And curlFileMutex should be initialized via pthread_once
    And the system should be ready for HTTP POST operations

  Scenario: Set HTTP headers for report transmission
    Given a curl handle is initialized
    When setHeader function is called with destination URL
    Then CURLOPT_URL should be set to the destination endpoint
    And CURLOPT_SSLVERSION should be set to CURL_SSLVERSION_TLSv1_2
    And CURLOPT_CUSTOMREQUEST should be set to "POST"
    And CURLOPT_TIMEOUT should be set to 30 seconds
    And HTTP headers should include "Accept: application/json"
    And HTTP headers should include "Content-type: application/json"
    And CURLOPT_WRITEFUNCTION should be set to writeToFile callback
    And all curl_easy_setopt calls should be error-checked

  Scenario: Configure network interface for HTTP transmission
    Given the system has multiple network interfaces
    When setHeader configures the curl handle
    Then on RDKB devices, CURLOPT_INTERFACE should be set to "erouter0" by default
    And if WAN_FAILOVER_SUPPORTED is enabled, current WAN interface should be queried
    And TR-181 parameter "Device.X_RDK_WanManager.CurrentActiveInterface" should be read
    And if WAN interface is available, CURLOPT_INTERFACE should be updated
    And the configured interface should be used for HTTP requests

  Scenario: Set JSON payload for HTTP POST
    Given a telemetry report is ready for transmission
    When setPayload function is called
    Then CURLOPT_POSTFIELDS should be set to the JSON payload
    And CURLOPT_POSTFIELDSIZE should be set to payload length
    And payload should not be NULL
    And all setopt operations should be error-checked

  Scenario: Handle curl setopt failures
    Given curl_easy_setopt is called
    When any setopt operation returns non-CURLE_OK code
    Then the error code should be stored in childCurlResponse.curlSetopCode
    And the line number should be stored in childCurlResponse.lineNumber
    And T2ERROR_FAILURE should be returned
    And the curl handle should be cleaned up

  # ============================================================================
  # mTLS Support
  # ============================================================================

  Scenario: Check if mTLS is enabled
    Given a profile is configured for HTTP transmission
    When sendReportOverHTTP is called
    Then isMtlsEnabled() should be called to check mTLS status
    And if mTLS is enabled, certificate retrieval should be initiated
    And if mTLS is disabled, standard HTTP should be used

  Scenario: Configure mTLS headers without rdkcertselector
    Given mTLS is enabled
    And LIBRDKCERTSEL_BUILD is not defined
    When setMtlsHeaders is called
    Then CURLOPT_SSLENGINE_DEFAULT should be set to 1L
    And CURLOPT_SSLCERTTYPE should be set to "P12"
    And CURLOPT_SSLCERT should be set to certificate file path
    And CURLOPT_KEYPASSWD should be set to certificate password
    And CURLOPT_SSL_VERIFYPEER should be set to 1L for peer verification
    And all mTLS setopt operations should be error-checked

  Scenario: Initialize RDK certificate selector for mTLS
    Given LIBRDKCERTSEL_BUILD is enabled
    When curlCertSelectorInit is called
    Then if /tmp/stateRedEnabled exists, recovery cert selector should be initialized
    And rdkcertselector_new should be called with "RCVRY" tag for recovery mode
    And if state red is not enabled, standard cert selector should be initialized
    And rdkcertselector_new should be called with "MTLS" tag for standard mode
    And initialization status should be logged

  Scenario: Retrieve mTLS certificate using rdkcertselector
    Given rdkcertselector is initialized
    When certificate is needed for HTTPS connection
    Then rdkcertselector_getCert should be called to get certificate URI and password
    And if status is not certselectorOk, T2ERROR_FAILURE should be returned
    And certificate URI should be parsed to extract file path
    And "file://" scheme prefix should be stripped if present
    And certificate file path and password should be used for mTLS setup

  Scenario: Handle certificate retrieval failure
    Given rdkcertselector_getCert is called
    When certificate retrieval fails
    Then error should be logged
    And curlCertSelectorFree should be called to cleanup
    And curl handle should be cleaned up
    And child process should exit with failure

  Scenario: Set SSL engine for mTLS with rdkcertselector
    Given rdkcertselector is initialized
    When configuring curl for mTLS
    Then rdkcertselector_getEngine should be called
    And if engine is not NULL, CURLOPT_SSLENGINE should be set to engine name
    And if engine is NULL, CURLOPT_SSLENGINE_DEFAULT should be set to 1L
    And engine configuration should be error-checked

  Scenario: Retry certificate selection on connection failure
    Given rdkcertselector is being used
    When curl_easy_perform completes
    Then rdkcertselector_setCurlStatus should be called with curl code and URL
    And if status is TRY_ANOTHER, certificate retrieval should be retried
    And the retry loop should continue until success or no more certificates
    And each certificate should be attempted in sequence

  Scenario: Free certificate selector resources
    Given certificate selector is initialized
    When curlCertSelectorFree is called
    Then rdkcertselector_free should be called for curlCertSelector
    And rdkcertselector_free should be called for curlRcvryCertSelector
    And both selectors should be set to NULL
    And memory free status should be logged

  # ============================================================================
  # Process Forking for HTTP Transmission
  # ============================================================================

  Scenario: Fork child process for HTTP transmission
    Given a report is ready to send over HTTP
    When sendReportOverHTTP is called
    Then a pipe should be created for IPC between parent and child
    And user-defined signal handlers should be blocked before fork
    And fork() should be called to create child process
    And if fork fails, T2ERROR_FAILURE should be returned
    And signal handlers should be unblocked on fork failure

  Scenario: Child process executes curl operations
    Given fork succeeded and child process is created
    When child process executes
    Then curl_easy_init should be called to create curl handle
    And setHeader should configure URL, timeout, and headers
    And setPayload should configure JSON payload
    And if mTLS is enabled, setMtlsHeaders should configure certificates
    And curl_easy_perform should execute the HTTP POST request
    And curl_easy_getinfo should retrieve HTTP response code
    And child process should exit after curl operations complete

  Scenario: Write curl response to shared pipe
    Given child process completed curl operations
    When child process is ready to exit
    Then childCurlResponse structure should be populated with results
    And curlStatus, curlResponse, curlSetopCode should be set
    And http_code and lineNumber should be included
    And childCurlResponse should be written to sharedPipeFds[1]
    And if write fails, error should be logged to stderr
    And both pipe file descriptors should be closed
    And child process should call exit(0)

  Scenario: Parent process waits for child completion
    Given child process is executing curl operations
    When parent process continues after fork
    Then outForkedPid should be set to childPid if provided
    And waitpid should be called to wait for child process completion
    And user-defined signal handlers should be unblocked after wait
    And parent should read childCurlResponse from sharedPipeFds[0]
    And pipe file descriptors should be closed

  Scenario: Evaluate HTTP transmission success
    Given parent process received childCurlResponse from child
    When evaluating transmission success
    Then if http_code equals 200, transmission is successful
    And if curlResponse equals CURLE_OK, transmission is successful
    And if either condition is true, T2ERROR_SUCCESS should be returned
    And success message should be logged with HTTP code
    And if both conditions fail, T2ERROR_FAILURE should be returned

  Scenario: Log detailed curl response information
    Given child process completed and returned response
    When parent process logs the response
    Then childPid should be logged
    And curlSetopCode should be logged with curl_easy_strerror
    And curlResponse should be logged with curl_easy_strerror
    And http_code should be logged
    And lineNumber should be logged for debugging
    And all information should be logged at T2Info level

  Scenario: Handle fork failure
    Given sendReportOverHTTP is called
    When fork() returns negative value
    Then error "Failed to fork" should be logged
    And certificate resources should be freed if allocated
    And signal handlers should be unblocked
    And T2ERROR_FAILURE should be returned
    And no child process should be created

  Scenario: Memory cleanup in child process
    Given child process is exiting
    When cleanup is performed
    Then certificate file path should be freed if allocated
    And certificate password should be freed securely
    And if LIBRDKCONFIG_BUILD is enabled, rdkconfig_free should be used
    And curl handle should be cleaned up with curl_easy_cleanup
    And header list should be freed with curl_slist_free_all
    And all resources should be released before exit

  Scenario: Memory cleanup in parent process
    Given parent process received response from child
    When cleanup is performed
    Then certificate resources should be freed if allocated
    And certificate password should be freed securely
    And pipe file descriptors should be closed
    And memory cleanup should occur regardless of success or failure

  # ============================================================================
  # HTTP Output File Handling
  # ============================================================================

  Scenario: Write HTTP response to output file
    Given curl is configured for HTTP POST
    When curl_easy_perform is executed
    Then CURL_OUTPUT_FILE (/tmp/output.txt) should be opened in write-binary mode
    And CURLOPT_WRITEDATA should be set to file pointer
    And writeToFile callback should write response data to file
    And file should be closed after curl_easy_perform completes
    And curlFileMutex should protect file operations

  Scenario: Thread-safe file operations with mutex
    Given multiple profiles may send reports concurrently
    When HTTP transmission is in progress
    Then curlFileMutex should be acquired before opening output file
    And mutex should be held during curl_easy_perform
    And mutex should be released after file is closed
    And pthread_once should ensure mutex is initialized only once

  Scenario: Handle file open failure for HTTP output
    Given curl is ready to perform HTTP POST
    When fopen fails to open CURL_OUTPUT_FILE
    Then curl operations should not proceed
    And appropriate error handling should occur
    And mutex should be released if it was acquired

  # ============================================================================
  # Cached Report Transmission over HTTP
  # ============================================================================

  Scenario: Send cached reports over HTTP
    Given multiple reports are cached in a Vector
    When sendCachedReportsOverHTTP is called
    Then reports should be sent one at a time in order
    And for each report, sendReportOverHTTP should be called
    And if any transmission fails, remaining reports should stay in cache
    And successfully sent reports should be removed from Vector
    And report memory should be freed after removal

  Scenario: Handle failure in cached report transmission
    Given cached reports are being sent
    When sendReportOverHTTP returns T2ERROR_FAILURE
    Then error should be logged with remaining report count
    And transmission should stop immediately
    And T2ERROR_FAILURE should be returned
    And failed report should remain in cache for retry

  Scenario: Complete cached report transmission successfully
    Given all cached reports are sent successfully
    When sendCachedReportsOverHTTP completes
    Then Vector should be empty
    And T2ERROR_SUCCESS should be returned
    And all report memory should be freed

  # ============================================================================
  # RBUS Method Protocol
  # ============================================================================

  Scenario: Initialize RBUS method protocol
    Given RBUS_METHOD is selected for report transmission
    When sendReportsOverRBUSMethod is called
    Then rbusMethodMutex should be initialized via pthread_once
    And the system should be ready for RBUS method invocation

  Scenario: Validate RBUS method input parameters
    Given sendReportsOverRBUSMethod is called
    When input parameters are validated
    Then methodName should not be NULL
    And inputParams Vector should not be NULL
    And payload should not be NULL
    And if any parameter is NULL, T2ERROR_FAILURE should be returned

  Scenario: Build RBUS method input parameters
    Given a report is ready to send via RBUS method
    When input parameters are constructed
    Then rbusObject should be initialized
    And for each RBUSMethodParam in inputParams Vector
    Then rbusValue should be created with parameter value
    And rbusObject_SetValue should add parameter to object
    And rbusValue should be released after adding

  Scenario: Add default payload parameters to RBUS object
    Given RBUS input object is being constructed
    When default parameters are added
    Then "payloadlen" should be set to (strlen(payload) + 1)
    And payloadlen should be set as Int32 type
    And "payload" should be set to the JSON report string
    And payload should be set as String type
    And both values should be released after adding

  Scenario: Invoke RBUS method asynchronously
    Given RBUS input parameters are ready
    When rbusMethodCaller is invoked
    Then methodName should be passed as method to call
    And inParams object should be passed as input
    And payload should be passed for logging
    And asyncMethodHandler should be registered as callback
    And rbusMethodMutex should be locked before invocation

  Scenario: Handle RBUS method async callback
    Given RBUS method was invoked asynchronously
    When asyncMethodHandler is called by RBUS
    Then methodName and retStatus should be logged
    And if retStatus is RBUS_ERROR_SUCCESS, isRbusMethod should be set to true
    And if retStatus is not success, isRbusMethod should be set to false
    And error should be logged if method invocation failed
    And rbusMethodMutex should be unlocked in callback

  Scenario: Wait for RBUS method response with retry
    Given RBUS method was invoked
    When waiting for async response
    Then pthread_mutex_trylock should be attempted on rbusMethodMutex
    And if lock succeeds, isRbusMethod flag should be checked
    And if isRbusMethod is true, T2ERROR_SUCCESS should be returned
    And if isRbusMethod is false, T2ERROR_NO_RBUS_METHOD_PROVIDER should be returned
    And if lock fails, retry should occur after 2 second sleep

  Scenario: Handle maximum retry attempts for RBUS method
    Given RBUS method response is being awaited
    When MAX_RETRY_ATTEMPTS (5) is reached
    Then error "Max attempts reached for rbusmethodlock" should be logged
    And rbusMethodMutex should be unlocked
    And T2ERROR_NO_RBUS_METHOD_PROVIDER should be returned
    And total wait time should be approximately 10 seconds (5 retries × 2 seconds)

  Scenario: Cleanup RBUS method resources
    Given RBUS method invocation completed
    When cleanup is performed
    Then rbusMethodMutex should be unlocked
    And rbusObject should be released with rbusObject_Release
    And all rbusValue objects should have been released earlier
    And memory should be freed properly

  Scenario: Handle RBUS method caller failure
    Given rbusMethodCaller is invoked
    When rbusMethodCaller returns T2ERROR_FAILURE
    Then rbusMethodMutex should be unlocked immediately
    And no retry loop should be entered
    And T2ERROR_FAILURE should be returned
    And error should be logged

  # ============================================================================
  # Cached Report Transmission over RBUS Method
  # ============================================================================

  Scenario: Send cached reports over RBUS method
    Given multiple reports are cached in a Vector
    When sendCachedReportsOverRBUSMethod is called
    Then methodName and inputParams should be validated
    And reportList Vector should be validated
    And reports should be sent one at a time in order

  Scenario: Process each cached report via RBUS method
    Given cached reports are being sent
    When processing each report
    Then report should be retrieved from Vector at index 0
    And sendReportsOverRBUSMethod should be called with report payload
    And if transmission succeeds, report should be removed from Vector
    And report memory should be freed after removal
    And next report should be processed

  Scenario: Handle failure in cached RBUS method transmission
    Given cached reports are being sent via RBUS method
    When sendReportsOverRBUSMethod returns T2ERROR_FAILURE or T2ERROR_NO_RBUS_METHOD_PROVIDER
    Then error should be logged with remaining report count
    And transmission should stop immediately
    And T2ERROR_FAILURE should be returned
    And failed report should remain in cache

  Scenario: Complete cached RBUS method transmission successfully
    Given all cached reports are sent via RBUS method
    When sendCachedReportsOverRBUSMethod completes
    Then Vector should be empty
    And T2ERROR_SUCCESS should be returned
    And all report memory should be freed

  # ============================================================================
  # Protocol Selection and Integration
  # ============================================================================

  Scenario: Select HTTP protocol for report transmission
    Given a profile is configured with protocol "HTTP"
    When report is ready to send
    Then sendReportOverHTTP should be invoked
    And HTTP URL should be retrieved from profile configuration
    And JSON payload should be passed to HTTP sender
    And forked process should handle curl operations

  Scenario: Select RBUS_METHOD protocol for report transmission
    Given a profile is configured with protocol "RBUS_METHOD"
    When report is ready to send
    Then sendReportsOverRBUSMethod should be invoked
    And method name should be retrieved from profile configuration
    And input parameters should be constructed from profile
    And JSON payload should be passed to RBUS method sender

  Scenario: Handle unsupported protocol gracefully
    Given a profile is configured with unknown protocol
    When report transmission is attempted
    Then appropriate error should be logged
    And T2ERROR_FAILURE should be returned
    And report should be cached for retry if applicable

  # ============================================================================
  # Error Handling and Edge Cases
  # ============================================================================

  Scenario: Handle NULL URL in HTTP transmission
    Given sendReportOverHTTP is called
    When httpUrl parameter is NULL
    Then T2ERROR_FAILURE should be returned immediately
    And no curl operations should be performed
    And no fork should occur

  Scenario: Handle NULL payload in HTTP transmission
    Given sendReportOverHTTP is called
    When payload parameter is NULL
    Then T2ERROR_FAILURE should be returned immediately
    And no curl operations should be performed
    And no fork should occur

  Scenario: Handle NULL method name in RBUS method
    Given sendReportsOverRBUSMethod is called
    When methodName parameter is NULL
    Then T2ERROR_FAILURE should be returned immediately
    And no RBUS operations should be performed

  Scenario: Handle NULL input params in RBUS method
    Given sendReportsOverRBUSMethod is called
    When inputParams Vector is NULL
    Then T2ERROR_FAILURE should be returned immediately
    And no RBUS object should be constructed

  Scenario: Handle pipe creation failure
    Given sendReportOverHTTP is about to fork
    When pipe() system call fails
    Then error "Failed to create pipe" should be logged
    And T2ERROR_FAILURE should be returned
    And no fork should occur

  Scenario: Handle curl_easy_init failure
    Given child process is executing
    When curl_easy_init returns NULL
    Then childCurlResponse.curlStatus should be set to false
    And child process should exit with failure response
    And parent should receive failure status

  Scenario: Handle write to pipe failure in child
    Given child process completed curl operations
    When write to sharedPipeFds[1] fails
    Then error should be logged to stderr
    And error should be logged via T2Error
    And child process should still exit cleanly

  Scenario: Handle read from pipe failure in parent
    Given parent is waiting for child response
    When read from sharedPipeFds[0] fails
    Then error "unable to read from the pipe" should be logged
    And childCurlResponse may contain invalid data
    And appropriate error handling should occur

  Scenario: Handle HTTP response codes other than 200
    Given curl_easy_perform completed successfully
    When HTTP response code is not 200 (e.g., 404, 500)
    Then curl error should be logged to stderr
    And childCurlResponse.http_code should contain the error code
    And T2ERROR_FAILURE should be returned by parent
    And report may be cached for retry

  Scenario: Handle curl operation timeout
    Given curl is performing HTTP POST
    When operation exceeds TIMEOUT (30 seconds)
    Then curl should abort the operation
    And CURLE_OPERATION_TIMEDOUT should be returned
    And error should be logged
    And T2ERROR_FAILURE should be returned

  # ============================================================================
  # Security and Certificate Management
  # ============================================================================

  Scenario: Secure password handling with rdkconfig
    Given LIBRDKCONFIG_BUILD is enabled
    And certificate password is retrieved
    When password needs to be freed
    Then rdkconfig_free should be called instead of free()
    And password length should be passed to rdkconfig_free
    And if rdkconfig_free fails, T2ERROR_FAILURE should be returned
    And password memory should be securely cleared

  Scenario: Verify peer certificate in mTLS
    Given mTLS is enabled
    When curl is configured
    Then CURLOPT_SSL_VERIFYPEER should be set to 1L
    And peer certificate should be verified against CA
    And if verification fails, connection should be rejected
    And error should be logged

  Scenario: Use TLS 1.2 for secure communication
    Given HTTP transmission is configured
    When curl SSL version is set
    Then CURLOPT_SSLVERSION should be set to CURL_SSLVERSION_TLSv1_2
    And TLS 1.2 or higher should be used for connections
    And older TLS versions should not be allowed

  Scenario: Handle state red mode for certificate selection
    Given /tmp/stateRedEnabled file exists
    When certificate selector is initialized
    Then recovery certificate selector should be used
    And "RCVRY" tag should be passed to rdkcertselector_new
    And recovery certificates should be preferred over standard certificates

  # ============================================================================
  # Performance and Resource Management
  # ============================================================================

  Scenario: Fork to prevent memory leaks from OpenSSL
    Given OpenSSL has growing RSS that requires OPENSSL_cleanup
    And OPENSSL_cleanup is not thread-safe
    When HTTP transmission is needed
    Then libcurl calls should be forked to separate process
    And child process should execute and terminate
    And memory should be released per execution
    And parent process should not accumulate OpenSSL memory

  Scenario: Reuse curl handle within child process
    Given child process is created for HTTP transmission
    When curl operations are performed
    Then single curl handle should be used for all operations
    And curl_easy_init should be called once
    And curl_easy_cleanup should be called once before exit
    And handle should not be reused across fork boundaries

  Scenario: Minimize mutex contention for file operations
    Given multiple profiles may send reports
    When curlFileMutex is used
    Then mutex should be held only during file open, write, and close
    And mutex should not be held during curl_easy_perform
    And lock duration should be minimized for concurrency

  # ============================================================================
  # Logging and Debugging
  # ============================================================================

  Scenario: Log HTTP transmission details
    Given HTTP transmission is in progress
    When logging is performed
    Then destination URL should be logged (if debug enabled)
    And curl operation status should be logged
    And HTTP response code should be logged
    And curl error codes should be logged with curl_easy_strerror
    And line numbers should be logged for error tracing

  Scenario: Log RBUS method invocation details
    Given RBUS method is being invoked
    When logging is performed
    Then method name should be logged
    And payload should be logged at T2Info level
    And async callback status should be logged
    And retry attempts should be logged if applicable
    And final return status should be logged

  Scenario: Log certificate selector operations
    Given rdkcertselector is being used
    When certificate operations occur
    Then initialization status should be logged
    And certificate retrieval status should be logged
    And certificate name/path should be logged
    And state red mode status should be logged
    And memory free operations should be logged

  # ============================================================================
  # Integration with Report Generation
  # ============================================================================

  Scenario: Receive report from profile for HTTP transmission
    Given a profile generated a telemetry report
    When report is ready to send
    Then profile should call sendReportOverHTTP
    And HTTP URL should come from profile's T2HTTP destination
    And JSON payload should be the generated report
    And forked PID should be stored if needed for tracking

  Scenario: Receive report from profile for RBUS method transmission
    Given a profile generated a telemetry report
    When report is ready to send via RBUS method
    Then profile should call sendReportsOverRBUSMethod
    And method name should come from profile's RBUS_METHOD configuration
    And input parameters should be constructed from profile settings
    And JSON payload should be the generated report

  Scenario: Handle transmission success and update profile state
    Given report was sent successfully
    When transmission completes
    Then T2ERROR_SUCCESS should be returned to profile
    And profile should mark report as sent
    And cached report list should be cleared if applicable
    And next report interval should be scheduled

  Scenario: Handle transmission failure and cache report
    Given report transmission failed
    When T2ERROR_FAILURE is returned
    Then profile should cache the report
    And report should be added to cachedReportList
    And cached report should be retried with next successful transmission
    And profile should continue with next reporting interval