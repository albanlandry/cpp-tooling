#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct AgentConfig {
  std::string brokerHost;
  int brokerPort = 1883;
  bool useTls = false;
  std::string username;
  std::string password;
  std::string kioskId;

  // App to monitor (Unity kiosk app or other)
  std::string appName; // process name (e.g. "KioskApp.exe" or "kiosk_app")
  std::string appPath; // full path to executable

  // Card reader service / process name (OS-specific)
  std::string cardReaderServiceName;
};

// Load configuration from agent_config.json
bool loadConfig(const std::string &path, AgentConfig &cfg);

#endif // CONFIG_H
