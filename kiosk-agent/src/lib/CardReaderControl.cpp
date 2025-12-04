#include "CardReaderControl.h"
#include <iostream>
#include <string>

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
#endif

bool restartCardReader(const AgentConfig& cfg) {
#ifdef KA_OS_WINDOWS
    // TODO: Use Service Control Manager to restart cfg.cardReaderServiceName.
    std::wcout << L"[INFO] Would restart Windows service: "
               << std::wstring(cfg.cardReaderServiceName.begin(),
                               cfg.cardReaderServiceName.end())
               << std::endl;
    return true;
#else
    // TODO: Use systemctl or other mechanism on Linux/macOS.
    std::cout << "[INFO] Would restart card reader service: "
              << cfg.cardReaderServiceName << "\n";
    return true;
#endif
}
