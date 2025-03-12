#include "producer.h"
#include <iostream>
#include <sstream>
#include <iomanip>  
#include <mutex>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>  
#include "producer_stats.pb.h" 
#include "producer_stats.grpc.pb.h" 
#include <grpcpp/grpcpp.h>   

KafkaProducer::KafkaProducer(std::string brokers, std::string topic) : topic_(topic) {
    kafka_config_ = rd_kafka_conf_new();

    if (rd_kafka_conf_set(kafka_config_, "bootstrap.servers", brokers.c_str(), NULL, 0) != RD_KAFKA_CONF_OK) {
        std::cerr << "Error configuring Kafka brokers" << std::endl;
        exit(1);
    }

   if (rd_kafka_conf_set(kafka_config_, "linger.ms", "0", NULL, 0) != RD_KAFKA_CONF_OK) {
        std::cerr << "Error configuring linger.ms" << std::endl;
        exit(1);
    }

    if (rd_kafka_conf_set(kafka_config_, "batch.size", "10485760", NULL, 0) != RD_KAFKA_CONF_OK) {  // 256 KB
        std::cerr << "Error configuring batch.size" << std::endl;
        exit(1);
    }

    if (rd_kafka_conf_set(kafka_config_, "statistics.interval.ms", "10000", NULL, 0) != RD_KAFKA_CONF_OK) {
        std::cerr << "Error configuring statistics.interval.ms" << std::endl;
        exit(1);    
    }

    rd_kafka_conf_set_stats_cb(kafka_config_, stats_cb);
    rd_kafka_conf_set_opaque(kafka_config_, this);

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

// int KafkaProducer::stats_cb (rd_kafka_t* kafka_producer, char* json, size_t json_len, void* opaque) {
//     std::ofstream log_file("./metrics/producer_stats.json", std::ios::app);
//     if (log_file.is_open()) {
//         log_file << std::string(json, json_len) << std::endl;
//         log_file.close();
//     } else {
//         std::cerr << "Error opening stats log file" << std::endl;
//         return -1;
//     }
//     return 0;
// }

int KafkaProducer::stats_cb (rd_kafka_t* kafka_producer, char* json, size_t json_len, void* opaque) {
    KafkaProducer* producer = static_cast<KafkaProducer*>(opaque);
    if (!producer) {
        std::cerr << "[Kafka] stats_cb: Producer instance is NULL!" << std::endl;
        return -1;
    }    
    std::string json_copy(json, json_len);
    std::thread([producer, json_copy = std::move(json_copy)]() mutable {  
        try {
            nlohmann::json stats_json = nlohmann::json::parse(json_copy);
            
            ProducerStats producer_stats;
            producer_stats.set_name(stats_json["name"]);
            producer_stats.set_client_id(stats_json["client_id"]);
            producer_stats.set_type(stats_json["type"]);
            producer_stats.set_age(stats_json["age"]);
            producer_stats.set_msg_cnt(stats_json["msg_cnt"]);
            producer_stats.set_tx(stats_json["tx"]);
            producer_stats.set_rx(stats_json["rx"]);
            producer_stats.set_txmsgs(stats_json["txmsgs"]);

            for (const auto& [broker_name, broker_data] : stats_json["brokers"].items()) {
                BrokerStats* broker_stats = producer_stats.add_brokers();
                broker_stats->set_name(broker_name);
                broker_stats->set_source(broker_data["source"]);
                broker_stats->set_state(broker_data["state"]);
                broker_stats->set_outbuf_cnt(broker_data["outbuf_cnt"]);
                broker_stats->set_outbuf_msg_cnt(broker_data["outbuf_msg_cnt"]);
                broker_stats->set_tx(broker_data["tx"]);
                broker_stats->set_txerrs(broker_data["txerrs"]);
                broker_stats->set_rx(broker_data["rx"]);
                broker_stats->set_rxerrs(broker_data["rxerrs"]);
                broker_stats->set_connects(broker_data["connects"]);
                broker_stats->set_disconnects(broker_data["disconnects"]);
                auto int_latency = broker_stats->mutable_int_latency();
                int_latency->set_avg(broker_data["int_latency"]["avg"]);
                int_latency->set_p99(broker_data["int_latency"]["p99"]);
                auto outbuf_latency = broker_stats->mutable_outbuf_latency();
                outbuf_latency->set_avg(broker_data["outbuf_latency"]["avg"]);
                outbuf_latency->set_p99(broker_data["outbuf_latency"]["p99"]);
                auto rtt = broker_stats->mutable_rtt();
                rtt->set_avg(broker_data["rtt"]["avg"]);
                rtt->set_p99(broker_data["rtt"]["p99"]); 
            }

            for (const auto& [topic_name, topic_data] : stats_json["topics"].items()) {
                TopicStats* topic_stats = producer_stats.add_topics();
                topic_stats->set_topic(topic_name);
                auto batch_size = topic_stats->mutable_batch_size();
                batch_size->set_avg(topic_data["batchsize"]["avg"]);
                batch_size->set_p99(topic_data["batchsize"]["p99"]);
                auto batch_cnt = topic_stats->mutable_batch_cnt();
                batch_cnt->set_avg(topic_data["batchcnt"]["avg"]);
                batch_cnt->set_p99(topic_data["batchcnt"]["p99"]);
                
                for (const auto& [partition_id, partition_data] : topic_data["partitions"].items()) {
                    PartitionStats* partition_stats = topic_stats->add_partitions();
                    partition_stats->set_partition(std::stoi(partition_id));
                    partition_stats->set_broker(partition_data["broker"]);
                    partition_stats->set_leader(partition_data["leader"]);
                    partition_stats->set_msgq_cnt(partition_data["msgq_cnt"]);
                    partition_stats->set_txmsgs(partition_data["txmsgs"]);
                    partition_stats->set_rxmsgs(partition_data["rxmsgs"]);
                }
            }
            producer->sendStats(producer_stats);

        } catch (const std::exception& e) {
            std::cerr << "Error parsing Kafka stats JSON: " << e.what() << std::endl;
            return -1;
        }
    }).detach();  
    return 0;
}

void KafkaProducer::sendStats(const ProducerStats& producer_stats) {
    auto channel = grpc::CreateChannel("kafka_process_metrics:50051", grpc::InsecureChannelCredentials());
    std::unique_ptr<StatsService::Stub> stub = StatsService::NewStub(channel);
    grpc::ClientContext context;
    Empty response;
    grpc::Status status = stub->SendStats(&context, producer_stats, &response);
    if (!status.ok()) {
        std::cerr << "gRPC request failed: " << status.error_message() << std::endl;
    }
    // std::cout << "SEND STATS: " << std::endl;
}

void KafkaProducer::sortAndPrintLogs() {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto& pair : logs) {
        std::cout << pair.second << std::endl;
    }
}