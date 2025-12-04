#include "JsonUtils.h"
#include <sstream>
#include <chrono>

// Local helper: Unix time
static std::uint64_t nowUnixSeconds() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

std::string jsonEscape(const std::string& s) {
    std::ostringstream oss;
    for (char c : s) {
        if (c == '\\')      oss << "\\\\";
        else if (c == '"')  oss << "\\\"";
        else                oss << c;
    }
    return oss.str();
}

std::string buildStatusJson(const std::string& kioskId, const std::string& state) {
    std::ostringstream oss;
    oss << "{"
        << "\"kioskId\":\"" << jsonEscape(kioskId) << "\","
        << "\"timestamp\":" << nowUnixSeconds() << ","
        << "\"state\":\"" << jsonEscape(state) << "\""
        << "}";
    return oss.str();
}

std::string buildMetricsJson(const std::string& kioskId,
                             double cpu,
                             double ramUsed,
                             double ramTotal,
                             const std::vector<DiskUsage>& disks) {
    std::ostringstream oss;
    oss << "{"
        << "\"kioskId\":\"" << jsonEscape(kioskId) << "\","
        << "\"timestamp\":" << nowUnixSeconds() << ","
        << "\"cpu\":" << cpu << ","
        << "\"ram\":{\"usedMb\":" << ramUsed << ",\"totalMb\":" << ramTotal << "},"
        << "\"disks\":[";
    for (std::size_t i = 0; i < disks.size(); ++i) {
        const auto& d = disks[i];
        if (i > 0) oss << ",";
        oss << "{"
            << "\"name\":\"" << jsonEscape(d.name) << "\","
            << "\"usedGb\":" << d.usedGb << ","
            << "\"totalGb\":" << d.totalGb
            << "}";
    }
    oss << "]}";
    return oss.str();
}

std::string buildCmdResultJson(const std::string& kioskId,
                               const std::string& commandId,
                               const std::string& status,
                               const std::string& details) {
    std::ostringstream oss;
    oss << "{"
        << "\"commandId\":\"" << jsonEscape(commandId) << "\","
        << "\"kioskId\":\"" << jsonEscape(kioskId) << "\","
        << "\"status\":\"" << jsonEscape(status) << "\","
        << "\"timestamp\":" << nowUnixSeconds() << ","
        << "\"details\":\"" << jsonEscape(details) << "\""
        << "}";
    return oss.str();
}

std::string extractCommandId(const std::string& json) {
    std::string pattern = "\"commandId\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos);
    if (pos == std::string::npos) return "";
    auto end = json.find('"', pos + 1);
    if (end == std::string::npos) return "";
    return json.substr(pos + 1, end - pos - 1);
}
