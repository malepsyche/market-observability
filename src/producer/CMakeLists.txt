cmake_minimum_required (VERSION 3.7.2)
project(KafkaProducer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(/usr/include/librdkafka)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RDKAFKA REQUIRED rdkafka)

set(PROTO_FILES ${CMAKE_SOURCE_DIR}/producer_stats.proto)
include_directories(${CMAKE_SOURCE_DIR})

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

find_package(gRPC REQUIRED)
include_directories(${gRPC_INCLUDE_DIRS})
protobuf_generate(
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PROTOS ${PROTO_FILES}
    OUT_VAR GRPC_SRCS
)

add_executable(kafka_producer main.cpp producer.cpp ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS})   
target_link_libraries(kafka_producer rdkafka protobuf::libprotobuf gRPC::grpc++ gRPC::grpc)     

message("CMAKE SOURCE DIR: " ${CMAKE_SOURCE_DIR})
message("CMAKE BINARY DIR: " ${CMAKE_BINARY_DIR})
message("Protobuf BIN: " ${PROTO_SRCS})
message("gRPC BIN: " ${GRPC_SRCS})
