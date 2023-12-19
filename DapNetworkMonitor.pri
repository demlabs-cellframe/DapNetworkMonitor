SOURCES += \
    $$PWD/DapNetworkMonitorAbstract.cpp \
    $$PWD/DapMonitorCmdProgram.cpp

HEADERS += \
    $$PWD/DapNetworkMonitor.h \
    $$PWD/DapNetworkMonitorAbstract.h \
    $$PWD/DapMonitorCmdProgram.h

win32 {
    SOURCES += $$PWD/DapNetworkMonitorWindows.cpp
    HEADERS += $$PWD/DapNetworkMonitorWindows.h

    LIBS += -lws2_32
    LIBS += -ladvapi32
    LIBS += -liphlpapi
    LIBS += -luser32
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
    #LIBS += -framework NetworkExtension
    LIBS += -framework SystemConfiguration
    SOURCES += $$PWD/DapNetworkMonitorDarwin.cpp
    HEADERS += $$PWD/DapNetworkMonitorDarwin.h
}

ios {
    QT -= core gui macextras
    LIBS += -framework Foundation
    LIBS += -framework CoreFoundation
    LIBS += -framework SystemConfiguration
    SOURCES += $$PWD/DapNetworkMonitorIOS.mm
    HEADERS += $$PWD/DapNetworkMonitorIOS.h
    SOURCES -= $$PWD/DapMonitorCmdProgram.cpp
    HEADERS -= $$PWD/DapMonitorCmdProgram.h
}

INCLUDEPATH += $$PWD
