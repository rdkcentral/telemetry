FROM ubuntu:jammy
ARG DEBIAN_FRONTEND=noninteractive
# Get the revision number from the build environment
# This must be a release tag derived from a github relase event context
ARG REVISION=0.0.0

# Add instructions to install autotools
RUN apt-get update && apt-get install -y build-essential \
          wget openssl tar vim git git-lfs


RUN apt-get install -y libtool autotools-dev automake zlib1g-dev ninja-build meson

RUN apt-get install -y libglib2.0-dev libcurl4-openssl-dev \
          libmsgpack-dev libsystemd-dev libssl-dev libcjson-dev python3-pip libsqlite3-dev
          
RUN apt-get install -y libgtest-dev libgmock-dev libjansson-dev libbsd-dev tcl-dev \
          libboost-all-dev libwebsocketpp-dev libcunit1 libcunit1-dev libunwind-dev

RUN apt-get update && apt-get install -y gdb valgrind lcov g++ wget gperf curl

# Add additional pytest packages 
RUN apt-get update && apt-get install -y \
    python3-pytest python3-pytest-cov software-properties-common

RUN add-apt-repository ppa:deadsnakes/ppa && apt-get update && apt-get install -y \
    python3-pytest-bdd
    
RUN pip3 install requests pytest-ordering pytest-json-report pytest-html msgpack base64 json

# Install CMake
RUN apt-get update && apt-get install -y cmake

# Install gtest libraries
RUN cd /usr/src/googletest/googlemock/ && mkdir build && cmake .. && make && make install

RUN apt-get update && apt-get install -y \
    liblog4c-dev

COPY ./mock-xconf-server-cert.pem /usr/share/ca-certificates/mock-xconf-server-cert.pem
RUN chmod 644 /usr/share/ca-certificates/mock-xconf-server-cert.pem
RUN echo "mock-xconf-server-cert.pem" >> /etc/ca-certificates.conf
RUN update-ca-certificates --fresh

# Build and install test binary that acts as a provider for all mandatory RFC parameters
RUN mkdir -p /opt 
COPY dependent_rdk_pkg_installer.sh /opt/dependent_rdk_pkg_installer.sh
RUN chmod +x /opt/dependent_rdk_pkg_installer.sh && /opt/dependent_rdk_pkg_installer.sh
RUN rm -rf /opt/dependent_rdk_pkg_installer.sh

COPY rdk_logger /opt/rdk_logger
RUN cd /opt/rdk_logger && autoreconf --install && ./configure && make && make install
RUN rm -rf /opt/rdk_logger
COPY debug.ini /etc/debug.ini
COPY log4crc /etc/log4crc

COPY WebconfigFramework /opt/WebconfigFramework
RUN cd /opt/WebconfigFramework && export INSTALL_DIR='/usr/local'&& \
export CFLAGS="-I${INSTALL_DIR}/include/rtmessage -I${INSTALL_DIR}/include/msgpack -I${INSTALL_DIR}/include/rbus -I${INSTALL_DIR}/include" && \
export LDFLAGS="-L${INSTALL_DIR}/lib" && \
autoreconf --install && \
./configure --prefix=/usr/local && make && make install && cp -r include/* /usr/local/include/
RUN rm -rf /opt/WebconfigFramework

# Mock implementation of RFC provider in target device
COPY  ./ /opt/containers
RUN cd /opt/containers/mock-rfc-providers && export INSTALL_DIR='/usr/local' && \
export CFLAGS="-I${INSTALL_DIR}/include/rtmessage -I${INSTALL_DIR}/include/msgpack -I${INSTALL_DIR}/include/rbus -I${INSTALL_DIR}/include" && \
export LDFLAGS="-L${INSTALL_DIR}/lib" && \
autoreconf --install && \
./configure --prefix=/usr/local && make && make install
RUN rm -rf /opt/containers

# Trim down the docker image size
RUN rm -rf /var/lib/apt/lists/*

# Emulate the device side settings
RUN mkdir -p /opt/logs
RUN mkdir -p /etc
RUN echo "BUILD_TYPE=PROD" > /etc/device.properties

RUN echo "DEVICE_NAME=DEVELOPER_CONTAINER" >> /etc/device.properties
RUN echo "imagename=T2_Container_${REVISION}" >> /version.txt

# Custom exepectation from RDK stack to have a timezone file in the stack
RUN mkdir -p /opt/persistent
RUN echo 'US/Mountain' > /opt/persistent/timeZoneDST

# Create A Shared Volume
RUN mkdir -p /mnt/L2_CONTAINER_SHARED_VOLUME

COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh
RUN echo 'export PATH=$PATH:/usr/local/bin' >> ~/.bashrc
RUN echo 'export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:/usr/local/lib:$LD_LIBRARY_PATH' >> ~/.bashrc

# Set the entry point command
#CMD ["/bin/bash"]

CMD ["/usr/local/bin/entrypoint.sh"]
