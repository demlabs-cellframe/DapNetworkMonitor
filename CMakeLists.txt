cmake_minimum_required(VERSION 3.16)

set(CMAKE_VERBOSE_MAKEFILE ON)

project(dap_networkmonitor CXX)

qt_standard_project_setup()


find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)


file(GLOB DAP_NM_NATIVE_SRC  DapNetworkMonitorAbstract.cpp DapNetworkMonitor.cpp )
file(GLOB DAP_NM_NATIVE_HDR  DapNetworkMonitorAbstract.h DapNetworkMonitor.h )

if (ANDROID)
    file(GLOB DAP_NM_PLATFORM_SRC               ./android/*.cpp )
    file(GLOB DAP_NM_PLATFORM_HDR               ./android/*.h )
    set(DAP_NM_PLATFORM_PUBLIC_INCLUDES         ./android/ )
    message("[*] DAP-NETWORKMON: android")
else()
if (MACOS)
    file(GLOB DAP_NM_PLATFORM_SRC               ./macos/*.cpp)
    file(GLOB DAP_NM_PLATFORM_HDR               ./macos/*.h )
    set(DAP_NM_PLATFORM_PUBLIC_INCLUDES         ./macos/ )
    message("[*] DAP-NETWORKMON: macOS")
elseif(IOS)
    file(GLOB DAP_NM_PLATFORM_SRC               ./ios/*.mm ./ios/*.cpp)
    file(GLOB DAP_NM_PLATFORM_HDR               ./ios/*.h )
    set(DAP_NM_PLATFORM_PUBLIC_INCLUDES         ./ios/ )
    message("[*] DAP-NETWORKMON: ios")
elseif(UNIX)
    file(GLOB DAP_NM_PLATFORM_SRC               ./unix/*.cpp)
    file(GLOB DAP_NM_PLATFORM_HDR               ./unix/*.h )
    set(DAP_NM_PLATFORM_PUBLIC_INCLUDES         ./unix/ )
    message("[*] DAP-NETWORKMON: linux")
endif()
endif()

if (WIN32)
    file(GLOB DAP_NM_PLATFORM_SRC               ./windows/*.cpp ./windows/tuntap/*.cpp)
    file(GLOB DAP_NM_PLATFORM_HDR               ./windows/*.h ./windows/tuntap/*.h)
    set(DAP_NM_PLATFORM_PUBLIC_INCLUDES         ./windows/ ./windows/tuntap)
    message("[*] DAP-NETWORKMON: windows")
endif()


add_library(${PROJECT_NAME} STATIC ${DAP_NM_NATIVE_SRC} ${DAP_NM_NATIVE_HDR} ${DAP_NM_PLATFORM_SRC} ${DAP_NM_PLATFORM_HDR})
target_include_directories(${PROJECT_NAME} PUBLIC ./)
target_include_directories(${PROJECT_NAME} PUBLIC ${DAP_NM_PLATFORM_PUBLIC_INCLUDES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core5Compat)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Concurrent)

target_link_libraries(${PROJECT_NAME} PRIVATE dap_core)

