#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <librdkafka/rdkafka.h>
#include <string>
#include <mutex>
class KafkaProducer {
public:
    KafkaProducer(std::string brokers, std::string topic);
    ~KafkaProducer();
    
    bool sendMessage(std::string message, int thread_id);

private:
    std::string topic_;
    rd_kafka_t *kafka_producer_;
    rd_kafka_conf_t * kafka_config_;
    rd_kafka_topic_t * kafka_topic_;
    std::mutex mutex_;
};

#endif