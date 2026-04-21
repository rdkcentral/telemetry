
## Introduction

This container is intended to be used for faster development and integration testing of the application. It is not intended to be used in production.
It includes a pre-built version of the application and all the dependencies required to run the application.
All tools required to build the application are also included in the container.

A container that mocks the required XCONF services is also included as a separate container within the same docker network.
The mock XCONF services are used to simulate the XCONF services that are required by the application.
Service will be available at `http://mockxconf:50050` .

In the current version mock WEBCONFIG services are not included.
An alternative utility is provided to mock a very similar setter commands that will set the IPC name space `Device.X_RDKCENTRAL-COM_T2.ReportProfilesMsgPack` with the same format as the actual WEBCONFIG service.

Utilities to set multiprofiles in JSON format will also be included in the container in the next release.
Until next release please use the following command to set the multiprofiles in JSON format.

```bash

rbuscli setvalues Device.X_RDKCENTRAL-COM_T2.ReportProfiles string ''

```


## Prerequisites
A working installation of Docker and Docker Compose is required to run this container.
Host machine should have relevant credentials to clone dependant repositories from rdkcentral github.

## Steps to build the container
1. Clone the repository to the host machine.
2. Change directory to the cloned repository's sub-directory `containers`.
3. Run the following command to build the container:
```
./build.sh
```

## Steps to run the container and mock services

### Pre-requisites
1. Ensure there is a directory named L2_CONTAINER_SHARED_VOLUME crated in home directory of the host machine.
2. If user desires to mount a different directory, please update the `docker-compose.yaml` file with the desired path.
3. Both containers mount the same directory to share the data between them and is available at `/mnt/L2_CONTAINER_SHARED_VOLUME` directory in the container.

From within same location where compose.yaml is available, run the following command to start the container:
```
docker-compose up
```

## Logging inside individual application container
### Native Platform container
```
docker exec -it native-platform /bin/bash
```
### Webservices container
- The services are enabled with https support with selfsigned certificates.
- Selfsigned certificates are shared between the containers.
- Leveraging the capabilities of docker network, self-signed certificates are added to the trusted certificates of the container which makes the services accessible over https without any certificate warnings.


#### Services are available at 
`https://mockxconf:50051/dataLakeMock` ==> Data upload service
`https://mockxconf:50050/loguploader/getT2DCMSettings` ==> DCA settings service

- Data source for the services is available at `/etc/xconf/xconf-dcm-response.json` directory.

#### Login to the container using the following command:
```
docker exec -it mockxconf /bin/bash
```