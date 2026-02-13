#!/bin/bash

# Monitor memory usage of curl_leak_test application
# Usage: ./monitor_memory.sh

LOG_FILE="memory_usage.log"
INTERVAL=5

echo "Monitoring curl_leak_test memory usage..."
echo "Results will be saved to $LOG_FILE"
echo "Press Ctrl+C to stop"
echo ""
echo "Timestamp,PID,VmSize(kB),VmRSS(kB)" > $LOG_FILE

while true; do
    PID=$(pgrep -f curl_leak_test)
    
    if [ -z "$PID" ]; then
        echo "curl_leak_test process not found. Waiting..."
        sleep $INTERVAL
        continue
    fi
    
    TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
    
    # Extract memory info from /proc/[pid]/status
    VMSIZE=$(grep "VmSize:" /proc/$PID/status | awk '{print $2}')
    VMRSS=$(grep "VmRSS:" /proc/$PID/status | awk '{print $2}')
    
    echo "$TIMESTAMP,$PID,$VMSIZE,$VMRSS" | tee -a $LOG_FILE
    
    sleep $INTERVAL
done
