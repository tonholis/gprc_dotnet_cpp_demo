FROM ubuntu:19.10 as buildServer

ENV PKG_INSTALL_DIR=/opt/grpc
ENV GRPC_VER=v1.32.0
ENV CMAKE_VERSION=3.18.4

# Build tools
RUN apt-get update
RUN apt-get install -y \
	build-essential autoconf git pkg-config \
	automake libtool curl make g++ unzip wget \
	&& apt-get clean

RUN wget -q -O cmake-linux.sh https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-Linux-x86_64.sh \
	&& mkdir -p /opt/cmake && sh cmake-linux.sh --prefix=/opt/cmake --skip-license \
	&& rm cmake-linux.sh \
	&& ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

# Build gRPC
WORKDIR /usr/grpc_src
RUN git clone --recurse-submodules -b $GRPC_VER https://github.com/grpc/grpc.git .

WORKDIR /usr/grpc_build
RUN cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=$PKG_INSTALL_DIR /usr/grpc_src \
	&& make -j$(nproc) \
	&& make install \
	&& make clean \
	&& ldconfig

# Build our project
WORKDIR /app
COPY server/ ./
RUN mkdir build && cd build \
	&& cmake -DCMAKE_PREFIX_PATH=$PKG_INSTALL_DIR ../ \
	&& make -j

# Build our final docker image
FROM ubuntu:19.10 as final
WORKDIR /app
COPY --from=buildServer $PKG_INSTALL_DIR/ $PKG_INSTALL_DIR/
COPY --from=buildServer /app/build/server ./

CMD [ "./server" ]