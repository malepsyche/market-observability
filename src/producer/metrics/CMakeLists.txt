set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/producer_stats.proto)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
protobuf_generate_cpp(
    PROTO_SRCS PROTO_HDRS 
    ${PROTO_FILES}
)

find_package(gRPC REQUIRED)
include_directories(${gRPC_INCLUDE_DIRS})
protobuf_generate(
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PROTOS ${PROTO_FILES}
    OUT_VAR GRPC_SRCS
)

add_library(producer_metrics STATIC ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS})
target_link_libraries(producer_metrics protobuf::libprotobuf gRPC::grpc++ gRPC::grpc)
target_include_directories(producer_metrics PUBLIC ${CMAKE_CURRENT_BINARY_DIR})    

message("CMAKE CURRENT SOURCE DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
message("CMAKE CURRENT BINARY DIR: " ${CMAKE_CURRENT_BINARY_DIR})
message("Protobuf BIN: " ${PROTO_SRCS})
message("gRPC BIN: " ${GRPC_SRCS})
