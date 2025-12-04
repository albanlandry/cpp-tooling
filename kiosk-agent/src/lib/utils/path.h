#ifndef PATH_H
#define PATH_H

#include <string>

std::string getConfigPath();
std::string getDataPath();
std::string getLogsPath();
std::string getTempPath();
std::string getCachePath();
std::string getConfigPath();
std::string getConfigPath();

template<typename... Args>
std::string joinPath(const std::string& path1, const Args&... args) {
    return joinPath(path1, args...);
}

template<typename... Paths>
std::string joinPath(Paths&&... paths);

#endif // PATH_H