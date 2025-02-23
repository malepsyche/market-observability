#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <librdkafka/rdkafka.h>
#include <string>
using namespace std;

class KafkaProducer {
public:
    KafkaProducer(string brokers, string topic);
    ~KafkaProducer();
    
    bool sendMessage(string message);

private:
    string topic_;
    rd_kafka_t *kafka_producer_;
    rd_kafka_conf_t * kafka_config_;
    rd_kafka_topic_t * kafka_topic_;
};

#endif