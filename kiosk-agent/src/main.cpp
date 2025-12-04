#include "lib/Config.h"
#include "lib/Metrics.h"
#include "lib/utils/JsonUtils.h"
#include "lib/mqtt/MqttClient.h"
#include "lib/CommandHandler.h"

#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>

static AgentConfig   g_config;
static IMqttClient*  g_mqtt = nullptr;

static void onMqttMessage(const std::string& topic, const std::string& payload) {
    if (!g_mqtt) return;
    handleCommand(g_config, *g_mqtt, topic, payload);
}

static std::uint64_t nowMillis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

static std::string getConfigPath() {
    return "data/agent_config.json";
}

int main() {
    if (!loadConfig("gooagent_config.json", g_config)) {
        std::cerr << "Failed to load config\n";
        return 1;
    }

    DummyMqttClient mqtt(g_config, &onMqttMessage); // Swap with real MQTT client later
    g_mqtt = &mqtt;

    // Connect with retry
    while (!mqtt.connect()) {
        std::cerr << "MQTT connect failed, retrying in 5s...\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Subscribe to command topic
    std::string cmdTopic = "kiosk/" + g_config.kioskId + "/cmd/#";
    mqtt.subscribe(cmdTopic, 1);

    // Publish online status
    std::string statusTopic   = "kiosk/" + g_config.kioskId + "/status";
    std::string statusPayload = buildStatusJson(g_config.kioskId, "online");
    mqtt.publish(statusTopic, statusPayload, 1, true);

    std::uint64_t lastMetricsMs = 0;
    std::atomic<bool> running(true);

    while (running) {
        std::uint64_t nowMs = nowMillis();

        if (nowMs - lastMetricsMs >= 10000) { // every 10s
            double cpu = getCpuUsagePercent();

            double ramUsed = 0.0;
            double ramTotal = 0.0;
            getMemoryUsageMB(ramUsed, ramTotal);

            auto disks = collectDiskUsage();

            std::string metricsJson = buildMetricsJson(g_config.kioskId, cpu, ramUsed, ramTotal, disks);
            std::string metricsTopic = "kiosk/" + g_config.kioskId + "/metrics";
            mqtt.publish(metricsTopic, metricsJson, /*qos=*/0, /*retain=*/false);

            lastMetricsMs = nowMs;
        }

        mqtt.loop(100); // process MQTT I/O (dummy just sleeps)
    }

    // On shutdown, send offline status (optional)
    mqtt.publish(statusTopic, buildStatusJson(g_config.kioskId, "offline"), 1, true);
    mqtt.disconnect();

    return 0;
}
