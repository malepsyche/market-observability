#include "producer.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <barrier>


void produceMessages(KafkaProducer& producer, int thread_id, std::barrier<>& sync_point) {
    int numMessages = 1000000;
    for (int i=0; i<numMessages; i++) {
        // sync_point.arrive_and_wait();  
        std::string message = "Thread " + std::to_string(thread_id) + " - Market Update " + std::to_string(i);
        producer.sendMessage(message, thread_id);
    }
}

int main() {
    std::string brokers = "kafka:9092";
    std::string topic = "market_data";

    KafkaProducer producer(brokers, topic);

    std::vector<std::thread> threads;

    int num_threads = 2;    
    std::barrier sync_point(num_threads);   
    
    auto begin = std::chrono::high_resolution_clock::now();
    std::cout << "[Main] Starting " << num_threads << " producer threads..." << std::endl;

    for (int i=0; i<num_threads; i++) {
        threads.emplace_back(produceMessages, std::ref(producer), i, std::ref(sync_point));
    }
    for (auto& t : threads) {
        t.join();
    }

    producer.sortAndPrintLogs();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> elapsed = end - begin; 
    std::cout << "[Main] All producer threads completed." << std::endl;
    std::cout << "[Main] Time Elapsed: " << elapsed.count() << " microseconds" << std::endl;
    std::cout << "[Main] Average latency: " << producer.total_latency/producer.total_messages << " microseconds" << std::endl;
    std::cout << "[Main] Throughput: " << producer.total_messages/(elapsed.count()/1000000) << " messages per second" << std::endl;

    return 0;
}