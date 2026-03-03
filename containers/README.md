
## Introduction

This container is intended to be used for faster development and integration testing of the application. It is not intended to be used in production.
It includes a pre-built version of the application and all the dependencies required to run the application.
All tools required to build the application are also included in the container.

This project reuses the existing dockers from https://github.com/rdkcentral/docker-device-mgt-service-test, which provides a base image with all the necessary tools and dependencies for building and testing the application.

- [native-platform](https://github.com/rdkcentral/docker-device-mgt-service-test/pkgs/container/docker-device-mgt-service-test%2Fnative-platform)

The application is built and tested inside the container using the existing build and test scripts.
Contaier images are built and pushed to Docker Hub as part of the CI/CD pipeline for the project. Images are available at : 

- [mockxconf](https://github.com/rdkcentral/docker-device-mgt-service-test/pkgs/container/docker-device-mgt-service-test%2Fmockxconf)

A container that mocks the required XCONF services is also included as a separate container within the same docker network.
The mock XCONF services are used to simulate the XCONF services that are required by the application.
Service will be available at `http://mockxconf:50050` .

In the current version mock WEBCONFIG services are not included.

Utilities to set multiprofiles in JSON format will also be included in the container in the next release.
Until next release please use the following command to set the multiprofiles in JSON format.

```bash

rbuscli setvalues Device.X_RDKCENTRAL-COM_T2.ReportProfiles string ''

```


## Prerequisites

A working installation of Docker and Docker Compose is required to run this container.
Host machine should have relevant credentials to clone dependant repositories from rdkcentral github.
