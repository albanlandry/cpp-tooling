#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <string>
#include <vector>
#include "../Metrics.h"

// Escape for JSON strings.
std::string jsonEscape(const std::string& s);

// Build JSON payloads
std::string buildStatusJson(const std::string& kioskId, const std::string& state);

std::string buildMetricsJson(const std::string& kioskId,
                             double cpu,
                             double ramUsed,
                             double ramTotal,
                             const std::vector<DiskUsage>& disks);

std::string buildCmdResultJson(const std::string& kioskId,
                               const std::string& commandId,
                               const std::string& status,
                               const std::string& details);

// Very naive extraction of "commandId" from JSON.
std::string extractCommandId(const std::string& json);

#endif // JSON_UTILS_H
