#include "Config.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Naive helper: find "key": "value" or "key": value in a JSON string.
static bool extractJsonString(const std::string& json, const std::string& key, std::string& out) {
    std::string pattern = "\"" + key + "\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return false;
    pos = json.find('"', pos);
    if (pos == std::string::npos) return false;
    auto end = json.find('"', pos + 1);
    if (end == std::string::npos) return false;
    out = json.substr(pos + 1, end - pos - 1);
    return true;
}

static bool extractJsonInt(const std::string& json, const std::string& key, int& out) {
    std::string pattern = "\"" + key + "\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return false;
    // skip spaces
    while (pos < json.size() && (json[pos] == ':' || json[pos] == ' ')) ++pos;
    std::size_t end = pos;
    while (end < json.size() && (json[end] == '-' || (json[end] >= '0' && json[end] <= '9'))) {
        ++end;
    }
    if (end == pos) return false;
    try {
        out = std::stoi(json.substr(pos, end - pos));
        return true;
    } catch (...) {
        return false;
    }
}

static bool extractJsonBool(const std::string& json, const std::string& key, bool& out) {
    std::string pattern = "\"" + key + "\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return false;
    while (pos < json.size() && (json[pos] == ':' || json[pos] == ' ')) ++pos;
    if (json.compare(pos, 4, "true") == 0) {
        out = true;
        return true;
    }
    if (json.compare(pos, 5, "false") == 0) {
        out = false;
        return true;
    }
    return false;
}

bool loadConfig(const std::string& path, AgentConfig& cfg) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "Could not open config file: " << path << '\n';
        return false;
    }
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string json = buffer.str();

    extractJsonString(json, "brokerHost", cfg.brokerHost);
    extractJsonInt(json, "brokerPort", cfg.brokerPort);
    extractJsonBool(json, "useTls", cfg.useTls);
    extractJsonString(json, "username", cfg.username);
    extractJsonString(json, "password", cfg.password);
    extractJsonString(json, "kioskId", cfg.kioskId);
    extractJsonString(json, "appName", cfg.appName);
    extractJsonString(json, "appPath", cfg.appPath);
    extractJsonString(json, "cardReaderServiceName", cfg.cardReaderServiceName);

    if (cfg.kioskId.empty()) {
        std::cerr << "Config error: kioskId is required\n";
        return false;
    }
    if (cfg.brokerHost.empty()) {
        std::cerr << "Config error: brokerHost is required\n";
        return false;
    }
    return true;
}
