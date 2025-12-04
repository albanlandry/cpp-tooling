#ifndef METRICS_H
#define METRICS_H

#include <vector>
#include <string>

struct DiskUsage {
    std::string name;   // e.g. "C:", "/" etc.
    double      usedGb;
    double      totalGb;
};

// Returns used/total memory in MB. Returns false on failure.
bool getMemoryUsageMB(double& usedMb, double& totalMb);

// Returns CPU usage (0–100) as a percentage (approximate).
double getCpuUsagePercent();

// Returns vector of disk usage info.
std::vector<DiskUsage> collectDiskUsage();

#endif // METRICS_H
