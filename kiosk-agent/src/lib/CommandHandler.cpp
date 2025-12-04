#include "CommandHandler.h"
#include "UnityControl.h"
#include "CardReaderControl.h"
#include "utils/JsonUtils.h"
#include <vector>
#include <sstream>

// Simple split helper (topic parts)
static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) out.push_back(item);
    return out;
}

void handleCommand(const AgentConfig& cfg,
                   IMqttClient& client,
                   const std::string& topic,
                   const std::string& payload) {
    // topic: "kiosk/<kioskId>/cmd/<commandName>"
    auto parts = split(topic, '/');
    if (parts.size() < 4) return;
    std::string commandName = parts[3];

    std::string commandId = extractCommandId(payload);
    if (commandId.empty()) commandId = "no-id";

    std::string status;
    std::string details;

    if (commandName == "restart_app") {
        bool ok = restartApp(cfg);
        status  = ok ? "success" : "failure";
        details = ok ? "App restarted." : "Failed to restart app.";
    } else if (commandName == "restart_card_reader") {
        bool ok = restartCardReader(cfg);
        status  = ok ? "success" : "failure";
        details = ok ? "Card reader restart OK (stub)." : "Card reader restart failed.";
    } else {
        status  = "error";
        details = "Unknown command: " + commandName;
    }

    std::string respTopic = "kiosk/" + cfg.kioskId + "/cmd_resp/" + commandId;
    std::string respJson  = buildCmdResultJson(cfg.kioskId, commandId, status, details);
    client.publish(respTopic, respJson, /*qos=*/1, /*retain=*/false);
}
