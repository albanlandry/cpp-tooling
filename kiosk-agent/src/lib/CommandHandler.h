#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "Config.h"
#include "mqtt/MqttClient.h"

// Handle single incoming command message.
void handleCommand(const AgentConfig& cfg,
                   IMqttClient& client,
                   const std::string& topic,
                   const std::string& payload);

#endif // COMMAND_HANDLER_H
