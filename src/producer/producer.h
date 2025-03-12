#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <librdkafka/rdkafka.h>
#include <vector>
#include <string>
#include <mutex>
#include "producer_stats.pb.h"   
class KafkaProducer {
public:
    KafkaProducer(std::string brokers, std::string topic);
    ~KafkaProducer();
    
    double total_latency = 0;
    int total_messages = 0;
    bool sendMessage(std::string message, int thread_id);
    static int stats_cb (rd_kafka_t* kafka_producer, char* json, size_t json_len, void* opaque);
    static void sendStats(const ProducerStats& producer_stats);
    void sortAndPrintLogs();

private:
    std::string topic_;
    rd_kafka_t *kafka_producer_;
    rd_kafka_conf_t * kafka_config_;
    rd_kafka_topic_t * kafka_topic_;

    std::vector<std::pair<int, std::string>> logs; 
    std::mutex mutex; 
};

#endif