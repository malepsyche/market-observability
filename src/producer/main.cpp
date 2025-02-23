#include "producer.h"
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

void produceMessages(KafkaProducer& producer, int thread_id) {
    int numMessages = 3;
    for (int i=0; i<numMessages; i++) {
        string message = "Thread " + to_string(thread_id) + " - Market Update " + to_string(i);
        producer.sendMessage(message, thread_id);
    }
}

int main() {
    string brokers = "kafka:9092";
    string topic = "market_data";

    KafkaProducer producer(brokers, topic);

    vector<thread> threads;
    int num_threads = 4;    // start with min(num_cores, num_partitions)
    cout << "[Main] Starting " << num_threads << " producer threads..." << endl;

    for (int i=0; i<num_threads; i++) {
        threads.emplace_back(produceMessages, ref(producer), i);
    }
    for (auto& t : threads) {
        t.join();
    }
    cout << "[Main] All producer threads completed." << endl;

    return 0;
}