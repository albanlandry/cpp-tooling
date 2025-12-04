#include "path.h"

std::string getConfigPath() { return "data/agent_config.json"; }

std::string getDataPath() { return "data"; }

std::string getLogsPath() { return "logs"; }

std::string getTempPath() { return "temp"; }

std::string getCachePath() { return "cache"; }

//  

template <typename... Paths> std::string joinPath(Paths &&...paths) {
  // Handle empty case
  if constexpr (sizeof...(paths) == 0) {
    return "";
  }

  std::string result;
  bool first = true;

  // Fold expression to process each path
  (([&]() {
     std::string part = toString(std::forward<Paths>(paths));

     // Skip empty parts
     if (part.empty()) {
       return;
     }

     // Remove leading slash from non-first parts
     if (!first && !part.empty() && part[0] == '/') {
       part = part.substr(1);
     }

     // Remove trailing slash
     if (!part.empty() && part.back() == '/') {
       part.pop_back();
     }

     // Add separator if needed
     if (!first && !result.empty() && !part.empty()) {
       result += '/';
     }

     result += part;
     first = false;
   }()),
   ...);

  return result;
}