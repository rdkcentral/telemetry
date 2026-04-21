#!/bin/sh

#set -m

node /usr/local/bin/data-lake-mock.js &

#httpd-foreground
node /usr/local/bin/getT2DCMSettings.js &


## Keep the container running . Running an independent process will help in simulating scenarios of webservices going down and coming up
while true ; do echo "Mocked webservice heartbeat ..." && sleep 5 ; done