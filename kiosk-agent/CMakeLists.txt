cmake_minimum_required(VERSION 3.15)

# Project
project(kiosk_agent
    VERSION 0.1.0
    LANGUAGES CXX
)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Enable more warnings
if(MSVC)
    add_compile_options(/W4 /permissive- /EHsc)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/lib/Config.cpp
    ${SRC_DIR}/lib/Metrics.cpp
    ${SRC_DIR}/lib/UnityControl.cpp
    ${SRC_DIR}/lib/CardReaderControl.cpp
    ${SRC_DIR}/lib/mqtt/MqttClient.cpp
    ${SRC_DIR}/lib/CommandHandler.cpp
    ${SRC_DIR}/lib/utils/JsonUtils.cpp
)

set(HEADERS
    ${SRC_DIR}/lib/Config.h
    ${SRC_DIR}/lib/Metrics.h
    ${SRC_DIR}/lib/UnityControl.h
    ${SRC_DIR}/lib/CardReaderControl.h
    ${SRC_DIR}/lib/mqtt/MqttClient.h
    ${SRC_DIR}/lib/CommandHandler.h
    ${SRC_DIR}/lib/utils/JsonUtils.h
)

add_executable(kiosk_agent ${SOURCES} ${HEADERS})

target_include_directories(kiosk_agent
    PRIVATE
        ${SRC_DIR}
        ${SRC_DIR}/lib
        ${SRC_DIR}/lib/mqtt
        ${SRC_DIR}/lib/utils
)

# OS-specific settings (if needed later)
if(WIN32)
    message(STATUS "Configuring for Windows")

    # On Windows you might later add ws2_32, secur32, etc. for real MQTT/TLS.
    # target_link_libraries(kiosk_agent PRIVATE ws2_32)

elseif(APPLE)
    message(STATUS "Configuring for macOS")

    # For mach APIs used in Metrics.cpp we need CoreServices / IOKit / etc.
    # Currently we only use <mach/mach.h>, which is in libSystem.
    # If you later link extra frameworks, do it like:
    # target_link_libraries(kiosk_agent PRIVATE "-framework CoreServices")

elseif(UNIX)
    message(STATUS "Configuring for Linux")

    # If you later add pthreads explicitly:
    # find_package(Threads REQUIRED)
    # target_link_libraries(kiosk_agent PRIVATE Threads::Threads)

endif()

# Install rule (optional)
install(TARGETS kiosk_agent
    RUNTIME DESTINATION bin
)
