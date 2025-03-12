FROM debian
RUN apt-get update && apt-get install -y \
    git \
    g++ \
    make \
    cmake \
    libc6-dev \
    autoconf \
    libtool \
    pkg-config \
    libboost-system-dev \
    nlohmann-json3-dev \
    libutf8proc-dev \
    protobuf-compiler \
    libprotobuf-dev \
    grpc-proto \
    libgrpc-dev \
    libgrpc++-dev \
    protobuf-compiler-grpc
RUN ln -s $(which grpc_cpp_plugin) /usr/bin/protoc-gen-grpc
WORKDIR /app
RUN git clone --depth 1 --branch v2.2.0 https://github.com/confluentinc/librdkafka.git && \
    cd librdkafka && \
    ./configure && make -j$(nproc) && \
    make install

# RUN git clone -b v30.0 https://github.com/protocolbuffers/protobuf.git && \
#     cd protobuf && mkdir build && cd build && \
#     cmake ../ \
#         -DCMAKE_INSTALL_PREFIX=/usr/local \
#         -Dprotobuf_BUILD_TESTS=OFF \
#         -Dprotobuf_BUILD_SHARED_LIBS=OFF \
#         -DgRPC_ABSL_PROVIDER=module && \
#     make -j$(nproc) && make install
# RUN ln -s /usr/local/bin/protoc /usr/bin/protoc

# RUN git clone --recurse-submodules -b v1.70.0 --depth 1 https://github.com/grpc/grpc.git && \
#     cd grpc && git submodule update --init && mkdir -p cmake/build && cd cmake/build && \
#     cmake ../.. \
#         -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_CXX_STANDARD=17 \
#         -DgRPC_INSTALL=ON \
#         -DgRPC_BUILD_TESTS=OFF \
#         -DgRPC_SSL_PROVIDER=module && \ 
#     make -j$(nproc) && make install
# RUN which grpc_cpp_plugin && echo "grpc_cpp_plugin location: $(which grpc_cpp_plugin)"
# RUN ln -s $(which grpc_cpp_plugin) /usr/bin/protoc-gen-grpc

RUN ldconfig 
RUN cp /app/librdkafka/examples/rdkafka_performance /usr/local/bin/
COPY . /app
RUN cmake . -DCMAKE_VERBOSE_MAKEFILE=ON && make
CMD ["./kafka_producer"]



# FROM ubuntu:22.04 AS librdkafka-builder
# RUN apt-get update && apt-get install -y git g++ make cmake autoconf libtool pkg-config
# WORKDIR /deps
# RUN git clone --depth 1 --branch v2.2.0 https://github.com/confluentinc/librdkafka.git && \
#     cd librdkafka && \
#     ./configure && make -j$(nproc) && \
#     make install

# FROM ubuntu:22.04 AS protobuf-builder
# RUN apt-get update && apt-get install -y git g++ make cmake
# WORKDIR /deps
# RUN git clone -b v30.0 https://github.com/protocolbuffers/protobuf.git && \
#     cd protobuf && mkdir build && cd build && \
#     cmake ../ \
#         -DCMAKE_INSTALL_PREFIX=/usr/local \
#         -Dprotobuf_BUILD_TESTS=OFF \
#         -Dprotobuf_BUILD_SHARED_LIBS=OFF \
#         -DgRPC_ABSL_PROVIDER=module && \
#     make -j$(nproc) && make install
# RUN ln -s /usr/local/bin/protoc /usr/bin/protoc

# FROM ubuntu:22.04 AS grpc-builder
# RUN apt-get update && apt-get install -y git g++ make cmake autoconf libtool pkg-config
# WORKDIR /deps
# RUN git clone --recurse-submodules -b v1.70.0 --depth 1 https://github.com/grpc/grpc.git && \
#     cd grpc && git submodule update --init && mkdir -p cmake/build && cd cmake/build && \
#     cmake ../.. \
#         -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_CXX_STANDARD=17 \
#         -DgRPC_INSTALL=ON \
#         -DgRPC_BUILD_TESTS=OFF \
#         -DgRPC_SSL_PROVIDER=module \
#         -DgRPC_ABSL_PROVIDER=module \
#         -DgRPC_PROTOBUF_PROVIDER=module && \ 
#     make -j$(nproc) && make install
# RUN which grpc_cpp_plugin && echo "grpc_cpp_plugin location: $(which grpc_cpp_plugin)"
# RUN ln -s $(which grpc_cpp_plugin) /usr/bin/protoc-gen-grpc

# # FROM ubuntu:22.04 AS abseil-builder
# # RUN apt-get update && apt-get install -y git g++ make cmake autoconf libtool pkg-config
# # WORKDIR /deps
# # RUN git clone https://github.com/abseil/abseil-cpp.git && \
# #     cd abseil-cpp && git submodule update --init && mkdir -p cmake/build && cd cmake/build && \
# #     cmake ../.. \
# #         -DABSL_BUILD_TESTING=ON \
# #         -DABSL_USE_GOOGLETEST_HEAD=ON \
# #         -DCMAKE_CXX_STANDARD=17 && \
# #     make -j$(nproc) && make install

# FROM ubuntu:22.04 AS app-builder 
# RUN apt-get update && apt-get install -y g++ make cmake pkg-config libc6-dev libboost-system-dev nlohmann-json3-dev libutf8proc-dev
# WORKDIR /app
# COPY --from=librdkafka-builder /usr/local /usr/local
# COPY --from=protobuf-builder /usr/local /usr/local
# COPY --from=grpc-builder /usr/local /usr/local
# # COPY --from=abseil-builder /usr/local /usr/local
# RUN ldconfig
# COPY . /app
# RUN rm -rf /usr/share/cmake-3.22/Modules/FindProtobuf.cmake
# RUN cmake . -DCMAKE_VERBOSE_MAKEFILE=ON && make

# FROM ubuntu:22.04
# RUN apt-get update && apt-get install -y libc6 libboost-system1.74.0 libutf8proc2
# WORKDIR /app
# COPY --from=app-builder /app/kafka_producer /app/kafka_producer
# COPY --from=app-builder /usr/local /usr/local
# RUN ldconfig
# CMD ["./kafka_producer"]


