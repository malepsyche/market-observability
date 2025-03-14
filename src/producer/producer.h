#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <librdkafka/rdkafka.h>
#include <vector>
#include <string>
#include <mutex>
#include "producer_stats.pb.h" 
#include "tick.pb.h"  
class KafkaProducer {
public:
    KafkaProducer(std::string brokers, std::string topic);
    ~KafkaProducer();
    
    double total_latency = 0;
    int total_messages = 0;
    void fetch_data(int thread_id, const std::string& api_key, std::function<void(std::string)> callback);
    void fetch_data(int thread_id);
    bool send_broker(const market_fetcher::Tick& tick, int thread_id);
    bool send_broker(std::string message, int thread_id);
    static int stats_cb (rd_kafka_t* kafka_producer, char* json, size_t json_len, void* opaque);
    static void send_stats(const metrics::ProducerStats& producer_stats);
    void sort_and_print_logs();

private:
    std::string topic_;
    rd_kafka_t *kafka_producer_;
    rd_kafka_conf_t * kafka_config_;
    rd_kafka_topic_t * kafka_topic_;

    std::vector<std::pair<int, std::string>> logs; 
    std::mutex mutex; 
};

#endif