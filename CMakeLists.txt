cmake_minimum_required(VERSION 3.16)

project(DapNetworkMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorAbstract.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorLinux.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/DapMonitorCmdProgram.cpp
)

set(HDR
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorAbstract.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorLinux.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DapMonitorCmdProgram.h
)

add_library(dap_network_monitor STATIC ${SRC} ${HDR})

target_link_libraries(dap_network_monitor PUBLIC Qt6::Core Qt6::Network cellframe-sdk)

target_include_directories(dap_network_monitor PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/io/linux
  ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/io/macos
  ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/io
  ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/core/include
)


