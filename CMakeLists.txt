cmake_minimum_required(VERSION 3.16)

set(CMAKE_VERBOSE_MAKEFILE ON)

project(dap_networkmonitor CXX)

qt_standard_project_setup()


find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
#linux
file(GLOB DAP_NM_NATIVE_SRC  DapNetworkMonitorAbstract.cpp DapNetworkMonitor.cpp DapNetworkMonitorLinux.cpp)
file(GLOB DAP_NM_NATIVE_HDR  DapNetworkMonitorAbstract.h DapNetworkMonitor.h DapNetworkMonitorLinux.h)

add_library(${PROJECT_NAME} STATIC ${DAP_NM_NATIVE_SRC} ${DAP_NM_NATIVE_HDR})
target_include_directories(${PROJECT_NAME} PUBLIC ./)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core5Compat)
target_link_libraries(${PROJECT_NAME} PRIVATE dap_core)

#target_include_directories(${PROJECT_NAME} PUBLIC ./include)
#target_link_libraries(${PROJECT_NAME} PRIVATE dap_core)
#target_link_libraries(${PROJECT_NAME} PRIVATE dap_crypto)
#target_link_libraries(${PROJECT_NAME} PRIVATE DapUiSdk::Core)
#target_link_libraries(${PROJECT_NAME} PRIVATE DapUiSdk::Stream)


message("[*] DAP-NETWORKMON: linux")