#include "Metrics.h"
#include <chrono>
#include <thread>
#include <fstream>

#if defined(_WIN32) || defined(_WIN64)
    #define KA_OS_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #define KA_OS_MACOS
#elif defined(__linux__)
    #define KA_OS_LINUX
#else
    #error "Unsupported OS"
#endif

#ifdef KA_OS_WINDOWS
    #include <windows.h>
    #include <tlhelp32.h>
#endif

#ifdef KA_OS_LINUX
    #include <sys/statvfs.h>
    #include <unistd.h>
#endif

#ifdef KA_OS_MACOS
    #include <sys/mount.h>
    #include <sys/statvfs.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <mach/mach.h>
#endif

// ---------- Memory ----------

bool getMemoryUsageMB(double& usedMb, double& totalMb) {
#ifdef KA_OS_WINDOWS
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex)) return false;
    totalMb = static_cast<double>(statex.ullTotalPhys) / (1024.0 * 1024.0);
    double freeMb = static_cast<double>(statex.ullAvailPhys) / (1024.0 * 1024.0);
    usedMb = totalMb - freeMb;
    return true;
#elif defined(KA_OS_LINUX)
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo.is_open()) return false;
    long memTotalKb = 0, memAvailableKb = 0;
    std::string key;
    long value;
    std::string unit;
    while (meminfo >> key >> value >> unit) {
        if (key == "MemTotal:") memTotalKb = value;
        else if (key == "MemAvailable:") memAvailableKb = value;
    }
    if (memTotalKb <= 0) return false;
    totalMb = static_cast<double>(memTotalKb) / 1024.0;
    double availMb = static_cast<double>(memAvailableKb) / 1024.0;
    usedMb = totalMb - availMb;
    return true;
#elif defined(KA_OS_MACOS)
    int mib[2] = { CTL_HW, HW_MEMSIZE };
    int64_t memsize;
    size_t len = sizeof(memsize);
    if (sysctl(mib, 2, &memsize, &len, NULL, 0) != 0) return false;
    totalMb = static_cast<double>(memsize) / (1024.0 * 1024.0);

    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmstat;
    if (host_statistics64(mach_host_self(), HOST_VM_INFO,
                          (host_info64_t)&vmstat, &count) != KERN_SUCCESS) {
        return false;
    }
    int64_t pageSize = sysconf(_SC_PAGESIZE);
    int64_t freeBytes = static_cast<int64_t>(vmstat.free_count) * pageSize;
    int64_t inactiveBytes = static_cast<int64_t>(vmstat.inactive_count) * pageSize;
    int64_t usedBytes = memsize - freeBytes - inactiveBytes;
    usedMb = static_cast<double>(usedBytes) / (1024.0 * 1024.0);
    return true;
#else
    (void)usedMb; (void)totalMb;
    return false;
#endif
}

// ---------- CPU ----------

#ifdef KA_OS_WINDOWS
struct CpuTimesWin {
    ULARGE_INTEGER idle, kernel, user;
};

static bool getCpuTimesWin(CpuTimesWin& out) {
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) return false;
    out.idle.QuadPart   = reinterpret_cast<ULARGE_INTEGER*>(&idleTime)->QuadPart;
    out.kernel.QuadPart = reinterpret_cast<ULARGE_INTEGER*>(&kernelTime)->QuadPart;
    out.user.QuadPart   = reinterpret_cast<ULARGE_INTEGER*>(&userTime)->QuadPart;
    return true;
}

static double computeCpuUsageWin(const CpuTimesWin& prev, const CpuTimesWin& curr) {
    ULONGLONG idleDiff   = curr.idle.QuadPart   - prev.idle.QuadPart;
    ULONGLONG kernelDiff = curr.kernel.QuadPart - prev.kernel.QuadPart;
    ULONGLONG userDiff   = curr.user.QuadPart   - prev.user.QuadPart;
    ULONGLONG totalDiff  = kernelDiff + userDiff;
    if (totalDiff == 0) return 0.0;
    double idlePercent = static_cast<double>(idleDiff) * 100.0 / static_cast<double>(totalDiff);
    return 100.0 - idlePercent;
}
#endif

#ifdef KA_OS_LINUX
struct CpuTimesLinux {
    uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
};

static bool readCpuTimesLinux(CpuTimesLinux& t) {
    std::ifstream f("/proc/stat");
    if (!f.is_open()) return false;
    std::string cpu;
    f >> cpu;
    if (cpu != "cpu") return false;
    f >> t.user >> t.nice >> t.system >> t.idle
      >> t.iowait >> t.irq >> t.softirq >> t.steal;
    return true;
}

static double computeCpuUsageLinux(const CpuTimesLinux& prev, const CpuTimesLinux& curr) {
    uint64_t prevIdle = prev.idle + prev.iowait;
    uint64_t idle     = curr.idle + curr.iowait;

    uint64_t prevNonIdle = prev.user + prev.nice + prev.system
                           + prev.irq + prev.softirq + prev.steal;
    uint64_t nonIdle     = curr.user + curr.nice + curr.system
                           + curr.irq + curr.softirq + curr.steal;

    uint64_t prevTotal = prevIdle + prevNonIdle;
    uint64_t total     = idle + nonIdle;

    uint64_t totalDiff = total - prevTotal;
    uint64_t idleDiff  = idle - prevIdle;
    if (totalDiff == 0) return 0.0;
    return static_cast<double>(totalDiff - idleDiff) * 100.0 / static_cast<double>(totalDiff);
}
#endif

#ifdef KA_OS_MACOS
struct CpuTimesMac {
    uint64_t user, nice, system, idle;
};

static bool readCpuTimesMac(CpuTimesMac& t) {
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO,
                        reinterpret_cast<host_info_t>(&cpuinfo), &count) != KERN_SUCCESS) {
        return false;
    }
    t.user   = cpuinfo.cpu_ticks[CPU_STATE_USER];
    t.nice   = cpuinfo.cpu_ticks[CPU_STATE_NICE];
    t.system = cpuinfo.cpu_ticks[CPU_STATE_SYSTEM];
    t.idle   = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
    return true;
}

static double computeCpuUsageMac(const CpuTimesMac& prev, const CpuTimesMac& curr) {
    uint64_t userDiff = curr.user   - prev.user;
    uint64_t niceDiff = curr.nice   - prev.nice;
    uint64_t sysDiff  = curr.system - prev.system;
    uint64_t idleDiff = curr.idle   - prev.idle;
    uint64_t total    = userDiff + niceDiff + sysDiff + idleDiff;
    if (total == 0) return 0.0;
    return static_cast<double>(total - idleDiff) * 100.0 / static_cast<double>(total);
}
#endif

double getCpuUsagePercent() {
#ifdef KA_OS_WINDOWS
    static bool       first = true;
    static CpuTimesWin prev;
    CpuTimesWin        curr;
    if (first) {
        if (!getCpuTimesWin(prev)) return 0.0;
        first = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        getCpuTimesWin(curr);
    } else {
        if (!getCpuTimesWin(curr)) return 0.0;
    }
    double usage = computeCpuUsageWin(prev, curr);
    prev = curr;
    return usage;
#elif defined(KA_OS_LINUX)
    static bool          first = true;
    static CpuTimesLinux prev;
    CpuTimesLinux        curr;
    if (first) {
        if (!readCpuTimesLinux(prev)) return 0.0;
        first = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        readCpuTimesLinux(curr);
    } else {
        if (!readCpuTimesLinux(curr)) return 0.0;
    }
    double usage = computeCpuUsageLinux(prev, curr);
    prev = curr;
    return usage;
#elif defined(KA_OS_MACOS)
    static bool        first = true;
    static CpuTimesMac prev;
    CpuTimesMac        curr;
    if (first) {
        if (!readCpuTimesMac(prev)) return 0.0;
        first = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        readCpuTimesMac(curr);
    } else {
        if (!readCpuTimesMac(curr)) return 0.0;
    }
    double usage = computeCpuUsageMac(prev, curr);
    prev = curr;
    return usage;
#else
    return 0.0;
#endif
}

// ---------- Disks ----------

std::vector<DiskUsage> collectDiskUsage() {
    std::vector<DiskUsage> disks;

#ifdef KA_OS_WINDOWS
    wchar_t drives[512];
    DWORD len = GetLogicalDriveStringsW(511, drives);
    wchar_t* p = drives;
    while (*p) {
        std::wstring drive(p);
        UINT type = GetDriveTypeW(drive.c_str());
        if (type == DRIVE_FIXED) {
            ULARGE_INTEGER freeBytesAvailable, totalBytes, freeBytes;
            if (GetDiskFreeSpaceExW(drive.c_str(), &freeBytesAvailable, &totalBytes, &freeBytes)) {
                DiskUsage du;
                du.name = std::string(drive.begin(), drive.end());
                du.totalGb = static_cast<double>(totalBytes.QuadPart)
                             / (1024.0 * 1024.0 * 1024.0);
                double freeGb = static_cast<double>(freeBytes.QuadPart)
                                / (1024.0 * 1024.0 * 1024.0);
                du.usedGb = du.totalGb - freeGb;
                disks.push_back(du);
            }
        }
        p += wcslen(p) + 1;
    }
#elif defined(KA_OS_LINUX) || defined(KA_OS_MACOS)
    struct statvfs s;
    if (statvfs("/", &s) == 0) {
        double total = static_cast<double>(s.f_blocks) * s.f_frsize
                       / (1024.0 * 1024.0 * 1024.0);
        double free = static_cast<double>(s.f_bfree) * s.f_frsize
                      / (1024.0 * 1024.0 * 1024.0);
        DiskUsage du;
        du.name    = "/";
        du.totalGb = total;
        du.usedGb  = total - free;
        disks.push_back(du);
    }
#endif

    return disks;
}
