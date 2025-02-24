#include "producer.h"
#include <iostream>
#include <sstream>
#include <iomanip>  
#include <mutex>
#include <chrono>
#include <algorithm>

KafkaProducer::KafkaProducer(std::string brokers, std::string topic) : topic_(topic) {
    kafka_config_ = rd_kafka_conf_new();

    if (rd_kafka_conf_set(kafka_config_, "bootstrap.servers", brokers.c_str(), NULL, 0) != RD_KAFKA_CONF_OK) {
        std::cerr << "Error configuring Kafka brokers" << std::endl;
        exit(1);
    }

   if (rd_kafka_conf_set(kafka_config_, "linger.ms", "50", NULL, 0) != RD_KAFKA_CONF_OK) {
        std::cerr << "Error configuring linger.ms" << std::endl;
        exit(1);
    }

    if (rd_kafka_conf_set(kafka_config_, "batch.size", "10485760", NULL, 0) != RD_KAFKA_CONF_OK) {  // 256 KB
        std::cerr << "Error configuring batch.size" << std::endl;
        exit(1);
    }

    kafka_producer_ = rd_kafka_new(RD_KAFKA_PRODUCER, kafka_config_, NULL, 0);
    if (!kafka_producer_) {
        std::cerr << "Failed to create Kafka producer" << std::endl;
        exit(1);
    }

    kafka_topic_ = rd_kafka_topic_new(kafka_producer_, topic_.c_str(), NULL);
    if (!kafka_topic_) {
        std::cerr << "Failed to create Kafka topic: " << topic << std::endl;
        exit(1);
    }
}

KafkaProducer::~KafkaProducer() {
    rd_kafka_flush(kafka_producer_, 5);  
    rd_kafka_topic_destroy(kafka_topic_);
    rd_kafka_destroy(kafka_producer_);
}

bool KafkaProducer::sendMessage(std::string message, int thread_id) {
    auto begin = std::chrono::high_resolution_clock::now();
    auto begin_us = std::chrono::duration_cast<std::chrono::microseconds>(begin.time_since_epoch()).count();
    
    // std::stringstream log;
    // log << "[Thread " << thread_id << "] Timestamp: " << begin_us 
    //     << " - Sending message: " << message;

    int produce_status = rd_kafka_produce(
        kafka_topic_, 
        RD_KAFKA_PARTITION_UA,  // partition scheduling algorithm
        RD_KAFKA_MSG_F_COPY,    
        const_cast<char*>(message.c_str()), message.size(),
        NULL, 0,    //  hash key and hash key byte size
        NULL    // opaque pointer to user data used for kafka client callbacks
    );

    auto end = std::chrono::high_resolution_clock::now();
    auto end_us = std::chrono::duration_cast<std::chrono::microseconds>(end.time_since_epoch()).count();
    std::chrono::duration<double, std::micro> elapsed = end - begin; 
    // log << " Latency: " << elapsed.count() << std::endl;
    
    // if (produce_status == -1) {
    //     log << "[Thread " << thread_id << "] Error: Failed to produce message at " 
    //         << begin_us << " - " << rd_kafka_err2str(rd_kafka_last_error()) << std::endl;
    // }

    {
        std::lock_guard<std::mutex> lock(mutex);
        // logs.push_back({begin_us, log.str()});
        total_latency += elapsed.count();
        total_messages += 1;
    }

    rd_kafka_poll(kafka_producer_, 0);

    return produce_status != -1;
} 

void KafkaProducer::sortAndPrintLogs() {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto& pair : logs) {
        std::cout << pair.second << std::endl;
    }
}