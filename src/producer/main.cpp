#include "producer.h"
#include <iostream>
using namespace std;

int main() {
    string brokers = "kafka:9092";
    string topic = "market_data";

    KafkaProducer producer(brokers, topic);

    for (int i=0; i<10; i++) {
        string message = "Market Update " + to_string(i);
        if (producer.sendMessage(message)) {
            cout << "Sent: " << message << endl;
        }
        else {
            cerr << "Failed to sent message: " << message << endl;
        }
    }

    return 0;
}