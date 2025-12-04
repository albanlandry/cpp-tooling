#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "../Config.h"
#include <string>

// Callback type for incoming messages (topic + payload)
typedef void (*MqttMessageCallback)(const std::string& topic,
                                    const std::string& payload);

class IMqttClient {
public:
    virtual ~IMqttClient() {}
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() = 0;
    virtual bool publish(const std::string& topic,
                         const std::string& payload,
                         int qos,
                         bool retain) = 0;
    virtual bool subscribe(const std::string& topic, int qos) = 0;
    virtual void loop(int timeoutMs) = 0;
};

// Dummy implementation (for testing / scaffolding).
// Replace with a real MQTT implementation (e.g. Paho) later.
class DummyMqttClient : public IMqttClient {
public:
    DummyMqttClient(const AgentConfig& cfg, MqttMessageCallback cb);

    bool connect() override;
    void disconnect() override;
    bool isConnected() override;
    bool publish(const std::string& topic,
                 const std::string& payload,
                 int qos,
                 bool retain) override;
    bool subscribe(const std::string& topic, int qos) override;
    void loop(int timeoutMs) override;

private:
    AgentConfig        config;
    MqttMessageCallback callback;
    bool               connected;
};

#endif // MQTT_CLIENT_H
