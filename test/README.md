
## Testing Instructions

All L1 (Unit tests) and L2 (Integration tests) tests for the application are included in this container.
Tests are built and run inside the container using the existing build and test scripts. 
Test results are printed to the console and can also be saved to a file for further analysis.

## Running L1 Tests
To run the tests, follow these steps:
````bash
# Clone the repository and navigate to the test directory
git clone <repository_url>
cd telemetry
# Build and run tests inside the container using the provided script

docker run -d --name l1-final-check --platform linux/amd64 -v "$PWD:/mnt/workspace" ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
docker exec -i l1-final-check /bin/bash -c "cd /mnt/workspace && sh test/run_ut.sh 2>&1" | tee /tmp/l1_test_output.log

# Once tests are complete, you can stop and remove the container
docker stop l1-final-check
docker rm l1-final-check

````

This will run all the unit tests and print the results to the console. The test results will also be saved to `/tmp/l1_test_output.log` for further analysis.

## Running L2 Tests
To run the integration tests, follow these steps:
````bash
# Start the mock XCONF container
docker run -d --name mockxconf --platform linux/amd64 ghcr.io/rdkcentral/docker-device-mgt-service-test/mockxconf:latest
# Start the main test container and link it to the mock XCONF container
docker run -d --name l2-final-check --platform linux/amd64 --link mockxconf:mockxconf -v "$PWD:/mnt/workspace" ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
# Run the integration tests inside the main test container
docker exec -i l2-final-check /bin/bash -c "cd /mnt/workspace && sh test/run_it.sh 2>&1" | tee /tmp/l2_test_output.log