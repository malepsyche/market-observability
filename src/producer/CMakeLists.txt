cmake_minimum_required (VERSION 3.7.2)
project(KafkaProducer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(/usr/include/librdkafka)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RDKAFKA REQUIRED rdkafka)

add_executable(kafka_producer main.cpp producer.cpp) 
add_subdirectory(market_fetcher)
add_subdirectory(metrics)
target_link_libraries(kafka_producer rdkafka tick producer_metrics)      

message("CMAKE CURRENT SOURCE DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
message("CMAKE CURRENT BINARY DIR: " ${CMAKE_CURRENT_BINARY_DIR})
