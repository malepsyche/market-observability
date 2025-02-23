#include "producer.h"
#include <iostream>
using namespace std;

KafkaProducer::KafkaProducer(string brokers, string topic) : topic_(topic) {
    kafka_config_ = rd_kafka_conf_new();

    if (rd_kafka_conf_set(kafka_config_, "bootstrap.servers", brokers.c_str(), NULL, 0) != RD_KAFKA_CONF_OK) {
        cerr << "Error configuring Kafka brokers" << endl;
        exit(1);
    }

    kafka_producer_ = rd_kafka_new(RD_KAFKA_PRODUCER, kafka_config_, NULL, 0);
    if (!kafka_producer_) {
        cerr << "Failed to create Kafka producer" << endl;
        exit(1);
    }

    kafka_topic_ = rd_kafka_topic_new(kafka_producer_, topic_.c_str(), NULL);
    if (!kafka_topic_) {
        cerr << "Failed to create Kafka topic: " << topic << endl;
        exit(1);
    }
}

KafkaProducer::~KafkaProducer() {
    rd_kafka_topic_destroy(kafka_topic_);
    rd_kafka_destroy(kafka_producer_);
}

bool KafkaProducer::sendMessage(string message) {
    int produce_status = rd_kafka_produce(
        kafka_topic_, 
        RD_KAFKA_PARTITION_UA,  // partition scheduling algorithm
        RD_KAFKA_MSG_F_COPY,    
        const_cast<char*>(message.c_str()), message.size(),
        NULL, 0,    //  hash key and hash key byte size
        NULL    // opaque pointer to user data used for kafka client callbacks
    );

    if (produce_status == -1) {
        cerr << "Failed to produce message: " << rd_kafka_err2str(rd_kafka_last_error()) << endl;
        return false;
    }

    rd_kafka_poll(kafka_producer_, 0);
    return true;
} 