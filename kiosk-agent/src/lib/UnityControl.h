#ifndef UNITY_CONTROL_H
#define UNITY_CONTROL_H

#include "Config.h"

// Check if monitored app is running.
bool isAppRunning(const AgentConfig& cfg);

// Restart the monitored app (kill + start).
bool restartApp(const AgentConfig& cfg);

#endif // UNITY_CONTROL_H
