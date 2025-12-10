cmake_minimum_required(VERSION 3.16)

project(DapNetworkMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

# Base sources (platform-independent)
set(SRC_BASE
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorAbstract.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/DapMonitorCmdProgram.cpp
)

set(HDR_BASE
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorAbstract.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DapMonitorCmdProgram.h
)

# Platform-specific sources
if(LINUX AND NOT ANDROID)
  list(APPEND SRC_BASE ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorLinux.cpp)
  list(APPEND HDR_BASE ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorLinux.h)
  message(STATUS "DapNetworkMonitor: Building for Linux")
elseif(WIN32)
  list(APPEND SRC_BASE ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorWindows.cpp)
  list(APPEND HDR_BASE ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorWindows.h)
  message(STATUS "DapNetworkMonitor: Building for Windows")
elseif(APPLE)
  list(APPEND SRC_BASE ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorDarwin.cpp)
  list(APPEND HDR_BASE ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorDarwin.h)
  message(STATUS "DapNetworkMonitor: Building for macOS")
elseif(ANDROID)
  list(APPEND SRC_BASE ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorAndroid.cpp)
  list(APPEND HDR_BASE ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorAndroid.h)
  message(STATUS "DapNetworkMonitor: Building for Android")
else()
  message(WARNING "DapNetworkMonitor: Unknown platform")
endif()

add_library(dap_network_monitor STATIC ${SRC_BASE} ${HDR_BASE})

target_link_libraries(dap_network_monitor PUBLIC Qt6::Core Qt6::Network cellframe-sdk)

# Windows-specific dependencies
if(WIN32)
    find_package(Qt6 COMPONENTS Concurrent REQUIRED)
    target_link_libraries(dap_network_monitor PUBLIC Qt6::Concurrent iphlpapi ws2_32)
endif()

# Platform-specific include directories
target_include_directories(dap_network_monitor PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/io
  ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/core/include
)

# Add platform-specific io includes
if(LINUX AND NOT ANDROID)
  target_include_directories(dap_network_monitor PUBLIC
    ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/io/linux
  )
elseif(WIN32)
  target_include_directories(dap_network_monitor PUBLIC
    ${CMAKE_SOURCE_DIR}/DapChainVPN/Service/vpn/tunnel/windows/tuntap  # For tuntap.h
  )
elseif(APPLE)
  target_include_directories(dap_network_monitor PUBLIC
    ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/io/macos
  )
endif()


