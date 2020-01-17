SOURCES += $$PWD/DapNetworkMonitorAbstract.cpp

HEADERS += \
    $$PWD/DapNetworkMonitor.h \
    $$PWD/DapNetworkMonitorAbstract.h

!ios {
HEADERS += $$PWD/DapMonitorCmdProgram.h
SOURCES += $$PWD/DapMonitorCmdProgram.cpp
}

win32 {
    SOURCES += $$PWD/DapNetworkMonitorWindows.cpp
    HEADERS += $$PWD/DapNetworkMonitorWindows.h

    LIBS += -lWS2_32
    LIBS += -lAdvapi32
    LIBS += -lIphlpapi
    LIBS += -lUser32
    LIBS += -lole32
    LIBS += -luuid

    DEFINES += NTDDI_VERSION=0x06000000
    DEFINES += _WIN32_WINNT=0x0600
}

linux-*: !android {
    SOURCES += $$PWD/DapNetworkMonitorLinux.cpp
    HEADERS += $$PWD/DapNetworkMonitorLinux.h
}

android {
    SOURCES += $$PWD/DapNetworkMonitorAndroid.cpp
    HEADERS += $$PWD/DapNetworkMonitorAndroid.h
}

macos {
LIBS += -framework Foundation
LIBS += -framework CoreFoundation
LIBS += -framework SystemConfiguration
}

macosx: !ios {
SOURCES += $$PWD/DapNetworkMonitorDarwin.cpp
HEADERS += $$PWD/DapNetworkMonitorDarwin.h
}

ios {
LIBS += -framework NetworkExtension
HEADERS += $$PWD/DapNetworkMonitorIOS.h
SOURCES += $$PWD/DapNetworkMonitorIOS.cpp
}

INCLUDEPATH += $$PWD
