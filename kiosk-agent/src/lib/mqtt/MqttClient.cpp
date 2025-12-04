#include "MqttClient.h"
#include <iostream>
#include <thread>
#include <chrono>

DummyMqttClient::DummyMqttClient(const AgentConfig& cfg, MqttMessageCallback cb)
    : config(cfg), callback(cb), connected(false) {}

bool DummyMqttClient::connect() {
    std::cout << "[MQTT] Dummy connect to "
              << config.brokerHost << ":" << config.brokerPort
              << " as kiosk " << config.kioskId << "\n";
    connected = true;
    return true;
}

void DummyMqttClient::disconnect() {
    std::cout << "[MQTT] Dummy disconnect\n";
    connected = false;
}

bool DummyMqttClient::isConnected() {
    return connected;
}

bool DummyMqttClient::publish(const std::string& topic,
                              const std::string& payload,
                              int qos,
                              bool retain) {
    (void)qos; (void)retain;
    std::cout << "[MQTT] PUBLISH " << topic << " : " << payload << "\n";
    return true;
}

bool DummyMqttClient::subscribe(const std::string& topic, int qos) {
    (void)qos;
    std::cout << "[MQTT] SUBSCRIBE " << topic << "\n";
    return true;
}

void DummyMqttClient::loop(int timeoutMs) {
    // In a real client, process network I/O and call callback(topic,payload).
    std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
}
