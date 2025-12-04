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
std::string joinPath(const std::string** path1);

#endif // PATH_H