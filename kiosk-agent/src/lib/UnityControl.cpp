#include "UnityControl.h"
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <iostream>

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

bool isAppRunning(const AgentConfig& cfg) {
#ifdef KA_OS_WINDOWS
    std::wstring exeName(cfg.appName.begin(), cfg.appName.end());
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    bool found = false;

    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, exeName.c_str()) == 0) {
                found = true;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }

    CloseHandle(snap);
    return found;
#else
    // Linux/macOS: use pgrep -x appName
    std::string cmd = "pgrep -x '" + cfg.appName + "' >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    return (ret == 0);
#endif
}

static bool killApp(const AgentConfig& cfg) {
#ifdef KA_OS_WINDOWS
    std::wstring exeName(cfg.appName.begin(), cfg.appName.end());
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    bool success = false;

    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, exeName.c_str()) == 0) {
                HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProc != NULL) {
                    if (TerminateProcess(hProc, 0)) success = true;
                    CloseHandle(hProc);
                }
            }
        } while (Process32NextW(snap, &pe));
    }

    CloseHandle(snap);
    return success;
#else
    std::string cmd = "pkill -x '" + cfg.appName + "' >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    return (ret == 0);
#endif
}

static bool startApp(const AgentConfig& cfg) {
#ifdef KA_OS_WINDOWS
    std::wstring exePath(cfg.appPath.begin(), cfg.appPath.end());
    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    wchar_t cmdBuf[1024];
    wcsncpy_s(cmdBuf, exePath.c_str(), _TRUNCATE);

    if (!CreateProcessW(NULL, cmdBuf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::wcerr << L"Failed to start app: " << exePath << std::endl;
        return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
#else
    std::string cmd = cfg.appPath + " >/dev/null 2>&1 &";
    int ret = std::system(cmd.c_str());
    return (ret == 0);
#endif
}

bool restartApp(const AgentConfig& cfg) {
    killApp(cfg);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return startApp(cfg);
}
