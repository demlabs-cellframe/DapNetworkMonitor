cmake_minimum_required(VERSION 3.16)

project(DapNetworkMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorAbstract.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/DapMonitorCmdProgram.cpp
)

set(HDR
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorAbstract.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DapMonitorCmdProgram.h
)

# Add platform-specific sources
if(APPLE)
  list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorDarwin.cpp)
  list(APPEND HDR ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorDarwin.h)
elseif(WIN32)
  list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorWindows.cpp)
  list(APPEND HDR ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorWindows.h)
elseif(ANDROID)
  list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorAndroid.cpp)
  list(APPEND HDR ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorAndroid.h)
else() # Linux and other Unix
  list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorLinux.cpp)
  list(APPEND HDR ${CMAKE_CURRENT_SOURCE_DIR}/DapNetworkMonitorLinux.h)
endif()

add_library(dap_network_monitor STATIC ${SRC} ${HDR})

target_link_libraries(dap_network_monitor PUBLIC Qt6::Core Qt6::Network cellframe-sdk)

# Add platform-specific include directories
target_include_directories(dap_network_monitor PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/io
  ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/core/include
)

# Add platform-specific IO includes
if(APPLE)
  target_include_directories(dap_network_monitor PUBLIC
    ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/io/macos
  )
elseif(WIN32)
  target_include_directories(dap_network_monitor PUBLIC
    ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/io/win32
  )
else() # Linux and other Unix
  target_include_directories(dap_network_monitor PUBLIC
    ${CMAKE_SOURCE_DIR}/cellframe-sdk/dap-sdk/io/linux
  )
endif()


