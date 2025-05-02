+-------------------------------------------------------------+
|                         Telemetry System                    |
+-------------------------------------------------------------+
|                                                             |
|  +----------------------+                                   |
|  | Telemetry Daemon     |                                   |
|  | (source/telemetry2_0.c)                                  |
|  | - Main entry point for telemetry operations              |
|  | - Initializes components and handles signals             |
|  +----------------------+                                   |
|                                                             |
|  +----------------------+                                   |
|  | Data Collection From Files  |                            |
|  | (source/dcautil/)                                        |
|  | - Collects system metrics (CPU, memory, logs, etc.)      |
|  | - Processes patterns and generates JSON data             |
|  +----------------------+                                   |
|                                                             |
|  +----------------------+                                   |
|  | Report Generation    |                                   |
|  | (source/reportgen/)                                      |
|  | - Encodes telemetry data into JSON or other formats      |
|  | - Prepares reports for upload                            |
|  +----------------------+                                   |
|                                                             |
|  +----------------------+                                   |
|  | Scheduler            |                                   |
|  | (source/scheduler/)                                      |
|  | - Manages periodic tasks and report generation schedules |
|  +----------------------+                                   |
|                                                             |
|  +----------------------+                                   |
|  | Report Profile Handling   |                              |
|  | (source/bulkdata/)                                       |
|  | - Processes large telemetry profiles                     |
|  | - Handles persistent storage of profiles                 |
|  +----------------------+                                   |
|                                                             |
|  +----------------------+                                   |
|  | XCONF Config Fetch  |                                    |
|  | (source/xconfclient/)                                    |
|  | - Fetches configuration from XCONF server                |
|  +------------------------+                                 |
|                                                             |
|  +----------------------+                                   |
|  | Report Profile Parsers  |                                |
|  | (source/reportparser/)                                   |
|  | - Parses telemetry profiles and extracts data            |
|  | - Validates and processes profile data                   |
|  +----------------------+                                   |
|                                                             |
|  +----------------------+                                   |
|  | CCSP Interface       |                                   |
|  | (source/ccspinterface/)                                  |
|  | - Communicates with the system via bus (rbus)            |
|  | - Discovers and interacts with remote components         |
|  +----------------------+                                   |
|                                                             |
|  +----------------------+                                   |
|  | Report upload protocols     |                            |
|  | (source/protocol )                                       |
|  | - Sends telemetry data over HTTP or RBUS Method          |
|  +----------------------+                                   |
|                                                             |
|  +----------------------+                                   |
|  | Generic Utilities    |                                   |
|  | (source/utils/)                                          |
|  | - Provides common utility functions                      |
|  | - Handles logging, error reporting, and other tasks      |
|  | - Implements generic DSA functions for data handling     |
|  +----------------------+                                   |
|                                                             |
|  +----------------------+                                   |
|  | Test Framework       |                                   |
|  | (source/test/)                                           |
|  | - Unit tests for various modules                         |
|  | - Mock implementations for testing                       |
|  +----------------------+                                   |
|                                                             |
+-------------------------------------------------------------+